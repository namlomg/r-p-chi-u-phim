/* ui.h - Thư viện giao diện console: khung, màu, phím, ô nhập và hộp thoại.
 * Mọi tọa độ tính theo khung 78x24, khung được căn giữa cửa sổ. */
#ifndef UI_H
#define UI_H

#include <stddef.h>

#define UI_RONG 78
#define UI_CAO 24

/* Màu 256 của terminal */
enum {
    M_NEN = 234,
    M_O = 237,
    M_VIEN = 67,
    M_CHU = 253,
    M_MO = 245,
    M_NHAN = 81,
    M_VANG = 221,
    M_XANH = 78,
    M_DO = 203,
    M_TOI = 233,
    M_TRANG = 231
};

/* Phím đặc biệt, giá trị nằm ngoài vùng ký tự Unicode */
enum {
    K_LEN = 0x110000,
    K_XUONG,
    K_TRAI,
    K_PHAI,
    K_HOME,
    K_END,
    K_PGUP,
    K_PGDN,
    K_ENTER,
    K_ESC,
    K_BACK,
    K_TAB,
    K_DOI_CO /* cửa sổ vừa đổi kích thước, cần vẽ lại */
};

/* Trả về 0 nếu thành công, -1 nếu không chạy trong console hỗ trợ màu. */
int ui_khoi_dong(void);
void ui_ket_thuc(void);

void ui_xuat(void); /* đẩy toàn bộ khung hình vừa vẽ ra màn hình */
int ui_doc_phim(void);

void ui_in(int hang, int cot, int fg, int bg, const char *dinh_dang, ...);

/* In trong ô rộng cố định, tự cắt và đệm khoảng trắng. can: 0 trái, 1 phải, 2 giữa. */
void ui_o(int hang, int cot, int rong, int fg, int bg, int can, const char *chuoi);

void ui_khung(const char *duong_dan, const char *trang_thai, int mau_tt, const char *phim_tat);
void ui_ke_ngang(int hang);

#define UI_NHAP_HOA 1 /* tự viết hoa, chỉ nhận A-Z và 0-9 */
#define UI_NHAP_SO 2  /* chỉ nhận chữ số */

/* Trả về phím kết thúc: K_ENTER, K_ESC, K_LEN, K_XUONG, K_TAB hoặc K_DOI_CO. */
int ui_nhap(int hang, int cot, int rong, char *bo_dem, size_t co, int toi_da, int co_nhap);

/* Hộp thoại có nút. Trả về chỉ số nút được chọn, -1 nếu nhấn Esc. */
int ui_chon(const char *tieu_de, int mau, const char *noi_dung, const char *const *nut, int so_nut,
            int mac_dinh);
void ui_bao_loi(const char *noi_dung);
void ui_bao_xong(const char *noi_dung);
int ui_xac_nhan(const char *noi_dung, const char *co, const char *khong);

#endif
