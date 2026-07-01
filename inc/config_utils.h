// Cấu hình tham số hệ thống và Macros

#ifndef CONFIG_UTILS_H
#define CONFIG_UTILS_H

#include <complex.h>
#include <stdint.h>

#define PI 3.14159265358979323846

#define NUM_SUBCARRIERS 3276
#define NUM_SYMBOLS 14
#define FFT_SIZE 4096
#define CP_LEN 288                               // Độ dài Cyclic Prefix mặc định
#define SYMBOL_LEN (FFT_SIZE + CP_LEN)           // Chiều dài 1 symbol miền thời gian
#define TOTAL_SAMPLES (SYMBOL_LEN * NUM_SYMBOLS) // 61440 mẫu
#define NUM_MONTE_CARLO 10000

// Hằng số cho 16QAM, QPSK và 64QAM
#define QAM16_NORM_FACTOR 0.316227766f // 1/sqrt(10)
#define QPSK_NORM_FACTOR 0.70710678f   // 1/sqrt(2)
#define QAM64_NORM_FACTOR 0.15430335f  // 1/sqrt(42)

typedef enum
{
    MOD_QPSK = 2,
    MOD_16QAM = 4,
    MOD_64QAM = 6
} ModScheme;

typedef struct
{
    ModScheme modulation;
    float velocity_kmph; // Vận tốc > 350 km/h
    float carrier_freq;  // Tần số sóng mang để tính Doppler
    float fs;            // Tần số lấy mẫu
    float snr_db;        // Tỷ số tín hiệu trên nhiễu
} SimConfig;

#endif