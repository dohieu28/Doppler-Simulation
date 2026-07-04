#include "..\inc\ai_estimator.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846

// Khởi tạo mạng ANFIS và nạp trọng số
AnfisEstimator *ai_estimator_init(const char *weight_file_path)
{
    AnfisEstimator *ai = malloc(sizeof(*ai));
    if (ai == NULL)
    {
        return NULL;
    }

    ai->num_inputs = 2;
    ai->num_mfs_per_input = 3;
    ai->num_rules = ai->num_inputs * ai->num_mfs_per_input;
    ai->num_rules = ai->num_mfs_per_input * ai->num_mfs_per_input;

    /*==========================
     * Cấp phát Input MFs
     *==========================*/
    ai->input_mfs = malloc(ai->num_inputs * sizeof(*ai->input_mfs));
    if (ai->input_mfs == NULL)
    {
        free(ai);
        return NULL;
    }

    for (int i = 0; i < ai->num_inputs; i++)
    {
        ai->input_mfs[i] = malloc(ai->num_mfs_per_input * sizeof(*ai->input_mfs[i]));
        if (ai->input_mfs[i] == NULL)
        {
            for (int j = 0; j < i; j++)
                free(ai->input_mfs[j]);

            free(ai->input_mfs);
            free(ai);
            return NULL;
        }
    }

    /*==========================
     * Cấp phát Rule Weights
     *==========================*/
    ai->rule_weights = malloc(ai->num_rules * sizeof(*ai->rule_weights));
    if (ai->rule_weights == NULL)
    {
        for (int i = 0; i < ai->num_inputs; i++)
            free(ai->input_mfs[i]);

        free(ai->input_mfs);
        free(ai);
        return NULL;
    }

    for (int i = 0; i < ai->num_rules; i++)
    {
        ai->rule_weights[i] =
            malloc((ai->num_inputs + 1) * sizeof(*ai->rule_weights[i]));

        if (ai->rule_weights[i] == NULL)
        {
            for (int j = 0; j < i; j++)
                free(ai->rule_weights[j]);

            free(ai->rule_weights);

            for (int j = 0; j < ai->num_inputs; j++)
                free(ai->input_mfs[j]);

            free(ai->input_mfs);
            free(ai);

            return NULL;
        }
    }

    /*=========================================================
     * Khởi tạo mặc định
     *=========================================================*/

    /* Input 1: Delta Theta */
    ai->input_mfs[0][0].mean = 0.0f;
    ai->input_mfs[0][0].sigma = 0.5f;

    ai->input_mfs[0][1].mean = PI / 2.0f;
    ai->input_mfs[0][1].sigma = 0.5f;

    ai->input_mfs[0][2].mean = PI;
    ai->input_mfs[0][2].sigma = 0.5f;

    /* Input 2: Magnitude */
    ai->input_mfs[1][0].mean = 0.0f;
    ai->input_mfs[1][0].sigma = 0.5f;

    ai->input_mfs[1][1].mean = 1.0f;
    ai->input_mfs[1][1].sigma = 0.5f;

    ai->input_mfs[1][2].mean = 2.0f;
    ai->input_mfs[1][2].sigma = 0.5f;

    /* Rule weights mặc định */
    /* Khởi tạo trọng số luật (p0, p1, p2). f = p0 + p1*x1 + p2*x2.
     Để ANFIS chạy ra kết quả giống lý thuyết ban đầu: fD = x1 / (2*PI*delta_t)
         Hệ số K = 1 / (2 * PI * (8 * 4384 / 122.88e6)) ~ 1115.35 */
    const float K = 1115.35f;

    for (int i = 0; i < ai->num_rules; i++)
    {
        ai->rule_weights[i][0] = 0.0f;
        ai->rule_weights[i][1] = K;
        ai->rule_weights[i][2] = 0.0f;
    }

    /*=========================================================
     * Nếu có file weight thì đọc đè lên giá trị mặc định
     *=========================================================*/
    if (weight_file_path != NULL)
    {
        FILE *fp = fopen(weight_file_path, "r");

        if (fp != NULL)
        {
            int ok = 1;

            for (int i = 0; i < ai->num_mfs_per_input && ok; i++)
            {
                ok &= (fscanf(fp, "%f %f",
                              &ai->input_mfs[0][i].mean,
                              &ai->input_mfs[0][i].sigma) == 2);
            }

            for (int i = 0; i < ai->num_mfs_per_input && ok; i++)
            {
                ok &= (fscanf(fp, "%f %f",
                              &ai->input_mfs[1][i].mean,
                              &ai->input_mfs[1][i].sigma) == 2);
            }

            for (int i = 0; i < ai->num_rules && ok; i++)
            {
                ok &= (fscanf(fp, "%f %f %f",
                              &ai->rule_weights[i][0],
                              &ai->rule_weights[i][1],
                              &ai->rule_weights[i][2]) == 3);
            }

            fclose(fp);

            if (!ok)
            {
                printf("Warning: File weight khong hop le. Su dung gia tri mac dinh.\n");
            }
            else
            {
                printf("Da nap ANFIS weights thanh cong.\n");
            }
        }
        else
        {
            printf("Khong tim thay file weight. Su dung gia tri mac dinh.\n");
        }
    }

    return ai;
}

