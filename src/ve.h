/* ve.h - Đặt vé, hủy vé và kiểm tra ghế trống của một suất chiếu. */
#ifndef VE_H
#define VE_H

#include <stddef.h>

#include "mohinh.h"

typedef enum {
    VE_OK,
    VE_SAI_VI_TRI,
    VE_DA_DAT,
    VE_TRUNG_LUA_CHON,
    VE_THONG_TIN_SAI,
    VE_CHUA_DAT,
    VE_HET_BO_NHO
} KetQuaVe;

GheDat *ve_tim(const SuatChieu *s, int hang, int cot);
int ve_ghe_da_dat(const SuatChieu *s, int hang, int cot);
int ve_dem(const SuatChieu *s);
int ve_so_trong(const SuatChieu *s);

/* Thêm một ghế, giữ danh sách theo thứ tự hàng rồi đến cột. */
KetQuaVe ve_them_ghe(SuatChieu *s, int hang, int cot, const char *ten, const char *sdt);

/* Đặt nhiều ghế: một ghế không hợp lệ thì không đặt ghế nào.
   Nếu lỗi và vi_tri_loi khác NULL, *vi_tri_loi là chỉ số ghế gây lỗi. */
KetQuaVe ve_dat_nhieu(SuatChieu *s, const ViTriGhe *ghe, int n, const char *ten, const char *sdt,
                      int *vi_tri_loi);
KetQuaVe ve_huy(SuatChieu *s, int hang, int cot);
void ve_giai_phong(SuatChieu *s);

/* Đệ quy: k ghế bắt đầu từ (hang, cot) có trống và liền nhau trên cùng một hàng không? */
int ve_day_trong(const SuatChieu *s, int hang, int cot, int k);

/* Đệ quy: tìm dãy k ghế trống đầu tiên, tính từ (hang, cot) trở đi. */
int ve_tim_day_ghe(const SuatChieu *s, int hang, int cot, int k, int *ra_hang, int *ra_cot);

void ve_ma_ghe(int hang, int cot, char *ra, size_t co); /* (0, 5) -> A5 */

#endif
