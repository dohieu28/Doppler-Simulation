#include "..\inc\config_utils.h"
#include <math.h>

// Ước lượng Doppler truyền thống bằng pha của DMRS
float estimate_doppler_traditional(float complex *rx_grid, float complex *tx_dmrs3, float complex *tx_dmrs11, float fs)
{
    float complex correlation = 0;

    // Rút trích DMRS thu được từ lưới tại symbol 3 và 11
    for (int sc = 0; sc < NUM_SUBCARRIERS; sc++)
    {
        float complex rx_sym3 = rx_grid[2 * FFT_SIZE + sc];
        float complex rx_sym11 = rx_grid[10 * FFT_SIZE + sc];

        // Loại bỏ ảnh hưởng của chuỗi Gold ban đầu bằng cách nhân với liên hợp phức của TX DMRS
        float complex h3 = rx_sym3 * conj(tx_dmrs3[sc]);
        float complex h11 = rx_sym11 * conj(tx_dmrs11[sc]);

        // Tính tương quan chéo để tìm sự xoay pha
        correlation += h11 * conj(h3);
    }

    // Tính góc lệch pha delta_theta
    float delta_theta = carg(correlation);

    // Khoảng thời gian giữa Symbol 3 và Symbol 11 (8 symbols)
    float delta_t = 8 * SYMBOL_LEN / fs;

    // f_D = delta_theta / (2 * PI * delta_t)
    float estimated_fd = delta_theta / (2 * PI * delta_t);

    // Đảm bảo f_D dương
    return fabsf(estimated_fd);
}