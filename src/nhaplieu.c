/* nhaplieu.c - Kiểm tra dữ liệu nhập và các hàm xử lý chuỗi UTF-8. */
#include "nhaplieu.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int la_so(char c) { return c >= '0' && c <= '9'; }

static int so_ngay_trong_thang(int nam, int thang) {
    static const int ngay[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int nhuan = (nam % 4 == 0 && nam % 100 != 0) || nam % 400 == 0;
    return thang == 2 && nhuan ? 29 : ngay[thang - 1];
}

static int ngay_ton_tai(int nam, int thang, int ngay) {
    if (nam < 2000 || nam > 2100 || thang < 1 || thang > 12) return 0;
    return ngay >= 1 && ngay <= so_ngay_trong_thang(nam, thang);
}

int nl_ngay_hop_le(const char *s) {
    int i;
    if (strlen(s) != 10 || s[4] != '-' || s[7] != '-') return 0;
    for (i = 0; i < 10; i++)
        if (i != 4 && i != 7 && !la_so(s[i])) return 0;
    return ngay_ton_tai(atoi(s), atoi(s + 5), atoi(s + 8));
}

int nl_gio_hop_le(const char *s) {
    if (strlen(s) != 5 || s[2] != ':') return 0;
    if (!la_so(s[0]) || !la_so(s[1]) || !la_so(s[3]) || !la_so(s[4])) return 0;
    return atoi(s) <= 23 && atoi(s + 3) <= 59;
}

int nl_ngay_vn_sang_iso(const char *vn, char *iso, size_t co) {
    int d, m, y;
    char du;
    if (sscanf(vn, "%d/%d/%d%c", &d, &m, &y, &du) != 3) return 0;
    if (!ngay_ton_tai(y, m, d)) return 0;
    snprintf(iso, co, "%04d-%02d-%02d", y, m, d);
    return 1;
}

void nl_iso_sang_vn(const char *iso, char *vn, size_t co) {
    snprintf(vn, co, "%.2s/%.2s/%.4s", iso + 8, iso + 5, iso);
}

long long nl_phut(const char *ngay, const char *gio) {
    /* Đổi ngày sang số ngày liên tục, để so sánh được cả suất chiếu qua đêm. */
    long long y = atoi(ngay), m = atoi(ngay + 5), d = atoi(ngay + 8);
    long long era, yoe, doy, doe, so_ngay;
    y -= m <= 2;
    era = y / 400;
    yoe = y - era * 400;
    doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    so_ngay = era * 146097 + doe;
    return so_ngay * 1440 + atoi(gio) * 60 + atoi(gio + 3);
}

int nl_so_nguyen(const char *s, long *ra, long nho_nhat, long lon_nhat) {
    char *het;
    long v;
    if (!*s) return 0;
    for (const char *p = s; *p; p++)
        if (!la_so(*p)) return 0;
    errno = 0;
    v = strtol(s, &het, 10);
    if (errno || *het || v < nho_nhat || v > lon_nhat) return 0;
    *ra = v;
    return 1;
}

int nl_ma_hop_le(const char *ma, size_t dai_toi_da) {
    size_t n = strlen(ma);
    if (n == 0 || n > dai_toi_da) return 0;
    for (; *ma; ma++)
        if (!la_so(*ma) && !(*ma >= 'A' && *ma <= 'Z')) return 0;
    return 1;
}

int nl_sdt_hop_le(const char *s) {
    size_t n = strlen(s);
    if (n < 9 || n > 11) return 0;
    for (; *s; s++)
        if (!la_so(*s)) return 0;
    return 1;
}

int nl_ten_hop_le(const char *s) {
    int co_chu = 0;
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c < 32 || c == '|') return 0;
        if (c != ' ') co_chu = 1;
    }
    return co_chu;
}

void nl_cat_khoang_trang(char *s) {
    size_t dau = 0, n = strlen(s);
    while (n > 0 && s[n - 1] == ' ') s[--n] = '\0';
    while (s[dau] == ' ') dau++;
    if (dau) memmove(s, s + dau, n - dau + 1);
}

/* Dấu tổ hợp U+0300..U+036F không chiếm thêm ô trên màn hình. */
static int do_rong_ky_tu(const unsigned char *p) {
    if ((p[0] & 0xC0) == 0x80) return 0;
    if (p[0] == 0xCC || (p[0] == 0xCD && p[1] < 0xB0)) return 0;
    return 1;
}

int nl_do_rong(const char *s) {
    int rong = 0;
    for (; *s; s++) rong += do_rong_ky_tu((const unsigned char *)s);
    return rong;
}

void nl_cat_hien_thi(const char *s, int rong, char *ra, size_t co) {
    static const char ba_cham[] = "\xE2\x80\xA6"; /* … */
    int da_dung = 0;
    size_t i = 0, j = 0;

    if (co == 0) return;
    if (rong <= 0) {
        ra[0] = '\0';
        return;
    }
    if (nl_do_rong(s) <= rong) {
        snprintf(ra, co, "%s", s);
        return;
    }
    while (s[i]) {
        int w = do_rong_ky_tu((const unsigned char *)s + i);
        if (da_dung + w > rong - 1) break;
        if (j + 1 + sizeof ba_cham > co) break;
        ra[j++] = s[i++];
        da_dung += w;
    }
    /* Không dừng giữa một ký tự nhiều byte. */
    while (s[i] && ((unsigned char)s[i] & 0xC0) == 0x80 && j + 1 + sizeof ba_cham <= co)
        ra[j++] = s[i++];
    while (j > 0 && ((unsigned char)s[i] & 0xC0) == 0x80) {
        j--;
        i--;
    }
    memcpy(ra + j, ba_cham, sizeof ba_cham);
}

void nl_tien(long long so, char *ra, size_t co) {
    char tam[32], kq[48];
    int n, i, j = 0;
    snprintf(tam, sizeof tam, "%lld", so);
    n = (int)strlen(tam);
    for (i = 0; i < n; i++) {
        kq[j++] = tam[i];
        if (tam[i] != '-' && (n - i - 1) % 3 == 0 && i != n - 1) kq[j++] = '.';
    }
    kq[j] = '\0';
    snprintf(ra, co, "%s", kq);
}
