/* mohinh.h - Cấu trúc dữ liệu dùng chung của chương trình. */
#ifndef MOHINH_H
#define MOHINH_H

#define SC_HANG_TOI_DA 10
#define SC_COT_TOI_DA 15
#define SC_GHE_TOI_DA (SC_HANG_TOI_DA * SC_COT_TOI_DA)

/* Một ghế đã đặt. hang tính từ 0 (A = 0), cot tính từ 1: A5 -> hang 0, cot 5. */
typedef struct GheDat {
    int hang, cot;
    char tenKhach[100];
    char sdt[12];
    struct GheDat *next;
} GheDat;

/* Một suất chiếu, kèm danh sách liên kết các ghế đã đặt. */
typedef struct SuatChieu {
    char maSuat[10], maPhim[10], tenPhim[180], phong[10];
    char ngay[11]; /* YYYY-MM-DD */
    char gio[6];   /* HH:MM */
    int thoiLuong; /* phút */
    int soHang, soCot;
    long giaVe;
    GheDat *dsGhe;
    struct SuatChieu *next;
} SuatChieu;

/* Vị trí một ghế khi người dùng đang chọn, chưa phải vé đã đặt. */
typedef struct {
    int hang, cot;
} ViTriGhe;

#endif
