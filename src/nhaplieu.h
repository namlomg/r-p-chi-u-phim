/* nhaplieu.h - Kiểm tra dữ liệu nhập và các hàm xử lý chuỗi UTF-8. */
#ifndef NHAPLIEU_H
#define NHAPLIEU_H

#include <stddef.h>

int nl_ngay_hop_le(const char *iso);               /* YYYY-MM-DD */
int nl_gio_hop_le(const char *gio);                /* HH:MM */
int nl_ngay_vn_sang_iso(const char *vn, char *iso, size_t co); /* DD/MM/YYYY -> ISO */
void nl_iso_sang_vn(const char *iso, char *vn, size_t co);
long long nl_phut(const char *ngay, const char *gio); /* số phút tuyệt đối */

int nl_so_nguyen(const char *s, long *ra, long nho_nhat, long lon_nhat);
int nl_ma_hop_le(const char *ma, size_t dai_toi_da); /* A-Z, 0-9 */
int nl_sdt_hop_le(const char *sdt);                  /* 9 đến 11 chữ số */
int nl_ten_hop_le(const char *ten);                  /* không rỗng, không có | */
void nl_cat_khoang_trang(char *s);

int nl_do_rong(const char *utf8); /* số ô hiển thị, không phải số byte */
void nl_cat_hien_thi(const char *utf8, int rong, char *ra, size_t co);
void nl_tien(long long so, char *ra, size_t co); /* 160000 -> 160.000 */

#endif
