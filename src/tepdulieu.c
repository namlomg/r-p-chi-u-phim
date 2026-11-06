/* tepdulieu.c - Lưu và đọc suất chiếu cùng vé đã đặt.
 *
 * Định dạng (UTF-8), số cuối dòng suất chiếu là số ghế đã đặt ở các dòng ngay sau đó:
 *   maSuat|maPhim|tenPhim|phong|ngay|gio|thoiLuong|soHang|soCot|giaVe|soGhe
 *   hang|cot|tenKhach|sdt
 */
#define WIN32_LEAN_AND_MEAN
#include "tepdulieu.h"

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nhaplieu.h"
#include "suatchieu.h"
#include "ve.h"

#define DONG_TOI_DA 1024

KetQuaTep tep_luu(const SuatChieu *dau, const char *duong_dan, char *loi, size_t co_loi) {
    char tam[MAX_PATH];
    FILE *f;
    int ghi_loi;

    snprintf(tam, sizeof tam, "%s.tmp", duong_dan);
    f = fopen(tam, "wb");
    if (!f) {
        snprintf(loi, co_loi, "Không tạo được tệp tạm %s.", tam);
        return TEP_LOI;
    }
    for (; dau; dau = dau->next) {
        const GheDat *g;
        fprintf(f, "%s|%s|%s|%s|%s|%s|%d|%d|%d|%ld|%d\n", dau->maSuat, dau->maPhim, dau->tenPhim,
                dau->phong, dau->ngay, dau->gio, dau->thoiLuong, dau->soHang, dau->soCot,
                dau->giaVe, ve_dem(dau));
        for (g = dau->dsGhe; g; g = g->next)
            fprintf(f, "%d|%d|%s|%s\n", g->hang, g->cot, g->tenKhach, g->sdt);
    }
    ghi_loi = ferror(f);
    if (fclose(f) != 0) ghi_loi = 1;
    if (ghi_loi || !MoveFileExA(tam, duong_dan, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        remove(tam);
        snprintf(loi, co_loi, "Không ghi được tệp %s.", duong_dan);
        return TEP_LOI;
    }
    return TEP_OK;
}

/* Đọc một dòng, bỏ ký tự xuống dòng. Trả về 1 nếu có dòng, 0 nếu hết tệp, -1 nếu dòng quá dài. */
static int doc_dong(FILE *f, char *dong, size_t co) {
    size_t n;
    if (!fgets(dong, (int)co, f)) return 0;
    n = strlen(dong);
    if (n == co - 1 && dong[n - 1] != '\n' && !feof(f)) return -1;
    while (n > 0 && (dong[n - 1] == '\n' || dong[n - 1] == '\r')) dong[--n] = '\0';
    return 1;
}

/* Tách theo dấu |, giữ cả trường rỗng. Trả về số trường. */
static int tach(char *dong, char **truong, int toi_da) {
    int n = 0;
    char *p = dong;
    for (;;) {
        char *gach = strchr(p, '|');
        if (n == toi_da) return toi_da + 1;
        truong[n++] = p;
        if (!gach) return n;
        *gach = '\0';
        p = gach + 1;
    }
}

static int sao_chep(char *dich, size_t co, const char *nguon) {
    if (strlen(nguon) >= co) return 0;
    memcpy(dich, nguon, strlen(nguon) + 1);
    return 1;
}

static int doc_suat(char **t, SuatChieu *s, long *so_ghe) {
    long v;
    memset(s, 0, sizeof *s);
    if (!sao_chep(s->maSuat, sizeof s->maSuat, t[0]) ||
        !sao_chep(s->maPhim, sizeof s->maPhim, t[1]) ||
        !sao_chep(s->tenPhim, sizeof s->tenPhim, t[2]) ||
        !sao_chep(s->phong, sizeof s->phong, t[3]) ||
        !sao_chep(s->ngay, sizeof s->ngay, t[4]) ||
        !sao_chep(s->gio, sizeof s->gio, t[5]))
        return 0;
    if (!nl_so_nguyen(t[6], &v, 1, 600)) return 0;
    s->thoiLuong = (int)v;
    if (!nl_so_nguyen(t[7], &v, 1, SC_HANG_TOI_DA)) return 0;
    s->soHang = (int)v;
    if (!nl_so_nguyen(t[8], &v, 1, SC_COT_TOI_DA)) return 0;
    s->soCot = (int)v;
    if (!nl_so_nguyen(t[9], &s->giaVe, 1000, 10000000)) return 0;
    return nl_so_nguyen(t[10], so_ghe, 0, (long)s->soHang * s->soCot);
}

KetQuaTep tep_doc(SuatChieu **ra, const char *duong_dan, char *loi, size_t co_loi) {
    FILE *f = fopen(duong_dan, "rb");
    SuatChieu *ds = NULL;
    char dong[DONG_TOI_DA];
    int so_dong = 0, kq;

    if (!f) {
        snprintf(loi, co_loi, "Chưa có tệp %s.", duong_dan);
        return TEP_KHONG_CO;
    }

    while ((kq = doc_dong(f, dong, sizeof dong)) != 0) {
        char *t[12], *p = dong;
        SuatChieu mau, *s;
        long so_ghe = 0, i;

        so_dong++;
        if (kq < 0) goto loi_dong;
        if (so_dong == 1 && (unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB &&
            (unsigned char)p[2] == 0xBF)
            p += 3; /* bỏ BOM nếu tệp được sửa bằng Notepad */
        if (!*p) continue;
        if (tach(p, t, 11) != 11 || !doc_suat(t, &mau, &so_ghe)) goto loi_dong;
        if (sc_them(&ds, &mau, NULL) != SC_OK) goto loi_dong;
        s = sc_tim(ds, mau.maSuat);

        for (i = 0; i < so_ghe; i++) {
            long hang, cot;
            kq = doc_dong(f, dong, sizeof dong);
            so_dong++;
            if (kq <= 0 || tach(dong, t, 4) != 4) goto loi_dong;
            if (!nl_so_nguyen(t[0], &hang, 0, SC_HANG_TOI_DA - 1) ||
                !nl_so_nguyen(t[1], &cot, 1, SC_COT_TOI_DA))
                goto loi_dong;
            if (strlen(t[2]) >= sizeof(((GheDat *)0)->tenKhach) ||
                strlen(t[3]) >= sizeof(((GheDat *)0)->sdt))
                goto loi_dong;
            if (ve_them_ghe(s, (int)hang, (int)cot, t[2], t[3]) != VE_OK) goto loi_dong;
        }
    }
    if (ferror(f)) {
        snprintf(loi, co_loi, "Không đọc được tệp %s.", duong_dan);
        goto that_bai;
    }
    fclose(f);
    *ra = ds;
    return TEP_OK;

loi_dong:
    snprintf(loi, co_loi, "Tệp %s sai định dạng ở dòng %d.", duong_dan, so_dong);
that_bai:
    fclose(f);
    sc_giai_phong(&ds);
    return TEP_LOI;
}
