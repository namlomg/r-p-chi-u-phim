/* suatchieu.h - Quản lý danh sách liên kết các suất chiếu. */
#ifndef SUATCHIEU_H
#define SUATCHIEU_H

#include "mohinh.h"

typedef enum {
    SC_OK,
    SC_KHONG_HOP_LE,
    SC_TRUNG_MA,
    SC_TRUNG_LICH,
    SC_PHIM_KHAC_TEN,
    SC_HET_BO_NHO
} KetQuaSuat;

/* Thêm bản sao của mau, giữ danh sách theo thứ tự thời gian.
   Nếu trùng lịch và trung khác NULL, *trung trỏ tới suất bị trùng. */
KetQuaSuat sc_them(SuatChieu **dau, const SuatChieu *mau, const SuatChieu **trung);
SuatChieu *sc_tim(SuatChieu *dau, const char *maSuat);
const char *sc_ten_phim(const SuatChieu *dau, const char *maPhim);
int sc_dem(const SuatChieu *dau);
void sc_giai_phong(SuatChieu **dau);

#endif
