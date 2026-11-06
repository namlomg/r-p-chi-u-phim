/* suatchieu.c - Quản lý danh sách liên kết các suất chiếu. */
#include "suatchieu.h"

#include <stdlib.h>
#include <string.h>

#include "nhaplieu.h"
#include "ve.h"

static int mau_hop_le(const SuatChieu *m) {
    return nl_ma_hop_le(m->maSuat, sizeof m->maSuat - 1) &&
           nl_ma_hop_le(m->maPhim, sizeof m->maPhim - 1) &&
           nl_ma_hop_le(m->phong, sizeof m->phong - 1) &&
           nl_ten_hop_le(m->tenPhim) && nl_ngay_hop_le(m->ngay) && nl_gio_hop_le(m->gio) &&
           m->thoiLuong >= 1 && m->thoiLuong <= 600 &&
           m->soHang >= 1 && m->soHang <= SC_HANG_TOI_DA &&
           m->soCot >= 1 && m->soCot <= SC_COT_TOI_DA &&
           m->giaVe >= 1000 && m->giaVe <= 10000000;
}

static int trung_lich(const SuatChieu *a, const SuatChieu *b) {
    long long a1 = nl_phut(a->ngay, a->gio), a2 = a1 + a->thoiLuong;
    long long b1 = nl_phut(b->ngay, b->gio), b2 = b1 + b->thoiLuong;
    return strcmp(a->phong, b->phong) == 0 && a1 < b2 && b1 < a2;
}

KetQuaSuat sc_them(SuatChieu **dau, const SuatChieu *mau, const SuatChieu **trung) {
    SuatChieu *moi, **vi_tri = dau;
    const SuatChieu *p;
    long long tg = 0;

    if (!mau_hop_le(mau)) return SC_KHONG_HOP_LE;
    for (p = *dau; p; p = p->next) {
        if (strcmp(p->maSuat, mau->maSuat) == 0) return SC_TRUNG_MA;
        if (strcmp(p->maPhim, mau->maPhim) == 0 && strcmp(p->tenPhim, mau->tenPhim) != 0)
            return SC_PHIM_KHAC_TEN;
        if (trung_lich(p, mau)) {
            if (trung) *trung = p;
            return SC_TRUNG_LICH;
        }
    }

    moi = (SuatChieu *)malloc(sizeof *moi);
    if (!moi) return SC_HET_BO_NHO;
    *moi = *mau;
    moi->dsGhe = NULL;

    tg = nl_phut(moi->ngay, moi->gio);
    while (*vi_tri && nl_phut((*vi_tri)->ngay, (*vi_tri)->gio) <= tg) vi_tri = &(*vi_tri)->next;
    moi->next = *vi_tri;
    *vi_tri = moi;
    return SC_OK;
}

SuatChieu *sc_tim(SuatChieu *dau, const char *maSuat) {
    for (; dau; dau = dau->next)
        if (strcmp(dau->maSuat, maSuat) == 0) return dau;
    return NULL;
}

const char *sc_ten_phim(const SuatChieu *dau, const char *maPhim) {
    for (; dau; dau = dau->next)
        if (strcmp(dau->maPhim, maPhim) == 0) return dau->tenPhim;
    return NULL;
}

int sc_dem(const SuatChieu *dau) {
    int n = 0;
    for (; dau; dau = dau->next) n++;
    return n;
}

void sc_giai_phong(SuatChieu **dau) {
    while (*dau) {
        SuatChieu *p = *dau;
        *dau = p->next;
        ve_giai_phong(p);
        free(p);
    }
}
