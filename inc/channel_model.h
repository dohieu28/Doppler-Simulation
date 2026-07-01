// Kênh truyền HST, TDL-C

#ifndef CHANNEL_MODEL_H
#define CHANNEL_MODEL_H

#include "config_utils.h"

// Struct mô phỏng một đường truyền (Tap) trong mô hình TDL-C
typedef struct
{
    int delay_samples;         // Trễ tính theo số lượng mẫu (mức lấy mẫu FFT)
    float power_linear;        // Công suất trung bình của tia
    float complex fading_coef; // Hệ số fading phức tại một thời điểm
} ChannelTap;

typedef struct
{
    int num_taps;
    ChannelTap *taps;
    float max_doppler_shift; // Tính từ vận tốc > 350km/h

    // Mảng lưu tín hiệu miền thời gian (61440 mẫu)
    float complex *time_signal_in;
    float complex *time_signal_out;

    // Cấu trúc ma trận Toeplitz dành cho DSP (Tùy chọn nếu tối ưu hóa phép chập)
    float complex **toeplitz_matrix;
} ChannelModel;

// Khai báo hàm
ChannelModel *channel_init(float velocity, float carrier_freq);
void channel_apply_tdlc_and_hst(ChannelModel *ch, float complex *tx_sig, float complex *rx_sig, int len);
void channel_add_awgn(float complex *sig, int len, float snr_db);
void channel_free(ChannelModel *ch);

#endif