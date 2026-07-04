#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "..\inc\config_utils.h"

extern void generate_dmrs_qpsk(int num_sc, uint32_t c_init, float complex *dmrs_col);
extern void apply_hst_doppler(float complex *sig, int len, float velocity, float fc, float fs);
extern void apply_tdlc_channel(float complex *sig_in, float complex *sig_out, int len);
extern void add_awgn(float complex *sig, int len, int snr_db);

int main()
{
    srand(time(NULL));

    SimConfig config = {.carrier_freq = 3.5e9, .fs = 122.88e6};

    float complex *tx_dmrs3 = malloc(NUM_SUBCARRIERS * sizeof(float complex));
    float complex *tx_dmrs11 = malloc(NUM_SUBCARRIERS * sizeof(float complex));
    generate_dmrs_qpsk(NUM_SUBCARRIERS, 103, tx_dmrs3);
    generate_dmrs_qpsk(NUM_SUBCARRIERS, 111, tx_dmrs11);

    FILE *fp = fopen("train_data.csv", "w");
    fprintf(fp, "Delta_Theta,Magnitude,True_Doppler\n");

    printf("Bat dau tao tap du lieu huan luyen ANFIS...\n");

    // Quét vận tốc từ 100 km/h đến 500 km/h (bước nhảy 10)
    for (float v = 100; v <= 500; v += 10)
    {
        float true_fd = (v / 3.6f) * config.carrier_freq / 300000000.0f;

        // Quét SNR từ -5 dB đến 30 dB (Tập trung nhiều vào SNR thấp để AI học chống nhiễu)
        for (int snr = -5; snr <= 30; snr += 5)
        {

            // 20 mẫu Monte Carlo cho mỗi cấu hình
            for (int mc = 0; mc < 20; mc++)
            {
                // Tái tạo lại tín hiệu DMRS lý tưởng ở miền thời gian (Giả lập nhanh)
                // Trong thực tế, bạn sẽ lấy từ lưới ofdm_modulate
                float complex time_sig[TOTAL_SAMPLES] = {0};
                // ... (Giả sử time_sig chứa symbol OFDM của 2 DMRS) ...

                // Áp dụng kênh truyền
                float complex rx_time_sig[TOTAL_SAMPLES] = {0};
                apply_hst_doppler(time_sig, TOTAL_SAMPLES, v, config.carrier_freq, config.fs);
                apply_tdlc_channel(time_sig, rx_time_sig, TOTAL_SAMPLES);
                add_awgn(rx_time_sig, TOTAL_SAMPLES, snr);

                // TRÍCH XUẤT ĐẶC TRƯNG (Mô phỏng sau khi FFT thu được rx_grid)
                // Đoạn này lấy giá trị tương quan (Correlation)
                float complex correlation = 0;
                for (int sc = 0; sc < NUM_SUBCARRIERS; sc++)
                {
                    // MOCK: Giả lập sự thay đổi pha do Doppler + Nhiễu
                    float noise_phase = ((float)rand() / RAND_MAX - 0.5f) * (30.0f / (snr + 10.0f));
                    float phase = 2 * PI * true_fd * (8 * SYMBOL_LEN / config.fs) + noise_phase;
                    float mag = 1.0f - ((float)rand() / RAND_MAX) * (15.0f / (snr + 20.0f));

                    correlation += (mag * cos(phase) + I * mag * sin(phase));
                }

                // x1: Góc pha (Từ 0 đến PI)
                float delta_theta = fabsf(carg(correlation));
                // x2: Biên độ trung bình
                float magnitude = cabs(correlation) / NUM_SUBCARRIERS;

                // Ghi vào Dataset
                fprintf(fp, "%f,%f,%f\n", delta_theta, magnitude, true_fd);
            }
        }
    }
    fclose(fp);
    free(tx_dmrs3);
    free(tx_dmrs11);
    printf("Hoan thanh! Da xuat ra file 'train_data.csv'\n");
    return 0;
}