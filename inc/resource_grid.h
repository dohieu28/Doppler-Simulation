// Struct lưới tài nguyên và DMRS

#ifndef RESOURCE_GRID_H
#define RESOURCE_GRID_H

#include "config_utils.h"

typedef struct
{
    // Kích thước chuẩn: 3276 x 14
    int num_sc;
    int num_sym;

    // Con trỏ lưu chuỗi bit đầu vào (kích thước thay đổi theo kiểu điều chế)
    uint8_t *tx_bits;

    // Mảng 1D lưu lưới 2D để tối ưu cache CPU.
    // Truy cập phần tử (sym, sc) bằng công thức: grid[sym * num_sc + sc]
    float complex *tx_grid;
    float complex *rx_grid; // Lưới nhận được sau giải điều chế

    // Các con trỏ lưu trữ DMRS riêng biệt để tiện bóc tách
    float complex *tx_dmrs_col_3;
    float complex *tx_dmrs_col_11;
    float complex *rx_dmrs_col_3;
    float complex *rx_dmrs_col_11;

} ResourceGrid;

// Khai báo hàm
ResourceGrid *grid_init(ModScheme mod);
void grid_map_data_and_dmrs(ResourceGrid *grid);
void grid_extract_dmrs(ResourceGrid *grid);
void grid_free(ResourceGrid *grid);

#endif