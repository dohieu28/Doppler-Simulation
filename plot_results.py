# Đọc file .csv và vẽ biểu đồ so sánh
import pandas as pd
import matplotlib.pyplot as plt
import os
import math

# 1. Đọc dữ liệu từ file CSV do chương trình C xuất ra
csv_files = ["output/output_ai_qpsk.csv",
             "output/output_ai_16qam.csv", "output/output_ai_64qam.csv"]
dfs = []

for csv_file in csv_files:
    if not os.path.exists(csv_file):
        print(
            f"Lỗi: Không tìm thấy file '{csv_file}'.")
        exit()

    df = pd.read_csv(csv_file)
    dfs.append(df)

    img_extension = 'qpsk' if 'qpsk' in csv_file else '16qam' if '16qam' in csv_file else '64qam'

 # 2. Thiết lập khung hình cho biểu đồ (gồm 2 biểu đồ phụ)
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    fig.suptitle('Đánh Giá Hiệu Năng Ước Lượng Doppler (Kênh HST + TDL-C) - ' + img_extension.upper(),
                 fontsize=16, fontweight='bold')

# --- Biểu đồ 1: Tần số Doppler Lý thuyết vs Ước lượng ---
    # ax1.plot(df['SNR_dB'], df['True_Doppler_Hz'], 'g--',
    #          linewidth=2, label='Doppler Lý Thuyết (True)')
    ax1.plot(df['SNR_dB'], (df['Est_Doppler_Hz'] - df['True_Doppler_Hz']).abs(), 'b-o', linewidth=2,
             markersize=6, label='Doppler Ước Lượng (Estimated)'),
    ax1.plot(df['SNR_dB'], (df['AI_Doppler_Hz'] - df['True_Doppler_Hz']).abs(), 'r-s', linewidth=2,
             markersize=6, label='Doppler Ước Lượng AI (ANFIS)')
    ax1.set_title('Sai lệch Tần số Doppler qua các mức SNR')
    ax1.set_xlabel('Tỷ số Tín hiệu trên Nhiễu - SNR (dB)', fontsize=12)
    ax1.set_ylabel('Tần số Doppler (Hz)', fontsize=12)
    ax1.grid(True, linestyle='--', alpha=0.7)
    ax1.legend(loc='best', fontsize=11)

# --- Biểu đồ 2: Mean Squared Error (MSE) ---
# Vẽ MSE trên thang đo Logarithmic để thấy rõ sự cải thiện ở SNR cao
    ax2.plot(df['SNR_dB'], df['MSE'], 'r-s', linewidth=2,
             markersize=6, label='Sai số toàn phương trung bình (MSE) -' + img_extension.upper())
    ax2.plot(df['SNR_dB'], df['MSE_AI'], 'b-o', linewidth=2,
             markersize=6, label='MSE của AI (ANFIS) -' + img_extension.upper())
    ax2.set_yscale('log')
    ax2.set_title('Độ Lỗi MSE của Thuật Toán Ước Lượng')
    ax2.set_xlabel('Tỷ số Tín hiệu trên Nhiễu - SNR (dB)', fontsize=12)
    ax2.set_ylabel('MSE (Log scale)', fontsize=12)
    ax2.grid(True, which="both", linestyle='--', alpha=0.7)
    ax2.legend(loc='best', fontsize=11)

    plt.tight_layout()
    plt.subplots_adjust(top=0.88)  # Tạo không gian cho title tổng

 # Lưu biểu đồ ra file ảnh định dạng chất lượng cao

    output_img = f'output/doppler_ai_evaluation_{img_extension}.png'
    plt.savefig(output_img, dpi=300)
    print(f"Đã lưu biểu đồ thành công vào: {output_img}")

 # Hiển thị biểu đồ lên màn hình
    plt.show()

dfs = [pd.read_csv(f) for f in csv_files]


fig, ax = plt.subplots(figsize=(8, 6))

styles = {
    "qpsk":  ("m-^", "QPSK"),
    "16qam": ("c-o", "16QAM"),
    "64qam": ("y-s", "64QAM")
}

for df, csv_file in zip(dfs, csv_files):
    name = csv_file.lower()

    for key, (style, label) in styles.items():
        if key in name:
            ax.plot(df["SNR_dB"],
                    df["BER"],
                    style,
                    linewidth=2,
                    markersize=6,
                    label=label)
            break

ax.set_title("So sánh BER của các phương pháp điều chế")
ax.set_xlabel("SNR (dB)")
ax.set_ylabel("BER")
ax.set_yscale("log")
ax.grid(True, which="both", linestyle="--", alpha=0.7)
ax.legend()
plt.savefig("output/ber_comparison.png", dpi=300)

plt.show()
