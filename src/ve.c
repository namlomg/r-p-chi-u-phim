/* ve.c - Đặt vé, hủy vé và kiểm tra ghế trống của một suất chiếu. */
#include "ve.h"

#include <stdio.h>
#include <stdlib.h>

#include "nhaplieu.h"

static int vi_tri_hop_le(const SuatChieu *s, int hang, int cot) {
    return hang >= 0 && hang < s->soHang && cot >= 1 && cot <= s->soCot;
}

GheDat *ve_tim(const SuatChieu *s, int hang, int cot) {
    GheDat *g;
    for (g = s->dsGhe; g; g = g->next)
        if (g->hang == hang && g->cot == cot) return g;
    return NULL;
}

int ve_ghe_da_dat(const SuatChieu *s, int hang, int cot) { return ve_tim(s, hang, cot) != NULL; }

int ve_dem(const SuatChieu *s) {
    int n = 0;
    const GheDat *g;
    for (g = s->dsGhe; g; g = g->next) n++;
    return n;
}

int ve_so_trong(const SuatChieu *s) { return s->soHang * s->soCot - ve_dem(s); }

KetQuaVe ve_them_ghe(SuatChieu *s, int hang, int cot, const char *ten, const char *sdt) {
    GheDat *moi, **p = &s->dsGhe;

    if (!vi_tri_hop_le(s, hang, cot)) return VE_SAI_VI_TRI;
    if (ve_ghe_da_dat(s, hang, cot)) return VE_DA_DAT;
    if (!nl_ten_hop_le(ten) || !nl_sdt_hop_le(sdt)) return VE_THONG_TIN_SAI;

    moi = (GheDat *)malloc(sizeof *moi);
    if (!moi) return VE_HET_BO_NHO;
    moi->hang = hang;
    moi->cot = cot;
    snprintf(moi->tenKhach, sizeof moi->tenKhach, "%s", ten);
    snprintf(moi->sdt, sizeof moi->sdt, "%s", sdt);

    while (*p && ((*p)->hang < hang || ((*p)->hang == hang && (*p)->cot < cot))) p = &(*p)->next;
    moi->next = *p;
    *p = moi;
    return VE_OK;
}

KetQuaVe ve_dat_nhieu(SuatChieu *s, const ViTriGhe *ghe, int n, const char *ten, const char *sdt,
                      int *vi_tri_loi) {
    int i, j;

    if (n <= 0 || !nl_ten_hop_le(ten) || !nl_sdt_hop_le(sdt)) return VE_THONG_TIN_SAI;

    /* Kiểm tra toàn bộ trước khi thêm ghế đầu tiên. */
    for (i = 0; i < n; i++) {
        KetQuaVe loi = VE_OK;
        if (!vi_tri_hop_le(s, ghe[i].hang, ghe[i].cot)) loi = VE_SAI_VI_TRI;
        else if (ve_ghe_da_dat(s, ghe[i].hang, ghe[i].cot)) loi = VE_DA_DAT;
        for (j = 0; j < i && loi == VE_OK; j++)
            if (ghe[j].hang == ghe[i].hang && ghe[j].cot == ghe[i].cot) loi = VE_TRUNG_LUA_CHON;
        if (loi != VE_OK) {
            if (vi_tri_loi) *vi_tri_loi = i;
            return loi;
        }
    }

    for (i = 0; i < n; i++) {
        KetQuaVe kq = ve_them_ghe(s, ghe[i].hang, ghe[i].cot, ten, sdt);
        if (kq != VE_OK) {
            /* Hết bộ nhớ giữa chừng: trả lại các ghế vừa thêm. */
            for (j = 0; j < i; j++) ve_huy(s, ghe[j].hang, ghe[j].cot);
            if (vi_tri_loi) *vi_tri_loi = i;
            return kq;
        }
    }
    return VE_OK;
}

KetQuaVe ve_huy(SuatChieu *s, int hang, int cot) {
    GheDat **p = &s->dsGhe;

    if (!vi_tri_hop_le(s, hang, cot)) return VE_SAI_VI_TRI;
    while (*p) {
        if ((*p)->hang == hang && (*p)->cot == cot) {
            GheDat *xoa = *p;
            *p = xoa->next;
            free(xoa);
            return VE_OK;
        }
        p = &(*p)->next;
    }
    return VE_CHUA_DAT;
}

void ve_giai_phong(SuatChieu *s) {
    while (s->dsGhe) {
        GheDat *g = s->dsGhe;
        s->dsGhe = g->next;
        free(g);
    }
}

int ve_day_trong(const SuatChieu *s, int hang, int cot, int k) {
    if (k == 0) return 1;
    if (!vi_tri_hop_le(s, hang, cot) || ve_ghe_da_dat(s, hang, cot)) return 0;
    return ve_day_trong(s, hang, cot + 1, k - 1);
}

int ve_tim_day_ghe(const SuatChieu *s, int hang, int cot, int k, int *ra_hang, int *ra_cot) {
    if (k <= 0 || k > s->soCot || hang >= s->soHang) return 0;
    if (cot + k - 1 > s->soCot) return ve_tim_day_ghe(s, hang + 1, 1, k, ra_hang, ra_cot);
    if (ve_day_trong(s, hang, cot, k)) {
        *ra_hang = hang;
        *ra_cot = cot;
        return 1;
    }
    return ve_tim_day_ghe(s, hang, cot + 1, k, ra_hang, ra_cot);
}

void ve_ma_ghe(int hang, int cot, char *ra, size_t co) {
    snprintf(ra, co, "%c%d", 'A' + hang, cot);
}
