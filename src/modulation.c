#include "..\inc\config_utils.h"
#include <stdlib.h>
#include <math.h>

// Tạo chuỗi bit ngẫu nhiên (0 hoặc 1)
void generate_random_bits(uint8_t *bits, int num_bits)
{
    for (int i = 0; i < num_bits; i++)
    {
        bits[i] = rand() % 2;
    }
}

// Ánh xạ chuỗi bit thành ký hiệu 16QAM theo chuẩn 3GPP
void modulate_16qam(uint8_t *bits, float complex *symbols, int num_syms)
{
    for (int i = 0; i < num_syms; i++)
    {
        uint8_t b0 = bits[4 * i];
        uint8_t b1 = bits[4 * i + 1];
        uint8_t b2 = bits[4 * i + 2];
        uint8_t b3 = bits[4 * i + 3];

        // Logic Gray mapping của 16QAM
        float real = (1.0f - 2.0f * b0) * (2.0f - (1.0f - 2.0f * b2));
        float imag = (1.0f - 2.0f * b1) * (2.0f - (1.0f - 2.0f * b3));

        symbols[i] = (real + I * imag) * QAM16_NORM_FACTOR;
    }
}

// Giải mã Hard-bit 16QAM thành chuỗi bit
void demodulate_16qam_hard(float complex *rx_symbols, uint8_t *out_bits, int num_syms)
{
    // K = 1/sqrt(10) là hằng số chuẩn hóa của 16QAM
    float norm_factor = QAM16_NORM_FACTOR;

    for (int i = 0; i < num_syms; i++)
    {
        // Bước 1: Chuẩn hóa ngược tín hiệu thu được
        float y_real = crealf(rx_symbols[i]) / norm_factor;
        float y_imag = cimagf(rx_symbols[i]) / norm_factor;

        // Bước 2: Tách bit trên Trục I (Phần thực) -> Quyết định b0 và b2
        // b0 quyết định bởi dấu (lớn hơn 0 là 0, nhỏ hơn bằng 0 là 1)
        out_bits[4 * i + 0] = (y_real > 0.0f) ? 0 : 1;
        // b2 quyết định bởi biên độ so với ngưỡng 2.0
        out_bits[4 * i + 2] = (fabsf(y_real) < 2.0f) ? 0 : 1;

        // Bước 3: Tách bit trên Trục Q (Phần ảo) -> Quyết định b1 và b3
        // b1 quyết định bởi dấu
        out_bits[4 * i + 1] = (y_imag > 0.0f) ? 0 : 1;
        // b3 quyết định bởi biên độ so với ngưỡng 2.0
        out_bits[4 * i + 3] = (fabsf(y_imag) < 2.0f) ? 0 : 1;
    }
}

// Điều chế QPSK
void modulate_qpsk(uint8_t *bits, float complex *symbols, int num_syms)
{
    for (int i = 0; i < num_syms; i++)
    {
        // Ánh xạ: 0 -> +1, 1 -> -1
        float real = (1.0f - 2.0f * bits[2 * i + 0]) * QPSK_NORM_FACTOR;
        float imag = (1.0f - 2.0f * bits[2 * i + 1]) * QPSK_NORM_FACTOR;
        symbols[i] = real + I * imag;
    }
}

// Giải điều chế QPSK (Hard-bit)
void demodulate_qpsk_hard(float complex *rx_symbols, uint8_t *out_bits, int num_syms)
{
    for (int i = 0; i < num_syms; i++)
    {
        // Chỉ cần xét dấu (lớn hơn 0 là bit 0, nhỏ hơn bằng 0 là bit 1)
        out_bits[2 * i + 0] = (crealf(rx_symbols[i]) > 0.0f) ? 0 : 1;
        out_bits[2 * i + 1] = (cimagf(rx_symbols[i]) > 0.0f) ? 0 : 1;
    }
}

// Điều chế 64QAM theo chuẩn 3GPP
void modulate_64qam(uint8_t *bits, float complex *symbols, int num_syms)
{
    for (int i = 0; i < num_syms; i++)
    {
        // Trục I: bit 0, 2, 4
        float i_val = (1.0f - 2.0f * bits[6 * i + 0]) * (4.0f - (1.0f - 2.0f * bits[6 * i + 2]) * (2.0f - (1.0f - 2.0f * bits[6 * i + 4])));

        // Trục Q: bit 1, 3, 5
        float q_val = (1.0f - 2.0f * bits[6 * i + 1]) * (4.0f - (1.0f - 2.0f * bits[6 * i + 3]) * (2.0f - (1.0f - 2.0f * bits[6 * i + 5])));

        symbols[i] = (i_val + I * q_val) * QAM64_NORM_FACTOR;
    }
}

// Giải điều chế 64QAM (Hard-bit bằng phương pháp gập không gian)
void demodulate_64qam_hard(float complex *rx_symbols, uint8_t *out_bits, int num_syms)
{
    for (int i = 0; i < num_syms; i++)
    {
        // 1. Chuẩn hóa ngược
        float y_real = crealf(rx_symbols[i]) / QAM64_NORM_FACTOR;
        float y_imag = cimagf(rx_symbols[i]) / QAM64_NORM_FACTOR;

        // 2. Tách 3 bit trục I
        out_bits[6 * i + 0] = (y_real > 0.0f) ? 0 : 1;
        out_bits[6 * i + 2] = (fabsf(y_real) < 4.0f) ? 0 : 1;
        // Thuật toán gập: Kéo các điểm về quanh gốc tọa độ để kiểm tra ngưỡng 2
        out_bits[6 * i + 4] = (fabsf(fabsf(y_real) - 4.0f) < 2.0f) ? 0 : 1;

        // 3. Tách 3 bit trục Q
        out_bits[6 * i + 1] = (y_imag > 0.0f) ? 0 : 1;
        out_bits[6 * i + 3] = (fabsf(y_imag) < 4.0f) ? 0 : 1;
        out_bits[6 * i + 5] = (fabsf(fabsf(y_imag) - 4.0f) < 2.0f) ? 0 : 1;
    }
}