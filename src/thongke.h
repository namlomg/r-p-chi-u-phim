/* thongke.h - Thống kê doanh thu theo suất và theo phim. */
#ifndef THONGKE_H
#define THONGKE_H

#include "mohinh.h"

typedef struct PhimDoanhThu {
    char maPhim[10];
    char tenPhim[180];
    int soSuat, soVe;
    long long doanhThu;
    struct PhimDoanhThu *next;
} PhimDoanhThu;

long long tk_doanh_thu_suat(const SuatChieu *s);
long long tk_tong_doanh_thu(const SuatChieu *dau);
int tk_tong_ve(const SuatChieu *dau);
int tk_tong_ghe(const SuatChieu *dau);

/* Tạo danh sách phim theo doanh thu giảm dần. Trả về 0 nếu thành công, -1 nếu hết bộ nhớ. */
int tk_theo_phim(const SuatChieu *dau, PhimDoanhThu **ra);
void tk_giai_phong(PhimDoanhThu **dau);

#endif
