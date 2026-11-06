/* tepdulieu.h - Lưu và đọc suất chiếu cùng vé đã đặt. */
#ifndef TEPDULIEU_H
#define TEPDULIEU_H

#include <stddef.h>

#include "mohinh.h"

typedef enum { TEP_OK, TEP_KHONG_CO, TEP_LOI } KetQuaTep;

/* Ghi ra tệp tạm rồi mới thay tệp chính, nên lỗi khi ghi không làm mất dữ liệu cũ. */
KetQuaTep tep_luu(const SuatChieu *dau, const char *duong_dan, char *loi, size_t co_loi);

/* Chỉ gán *ra khi đọc thành công toàn bộ tệp. */
KetQuaTep tep_doc(SuatChieu **ra, const char *duong_dan, char *loi, size_t co_loi);

#endif
