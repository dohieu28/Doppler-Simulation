# Khai báo trình biên dịch
CC = gcc

# Các cờ biên dịch (Flags)
# -Wall: Bật cảnh báo lỗi
# -O3: Tối ưu hóa tốc độ chạy mã C ở mức cao nhất (Rất cần cho vòng lặp Monte Carlo)
# -I./inc: Chỉ định thư mục chứa các file .h
CFLAGS = -Wall -O3 -I./inc

# Thư viện liên kết (-lm là thư viện Toán học)
LDFLAGS = -lm

# Tên thư mục
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = inc

# Tìm tất cả các file .c trong thư mục src/
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Tạo danh sách các file .o tương ứng trong thư mục obj/
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Tên file chạy thành phẩm
TARGET = 5g_sim

# Lệnh mặc định khi gõ "make"
all: $(TARGET)

# Quy tắc gom các file .o lại thành file 5g_sim
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Bien dich thanh cong! Chay bang lenh: ./$(TARGET)"

# Quy tắc biên dịch từng file .c thành file .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
# 	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Lệnh dọn dẹp (Gõ "make clean" để xóa các file đã biên dịch)
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
#     rmdir /S /Q obj
#     del 5g_sim.exe
	@echo "Da xoa cac file bien dich."