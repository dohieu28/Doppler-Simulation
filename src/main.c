#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "..\inc\config_utils.h"
#include "..\inc\simulation.h"

// extern void generate_random_bits(uint8_t *bits, int num_bits);
// extern void modulate_16qam(uint8_t *bits, float complex *symbols, int num_syms);
// extern void modulate_qpsk(uint8_t *bits, float complex *symbols, int num_syms);
// extern void modulate_64qam(uint8_t *bits, float complex *symbols, int num_syms);
// extern void generate_dmrs_qpsk(int num_sc, uint32_t c_init, float complex *dmrs_col);
// extern void map_to_grid(float complex *grid, float complex *data_syms, uint32_t cell_id);
// extern void ofdm_modulate(float complex *grid_in, float complex *time_out);
// extern void ofdm_demodulate(float complex *time_in, float complex *grid_out);
// extern void apply_hst_doppler(float complex *sig, int len, float velocity, float fc, float fs);
// extern void apply_tdlc_channel(float complex *sig_in, float complex *sig_out, int len);
// extern void add_awgn(float complex *sig, int len, int snr_db);
// extern float estimate_doppler_traditional(float complex *rx_grid, float complex *tx_dmrs3, float complex *tx_dmrs11, float fs);
// extern void extract_data_from_grid(float complex *rx_grid, float complex *rx_data_syms);
// extern void demodulate_16qam_hard(float complex *rx_symbols, uint8_t *out_bits, int num_syms);
// extern void demodulate_qpsk_hard(float complex *rx_symbols, uint8_t *out_bits, int num_syms);
// extern void demodulate_64qam_hard(float complex *rx_symbols, uint8_t *out_bits, int num_syms);
// extern void run_simulation(ModulationType mod_type, const char *output_file);

// // int main()
// {
//     srand(time(NULL));

//     SimConfig config = {
//         .velocity_kmph = 360.0f,
//         .carrier_freq = 3.5e9,
//         .fs = 122.88e6};

//     // Tính Doppler lý thuyết để so sánh
//     float true_fd = (config.velocity_kmph / 3.6f) * config.carrier_freq / 300000000.0f;
//     int num_data_syms = NUM_SUBCARRIERS * (NUM_SYMBOLS - 2);

//     uint8_t *tx_bits = malloc(num_data_syms * 4 * sizeof(uint8_t));
//     uint8_t *rx_bits = malloc(num_data_syms * 4 * sizeof(uint8_t));
//     float complex *tx_qam = malloc(num_data_syms * sizeof(float complex));
//     float complex *tx_grid = calloc(NUM_SYMBOLS * FFT_SIZE, sizeof(float complex));
//     float complex *rx_grid = calloc(NUM_SYMBOLS * FFT_SIZE, sizeof(float complex));
//     float complex *time_sig = malloc(TOTAL_SAMPLES * sizeof(float complex));
//     float complex *rx_time_sig = malloc(TOTAL_SAMPLES * sizeof(float complex));
//     float complex *rx_qam = malloc(num_data_syms * sizeof(float complex));

//     // Mảng chứa DMRS chuẩn để phía thu dùng cho việc tương quan
//     float complex tx_dmrs3[NUM_SUBCARRIERS];
//     float complex tx_dmrs11[NUM_SUBCARRIERS];
//     generate_dmrs_qpsk(NUM_SUBCARRIERS, 100 + 3, tx_dmrs3);
//     generate_dmrs_qpsk(NUM_SUBCARRIERS, 100 + 11, tx_dmrs11);

//     FILE *fp = fopen("output_mse.csv", "w");
//     if (fp == NULL)
//     {
//         printf("Loi: Khong the tao file CSV!\n");
//         return -1;
//     }
//     fprintf(fp, "SNR_dB,True_Doppler_Hz,Est_Doppler_Hz,MSE\n");

//     printf("Bat dau mo phong 5G PUSCH Channel...\n");
//     printf("Van toc: %.0f km/h | Tan so mang: 3.5 GHz | fD ly thuyet: %.2f Hz\n\n", config.velocity_kmph, true_fd);

//     // Vòng lặp quét SNR từ 0 đến 30, khoảng cách 2dB
//     for (int snr = 0; snr <= 30; snr += 2)
//     {
//         config.snr_db = snr;
//         float total_mse = 0;
//         float avg_est_fd = 0;
//         int num_monte_carlo = NUM_MONTE_CARLO;

//         for (int mc = 0; mc < num_monte_carlo; mc++)
//         {
//             // 1. Phía phát
//             generate_random_bits(tx_bits, num_data_syms * 4);
//             modulate_16qam(tx_bits, tx_qam, num_data_syms);
//             map_to_grid(tx_grid, tx_qam, 100);
//             ofdm_modulate(tx_grid, time_sig);

//             // 2. Kênh truyền
//             apply_hst_doppler(time_sig, TOTAL_SAMPLES, config.velocity_kmph, config.carrier_freq, config.fs);
//             apply_tdlc_channel(time_sig, rx_time_sig, TOTAL_SAMPLES);
//             add_awgn(rx_time_sig, TOTAL_SAMPLES, config.snr_db);

//             // 3. Phía thu & Giải điều chế
//             ofdm_demodulate(rx_time_sig, rx_grid);

//             // 4. Ước lượng Doppler
//             float est_fd = estimate_doppler_traditional(rx_grid, tx_dmrs3, tx_dmrs11, config.fs);

//             avg_est_fd += est_fd;
//             float error = est_fd - true_fd;
//             total_mse += (error * error);

//             // 5. Bóc tách dữ liệu
//             extract_data_from_grid(rx_grid, rx_qam);
//             // (Lưu ý: Trong thực tế, ở bước này tín hiệu rx_qam cần được đưa qua
//             // bộ cân bằng kênh (Channel Equalizer) để khử nhiễu pha trước khi giải mã.
//             // Nếu không có Equalizer, BER sẽ rất cao do pha đã bị xoay bởi Doppler).

//             // 6. Giải mã Hard-bit 16QAM
//             demodulate_16qam_hard(rx_qam, rx_bits, num_data_syms);

//             // 7. (Tùy chọn) Tính tỷ lệ lỗi bit BER cho 1 vòng lặp
//             int bit_errors = 0;
//             int total_bits = num_data_syms * 4;
//             for (int b = 0; b < total_bits; b++)
//             {
//                 if (tx_bits[b] != rx_bits[b])
//                 {
//                     bit_errors++;
//                 }
//             }
//         }

//         // Tính trung bình
//         avg_est_fd /= num_monte_carlo;
//         total_mse /= num_monte_carlo;

//         // Ghi ra màn hình và file CSV
//         printf("SNR: %2d dB | Est fD: %7.2f Hz | MSE: %f\n", snr, avg_est_fd, total_mse);
//         fprintf(fp, "%d,%.2f,%.2f,%f\n", snr, true_fd, avg_est_fd, total_mse);
//     }

//     fclose(fp);
//     free(tx_bits);
//     free(tx_qam);
//     free(tx_grid);
//     free(rx_grid);
//     free(time_sig);
//     free(rx_time_sig);
//     free(rx_qam);
//     free(rx_bits);

//     printf("\nMo phong hoan tat! Ket qua da luu vao 'output_mse.csv'\n");
//     return 0;
// }

int main()
{
    srand(time(NULL));

    run_simulation(MODQPSK,
                   "output/output_qpsk.csv");

    run_simulation(MODD16QAM,
                   "output/output_16qam.csv");

    run_simulation(MODD64QAM,
                   "output/output_64qam.csv");

    printf("Done!\n");
}