#include <stdio.h>
#include <stdint.h>
#include <complex.h>
#include <math.h>
#include "..\inc\resource_grid.h"

#define NC_OFFSET 1600 // Độ trễ khởi tạo theo chuẩn 3GPP

/**
 * @brief Hàm sinh chuỗi bit giả ngẫu nhiên (Gold sequence)
 * * @param num_bits  Số lượng bit cần sinh (Gấp đôi số lượng subcarrier)
 * @param c_init    Giá trị khởi tạo cho x2 (Phụ thuộc cấu hình hệ thống)
 * @param c_out     Mảng chứa chuỗi bit đầu ra
 */
void generate_gold_sequence(int num_bits, uint32_t c_init, uint8_t *c_out)
{
    // Khởi tạo trạng thái ban đầu của 2 LFSR (sử dụng 31 bit của uint32_t)
    uint32_t x1 = 1;      // Khởi tạo: x1(0) = 1, các bit khác = 0
    uint32_t x2 = c_init; // Khởi tạo x2 bằng c_init

    // Biến tạm để lưu bit mới tính được
    uint8_t next_x1, next_x2;

    // Chạy tổng cộng (NC_OFFSET + num_bits) vòng lặp
    for (int i = 0; i < NC_OFFSET + num_bits; i++)
    {

        // 1. Nếu đã vượt qua 1600 bit bỏ đi, tiến hành lấy bit đầu ra (bit tại vị trí 0)
        if (i >= NC_OFFSET)
        {
            c_out[i - NC_OFFSET] = (x1 & 1) ^ (x2 & 1); // XOR bit thấp nhất của x1 và x2
        }

        // 2. Tính toán bit mới sẽ được đẩy vào thanh ghi (theo phương trình 3GPP)
        // x1(n+31) = x1(n+3) + x1(n)
        next_x1 = ((x1 >> 3) ^ (x1 & 1)) & 1;

        // x2(n+31) = x2(n+3) + x2(n+2) + x2(n+1) + x2(n)
        next_x2 = ((x2 >> 3) ^ (x2 >> 2) ^ (x2 >> 1) ^ (x2 & 1)) & 1;

        // 3. Dịch phải toàn bộ thanh ghi 1 bit và chèn bit mới vào vị trí cao nhất (bit 30)
        x1 = (x1 >> 1) | ((uint32_t)next_x1 << 30);
        x2 = (x2 >> 1) | ((uint32_t)next_x2 << 30);
    }
}

/**
 * @brief Hàm tạo dãy DMRS QPSK để nhét vào cột lưới tài nguyên
 * * @param num_sc    Số lượng subcarrier (Ví dụ: 3276)
 * @param c_init    Giá trị khởi tạo
 * @param dmrs_col  Mảng số phức chứa DMRS đầu ra
 */
void generate_dmrs_qpsk(int num_sc, uint32_t c_init, float complex *dmrs_col)
{
    int num_bits = num_sc * 2; // Mỗi QPSK cần 2 bit
    uint8_t c_seq[num_bits];   // Có thể dùng malloc nếu cấp phát động

    // 1. Sinh chuỗi bit Gold
    generate_gold_sequence(num_bits, c_init, c_seq);

    // 2. Ánh xạ chuỗi bit thành ký hiệu QPSK
    float norm_factor = 1.0f / sqrtf(2.0f);

    for (int n = 0; n < num_sc; n++)
    {
        // Lấy 2 bit liên tiếp
        uint8_t bit_i = c_seq[2 * n];     // Phần thực (In-phase)
        uint8_t bit_q = c_seq[2 * n + 1]; // Phần ảo (Quadrature)

        // Công thức chuẩn: 1 -> -1, 0 -> +1
        float real_part = norm_factor * (1.0f - 2.0f * bit_i);
        float imag_part = norm_factor * (1.0f - 2.0f * bit_q);

        // Lưu vào mảng phức
        dmrs_col[n] = real_part + imag_part * I;
    }
}

// Ánh xạ Data và DMRS vào lưới (Mảng 1D giả 2D: Index = sym_idx * FFT_SIZE + sc_idx)
void map_to_grid(float complex *grid, float complex *data_syms, uint32_t cell_id)
{
    float complex dmrs3[NUM_SUBCARRIERS];
    float complex dmrs11[NUM_SUBCARRIERS];

    // C_init giả lập thay đổi theo chỉ số Symbol
    generate_dmrs_qpsk(NUM_SUBCARRIERS, cell_id + 3, dmrs3);
    generate_dmrs_qpsk(NUM_SUBCARRIERS, cell_id + 11, dmrs11);

    int data_ptr = 0;
    for (int sym = 0; sym < NUM_SYMBOLS; sym++)
    {
        for (int sc = 0; sc < NUM_SUBCARRIERS; sc++)
        {
            int grid_idx = sym * FFT_SIZE + sc; // Chừa các subcarrier trống do FFT_SIZE > NUM_SUBCARRIERS
            if (sym == 2)
            {
                grid[grid_idx] = dmrs3[sc]; // Symbol 3 (Index 2)
            }
            else if (sym == 10)
            {
                grid[grid_idx] = dmrs11[sc]; // Symbol 11 (Index 10)
            }
            else
            {
                grid[grid_idx] = data_syms[data_ptr++];
            }
        }
    }
}

// Hàm bóc tách dữ liệu từ lưới nhận được (Bỏ qua DMRS tại symbol 3 và 11)
void extract_data_from_grid(float complex *rx_grid, float complex *rx_data_syms)
{
    int data_ptr = 0;
    for (int sym = 0; sym < NUM_SYMBOLS; sym++)
    {
        // Bỏ qua cột DMRS
        if (sym == 2 || sym == 10)
            continue;

        for (int sc = 0; sc < NUM_SUBCARRIERS; sc++)
        {
            // Rút trích đúng các subcarrier chứa dữ liệu
            rx_data_syms[data_ptr++] = rx_grid[sym * FFT_SIZE + sc];
        }
    }
}