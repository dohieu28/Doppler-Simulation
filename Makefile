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

COMMON_SRCS = \
   $(SRC_DIR)/estimators.c \
   $(SRC_DIR)/config_utils.c \
   $(SRC_DIR)/simulation.c \
   $(SRC_DIR)/channel_model.c \
   $(SRC_DIR)/modulation.c \
   $(SRC_DIR)/ofdm_modem.c \
   $(SRC_DIR)/resource_grid.c \
   $(SRC_DIR)/ai_estimator.c 

# Target 1
SIM_SRCS = $(SRC_DIR)/main.c $(COMMON_SRCS)

SIM_OBJS = $(SIM_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Target 2
DATA_SRCS = $(SRC_DIR)/generate_data.c $(COMMON_SRCS)

DATA_OBJS = $(DATA_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Tên file chạy thành phẩm
TARGET = 5g_sim

# Lệnh mặc định khi gõ "make"
all: $(TARGET)

# Quy tắc gom các file .o lại thành file 5g_sim
$(TARGET): $(SIM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Bien dich thanh cong! Chay bang lenh: ./$(TARGET)"


dataset: $(DATA_OBJS)
	$(CC) -o generate_data $^ $(LDFLAGS)

# Quy tắc biên dịch từng file .c thành file .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Lệnh dọn dẹp (Gõ "make clean" để xóa các file đã biên dịch)
clean:
	rm -rf $(OBJ_DIR) $(TARGET) generate_data
	@echo "Da xoa cac file bien dich."