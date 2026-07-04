# Huấn luyện AI và xuất trọng số
import torch
import numpy as np
import pandas as pd
import torch.optim as optim
import torch.nn as nn


# 1. ĐỊNH NGHĨA KIẾN TRÚC ANFIS (Khớp 100% với file C)


class ANFIS(nn.Module):
    def __init__(self):
        super(ANFIS, self).__init__()
        # Số lượng hàm mờ (MF) cho mỗi đầu vào
        self.num_mfs = 3

        # Tham số cho Lớp 1 (Mờ hóa): Mean và Sigma cho 2 Inputs (Khởi tạo ngẫu nhiên nhưng có chủ ý)
        # Input 1 (Góc pha): Phân bố quanh 0, PI/2, PI
        self.mu_1 = nn.Parameter(torch.tensor([0.0, 1.57, 3.14]))
        self.sigma_1 = nn.Parameter(torch.tensor([0.5, 0.5, 0.5]))

        # Input 2 (Biên độ): Phân bố quanh 0, 0.5, 1.0
        self.mu_2 = nn.Parameter(torch.tensor([0.0, 0.5, 1.0]))
        self.sigma_2 = nn.Parameter(torch.tensor([0.2, 0.2, 0.2]))

        # Tham số cho Lớp 4 (Takagi-Sugeno): f = p0 + p1*x1 + p2*x2
        # Có 3x3 = 9 luật, mỗi luật có 3 hệ số (p0, p1, p2)
        self.rule_weights = nn.Parameter(torch.randn(9, 3) * 100.0)

    def forward(self, x):
        x1 = x[:, 0]
        x2 = x[:, 1]

        # Lớp 1: Tính giá trị Gauss
        mu1 = torch.exp(-0.5 * ((x1.unsqueeze(1) -
                        self.mu_1) / self.sigma_1)**2)
        mu2 = torch.exp(-0.5 * ((x2.unsqueeze(1) -
                        self.mu_2) / self.sigma_2)**2)

        # Lớp 2 & 3: Nhân luật và Chuẩn hóa (w_norm)
        w = torch.cat([
            (mu1[:, i] * mu2[:, j]).unsqueeze(1)
            for i in range(self.num_mfs) for j in range(self.num_mfs)
        ], dim=1)
        w_norm = w / (w.sum(dim=1, keepdim=True) + 1e-6)

        # Lớp 4: Hàm kết luận tuyến tính
        # f_i = p0 + p1*x1 + p2*x2
        f = torch.zeros(x.shape[0], 9, device=x.device)
        for i in range(9):
            f[:, i] = self.rule_weights[i, 0] + \
                self.rule_weights[i, 1] * x1 + self.rule_weights[i, 2] * x2

        # Lớp 5: Tính tổng (Defuzzification)
        out = torch.sum(w_norm * f, dim=1)
        return out


# 2. LOAD DATA
print("Đang đọc dữ liệu từ ..\\train_data.csv...")
df = pd.read_csv("..\\train_data.csv")
X_train = torch.tensor(
    df[['Delta_Theta', 'Magnitude']].values, dtype=torch.float32)
y_train = torch.tensor(df['True_Doppler'].values, dtype=torch.float32)

# Khởi tạo mô hình
model = ANFIS()
criterion = nn.MSELoss()
# Learning rate lớn vì fD có giá trị hàng nghìn Hz
optimizer = optim.Adam(model.parameters(), lr=1.0)

# 3. HUẤN LUYỆN (TRAINING LOOP)
epochs = 500
print("Bắt đầu huấn luyện mạng ANFIS...")
for epoch in range(epochs):
    optimizer.zero_grad()
    predictions = model(X_train)
    loss = criterion(predictions, y_train)
    loss.backward()
    optimizer.step()

    if (epoch+1) % 100 == 0:
        print(f"Epoch {epoch+1}/{epochs} | MSE Loss: {loss.item():.4f}")

# 4. XUẤT TRỌNG SỐ CHO CODE C (EXPORT)
print("\nHuấn luyện xong. Đang xuất trọng số ra file 'anfis_weights.txt'...")
with open("anfis_weights.txt", "w") as f:
    # Ghi MF của Input 1 (Góc pha)
    for i in range(3):
        f.write(f"{model.mu_1[i].item()} {model.sigma_1[i].item()}\n")

    # Ghi MF của Input 2 (Biên độ)
    for i in range(3):
        f.write(f"{model.mu_2[i].item()} {model.sigma_2[i].item()}\n")

    # Ghi 9 bộ hệ số Luật (p0, p1, p2)
    for i in range(9):
        f.write(
            f"{model.rule_weights[i, 0].item()} {model.rule_weights[i, 1].item()} {model.rule_weights[i, 2].item()}\n")

print("Hoàn tất! Hãy copy file 'anfis_weights.txt' vào thư mục code C của bạn.")
