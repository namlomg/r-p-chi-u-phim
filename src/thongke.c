/* thongke.c - Thống kê doanh thu theo suất và theo phim. */
#include "thongke.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ve.h"

long long tk_doanh_thu_suat(const SuatChieu *s) { return (long long)ve_dem(s) * s->giaVe; }

long long tk_tong_doanh_thu(const SuatChieu *dau) {
    long long tong = 0;
    for (; dau; dau = dau->next) tong += tk_doanh_thu_suat(dau);
    return tong;
}

int tk_tong_ve(const SuatChieu *dau) {
    int tong = 0;
    for (; dau; dau = dau->next) tong += ve_dem(dau);
    return tong;
}

int tk_tong_ghe(const SuatChieu *dau) {
    int tong = 0;
    for (; dau; dau = dau->next) tong += dau->soHang * dau->soCot;
    return tong;
}

int tk_theo_phim(const SuatChieu *dau, PhimDoanhThu **ra) {
    PhimDoanhThu *ds = NULL, *sap_xep = NULL;

    for (; dau; dau = dau->next) {
        PhimDoanhThu *p = ds;
        while (p && strcmp(p->maPhim, dau->maPhim) != 0) p = p->next;
        if (!p) {
            p = (PhimDoanhThu *)calloc(1, sizeof *p);
            if (!p) {
                tk_giai_phong(&ds);
                *ra = NULL;
                return -1;
            }
            snprintf(p->maPhim, sizeof p->maPhim, "%s", dau->maPhim);
            snprintf(p->tenPhim, sizeof p->tenPhim, "%s", dau->tenPhim);
            p->next = ds;
            ds = p;
        }
        p->soSuat++;
        p->soVe += ve_dem(dau);
        p->doanhThu += tk_doanh_thu_suat(dau);
    }

    while (ds) {
        PhimDoanhThu *p = ds, **vt = &sap_xep;
        ds = ds->next;
        while (*vt && (*vt)->doanhThu >= p->doanhThu) vt = &(*vt)->next;
        p->next = *vt;
        *vt = p;
    }
    *ra = sap_xep;
    return 0;
}

void tk_giai_phong(PhimDoanhThu **dau) {
    while (*dau) {
        PhimDoanhThu *p = *dau;
        *dau = p->next;
        free(p);
    }
}
