#include "..\inc\config_utils.h"
#include <math.h>

// Hàm thực hiện phép biến đổi Fourier nhanh (Radix-2 FFT)
// is_inverse = 1 -> IFFT, is_inverse = 0 -> FFT
void simple_fft(float complex *buf, int n, int is_inverse)
{
    // Đảo ngược bit (Bit-reversal)
    for (int i = 1, j = 0; i < n; i++)
    {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
        {
            float complex temp = buf[i];
            buf[i] = buf[j];
            buf[j] = temp;
        }
    }
    // Tính toán Butterfly
    for (int len = 2; len <= n; len <<= 1)
    {
        float angle = 2 * PI / len * (is_inverse ? 1 : -1);
        float complex wlen = cos(angle) + I * sin(angle);
        for (int i = 0; i < n; i += len)
        {
            float complex w = 1.0f;
            for (int j = 0; j < len / 2; j++)
            {
                float complex u = buf[i + j];
                float complex v = buf[i + j + len / 2] * w;
                buf[i + j] = u + v;
                buf[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    if (is_inverse)
    {
        for (int i = 0; i < n; i++)
            buf[i] /= n;
    }
}

// Biến đổi lưới tài nguyên thành tín hiệu miền thời gian có chèn CP
void ofdm_modulate(float complex *grid_in, float complex *time_out)
{
    float complex symbol_buf[FFT_SIZE];

    for (int sym = 0; sym < NUM_SYMBOLS; sym++)
    {
        // Copy 1 cột của lưới vào buffer để IFFT
        for (int i = 0; i < FFT_SIZE; i++)
        {
            symbol_buf[i] = grid_in[sym * FFT_SIZE + i];
        }

        simple_fft(symbol_buf, FFT_SIZE, 1); // IFFT

        // Chèn Cyclic Prefix: Copy đoạn đuôi lên đầu
        int out_offset = sym * SYMBOL_LEN;
        for (int i = 0; i < CP_LEN; i++)
        {
            time_out[out_offset + i] = symbol_buf[FFT_SIZE - CP_LEN + i];
        }
        // Nối tiếp phần data chính
        for (int i = 0; i < FFT_SIZE; i++)
        {
            time_out[out_offset + CP_LEN + i] = symbol_buf[i];
        }
    }
}

// Biến đổi tín hiệu miền thời gian về lại lưới tài nguyên (Loại bỏ CP)
void ofdm_demodulate(float complex *time_in, float complex *grid_out)
{
    float complex symbol_buf[FFT_SIZE];

    for (int sym = 0; sym < NUM_SYMBOLS; sym++)
    {
        int in_offset = sym * SYMBOL_LEN;

        // Bỏ qua đoạn CP (CP_LEN mẫu đầu tiên), chỉ lấy phần data chính (FFT_SIZE mẫu)
        for (int i = 0; i < FFT_SIZE; i++)
        {
            symbol_buf[i] = time_in[in_offset + CP_LEN + i];
        }

        // Thực hiện FFT (is_inverse = 0)
        simple_fft(symbol_buf, FFT_SIZE, 0);

        // Đưa dữ liệu trở lại lưới
        for (int i = 0; i < FFT_SIZE; i++)
        {
            grid_out[sym * FFT_SIZE + i] = symbol_buf[i];
        }
    }
}