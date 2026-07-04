// Ước lượng Doppler sử dụng AI

#ifndef AI_ESTIMATOR_H
#define AI_ESTIMATOR_H

#include <complex.h>

// Cấu trúc Hàm thuộc tính Gauss (Gaussian Membership Function)
typedef struct
{
    float mean;  // Giá trị trung tâm (c)
    float sigma; // Độ lệch chuẩn (độ rộng của chuông Gauss)
} GaussMF;

// Cấu trúc mạng ANFIS
typedef struct
{
    int num_inputs;        // Số lượng đầu vào
    int num_mfs_per_input; // Số lượng hàm mờ cho mỗi đầu vào
    int num_rules;         // Số lượng luật

    // Con trỏ 2D lưu các hàm mờ: [input_index][mf_index]
    GaussMF **input_mfs;

    // Con trỏ 2D lưu trọng số của hàm kết luận (Takagi-Sugeno): [rule_index][3]
    // Hàm kết luận có dạng: f = p0 + p1*x1 + p2*x2
    float **rule_weights;

} AnfisEstimator;

// Các hàm giao tiếp
AnfisEstimator *ai_estimator_init(const char *weight_file_path);
float ai_predict_doppler(AnfisEstimator *ai, float complex *rx_grid, float complex *tx_dmrs3, float complex *tx_dmrs11, int num_sc, float fs);
void ai_estimator_free(AnfisEstimator *ai);

#endif