// Hàm tính giá trị của hàm Gauss (Lớp 1: Fuzzification)
static float calc_gauss(float x, float mean, float sigma)
{
    return expf(-0.5f * powf((x - mean) / sigma, 2.0f));
}

// Chạy luồng suy luận của AI (Inference)
float ai_predict_doppler(AnfisEstimator *ai, float complex *rx_grid, float complex *tx_dmrs3, float complex *tx_dmrs11, int num_sc, float fs)
{

    // --- BƯỚC 1: TRÍCH XUẤT ĐẶC TRƯNG (FEATURE EXTRACTION) ---
    float complex correlation = 0;
    int fft_size = 4096; // Lấy từ config

    for (int sc = 0; sc < num_sc; sc++)
    {
        float complex rx_sym3 = rx_grid[2 * fft_size + sc];
        float complex rx_sym11 = rx_grid[10 * fft_size + sc];

        float complex h3 = rx_sym3 * conj(tx_dmrs3[sc]);
        float complex h11 = rx_sym11 * conj(tx_dmrs11[sc]);
        correlation += h11 * conj(h3);
    }

    // Đặc trưng đầu vào cho ANFIS
    float inputs[2];
    inputs[0] = fabsf(carg(correlation));   // x1: Góc pha Delta Theta
    inputs[1] = cabs(correlation) / num_sc; // x2: Biên độ trung bình

    // --- BƯỚC 2: MỜ HÓA (LỚP 1) ---
    float mu[2][3]; // Mảng lưu độ thuộc về (Membership degrees)
    for (int i = 0; i < ai->num_inputs; i++)
    {
        for (int j = 0; j < ai->num_mfs_per_input; j++)
        {
            mu[i][j] = calc_gauss(inputs[i], ai->input_mfs[i][j].mean, ai->input_mfs[i][j].sigma);
        }
    }

    // --- BƯỚC 3 & 4: TÍNH LUẬT VÀ CHUẨN HÓA (LỚP 2 & 3) ---
    float w[9]; // Độ kích hoạt của 9 luật
    float w_sum = 0.0f;
    int rule_idx = 0;

    for (int i = 0; i < ai->num_mfs_per_input; i++)
    {
        for (int j = 0; j < ai->num_mfs_per_input; j++)
        {
            // Toán tử T-norm (Nhân) để kết hợp các điều kiện mờ
            w[rule_idx] = mu[0][i] * mu[1][j];
            w_sum += w[rule_idx];
            rule_idx++;
        }
    }

    // Tránh lỗi chia cho 0
    if (w_sum == 0.0f)
        w_sum = 1e-6f;

    // --- BƯỚC 5: GIẢI MỜ TAKAGI-SUGENO (LỚP 4 & 5) ---
    float estimated_fd = 0.0f;
    for (int i = 0; i < ai->num_rules; i++)
    {
        float w_norm = w[i] / w_sum; // Chuẩn hóa

        // Tính giá trị kết luận của luật (f_i = p0 + p1*x1 + p2*x2)
        float f_i = ai->rule_weights[i][0] +
                    ai->rule_weights[i][1] * inputs[0] +
                    ai->rule_weights[i][2] * inputs[1];

        // Cộng dồn có trọng số
        estimated_fd += w_norm * f_i;
    }

    return estimated_fd;
}

// Giải phóng bộ nhớ
void ai_estimator_free(AnfisEstimator *ai)
{
    for (int i = 0; i < ai->num_inputs; i++)
        free(ai->input_mfs[i]);
    free(ai->input_mfs);

    for (int i = 0; i < ai->num_rules; i++)
        free(ai->rule_weights[i]);
    free(ai->rule_weights);

    free(ai);
}