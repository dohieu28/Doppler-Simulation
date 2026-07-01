#include "..\inc\config_utils.h"
#include "..\inc\channel_model.h"
#include <stdlib.h>
#include <math.h>

// 1. Thêm hiệu ứng Doppler (Xoay pha miền thời gian)
void apply_hst_doppler(float complex *sig, int len, float velocity_kmph, float fc, float fs)
{
    float fd = (velocity_kmph / 3.6f) * fc / 300000000.0f; // f_D = v/c * fc
    for (int i = 0; i < len; i++)
    {
        float t = (float)i / fs;
        float phase = 2 * PI * fd * t;
        sig[i] *= (cos(phase) + I * sin(phase));
    }
}

// 2. Chập tín hiệu qua kênh TDL-C (Đơn giản hóa với 3 tap để mô phỏng)
void apply_tdlc_channel(float complex *sig_in, float complex *sig_out, int len)
{
    // TDL-C Profile thu gọn: Trễ (mẫu) và Biên độ (tuyến tính)
    int delays[] = {0, 5, 12};
    float gains[] = {0.8f, 0.4f, 0.1f}; // NLOS fading tĩnh

    // Khởi tạo sig_out = 0
    for (int i = 0; i < len; i++)
        sig_out[i] = 0;

    for (int i = 0; i < len; i++)
    {
        for (int tap = 0; tap < 3; tap++)
        {
            if (i >= delays[tap])
            {
                sig_out[i] += sig_in[i - delays[tap]] * gains[tap];
            }
        }
    }
}

// 3. Thêm nhiễu Trắng (AWGN) qua thuật toán Box-Muller
void add_awgn(float complex *sig, int len, int snr_db)
{
    // Tính công suất tín hiệu
    float signal_power = 0;
    for (int i = 0; i < len; i++)
    {
        signal_power += pow(cabs(sig[i]), 2);
    }
    signal_power /= len;

    // Tính phương sai nhiễu từ SNR
    float snr_linear = pow(10, snr_db / 10.0f);
    float noise_var = signal_power / snr_linear;
    float noise_std = sqrt(noise_var / 2.0f); // Chia 2 vì nhiễu có cả phần thực và ảo

    for (int i = 0; i < len; i++)
    {
        // Sinh 2 số ngẫu nhiên phân bố đều (0,1]
        float u1 = ((float)rand() / RAND_MAX) + 1e-10f;
        float u2 = ((float)rand() / RAND_MAX) + 1e-10f;

        // Box-Muller transform tạo phân bố chuẩn (Gauss)
        float z0 = sqrt(-2.0f * log(u1)) * cos(2 * PI * u2);
        float z1 = sqrt(-2.0f * log(u1)) * sin(2 * PI * u2);

        sig[i] += noise_std * (z0 + I * z1);
    }
}