/* manhinh.c - Các màn hình: menu, lịch chiếu, đặt/hủy vé, thống kê, lưu/đọc tệp. */
#define WIN32_LEAN_AND_MEAN
#include "manhinh.h"

#include <windows.h>

#include <stdio.h>
#include <string.h>

#include "nhaplieu.h"
#include "suatchieu.h"
#include "tepdulieu.h"
#include "thongke.h"
#include "ui.h"
#include "ve.h"

typedef struct {
    SuatChieu *ds;
    int chua_luu;
    const char *tep;
} UngDung;

#define HANG_NOI_DUNG 3
#define HANG_CUOI 20

static void khung(const UngDung *a, const char *duong_dan, const char *phim_tat) {
    ui_khung(duong_dan, a->chua_luu ? "● Chưa lưu" : "● Đã lưu", a->chua_luu ? M_VANG : M_XANH,
             phim_tat);
}

/* ---------- Chọn suất chiếu ---------- */

static void hang_tieu_de_lich(int hang) {
    ui_o(hang, 2, 6, M_NHAN, M_NEN, 0, "Mã");
    ui_o(hang, 9, 24, M_NHAN, M_NEN, 0, "Phim");
    ui_o(hang, 34, 6, M_NHAN, M_NEN, 0, "Phòng");
    ui_o(hang, 41, 10, M_NHAN, M_NEN, 0, "Ngày");
    ui_o(hang, 52, 5, M_NHAN, M_NEN, 0, "Giờ");
    ui_o(hang, 58, 8, M_NHAN, M_NEN, 1, "Giá vé");
    ui_o(hang, 67, 8, M_NHAN, M_NEN, 1, "Trống");
}

static void hang_lich(int hang, const SuatChieu *s, int dang_chon) {
    int fg = dang_chon ? M_TOI : M_CHU, bg = dang_chon ? M_VANG : M_NEN;
    char ngay[16], gia[32], trong[16];

    nl_iso_sang_vn(s->ngay, ngay, sizeof ngay);
    nl_tien(s->giaVe, gia, sizeof gia);
    snprintf(trong, sizeof trong, "%d/%d", ve_so_trong(s), s->soHang * s->soCot);
    ui_o(hang, 1, 76, fg, bg, 0, "");
    ui_o(hang, 2, 6, fg, bg, 0, s->maSuat);
    ui_o(hang, 9, 24, fg, bg, 0, s->tenPhim);
    ui_o(hang, 34, 6, fg, bg, 0, s->phong);
    ui_o(hang, 41, 10, fg, bg, 0, ngay);
    ui_o(hang, 52, 5, fg, bg, 0, s->gio);
    ui_o(hang, 58, 8, fg, bg, 1, gia);
    ui_o(hang, 67, 8, dang_chon ? M_TOI : (ve_so_trong(s) ? M_XANH : M_DO), bg, 1, trong);
}

static SuatChieu *chon_suat(const UngDung *a, const char *duong_dan) {
    enum { SO_DONG = 15 };
    int tong = sc_dem(a->ds), chon = 0, dau = 0;

    if (tong == 0) {
        khung(a, duong_dan, NULL);
        ui_bao_loi("Chưa có suất chiếu nào.\nHãy thêm lịch chiếu trước.");
        return NULL;
    }
    for (;;) {
        SuatChieu *s = a->ds, *dang_chon = NULL;
        int i = 0, phim;
        char ds[32];

        if (chon < dau) dau = chon;
        if (chon >= dau + SO_DONG) dau = chon - SO_DONG + 1;

        khung(a, duong_dan, "↑↓ Di chuyển   Enter Chọn   Esc Quay lại");
        hang_tieu_de_lich(HANG_NOI_DUNG);
        for (; s; s = s->next, i++) {
            if (i == chon) dang_chon = s;
            if (i >= dau && i < dau + SO_DONG) hang_lich(HANG_NOI_DUNG + 1 + i - dau, s, i == chon);
        }
        snprintf(ds, sizeof ds, "Suất %d/%d", chon + 1, tong);
        ui_o(HANG_CUOI, 2, 74, M_MO, M_NEN, 1, ds);
        ui_xuat();

        phim = ui_doc_phim();
        if (phim == K_ESC) return NULL;
        if (phim == K_ENTER) return dang_chon;
        if (phim == K_LEN && chon > 0) chon--;
        if (phim == K_XUONG && chon < tong - 1) chon++;
        if (phim == K_PGUP) chon = chon > SO_DONG ? chon - SO_DONG : 0;
        if (phim == K_PGDN) chon = chon + SO_DONG < tong ? chon + SO_DONG : tong - 1;
        if (phim == K_HOME) chon = 0;
        if (phim == K_END) chon = tong - 1;
    }
}

/* ---------- Sơ đồ ghế ---------- */

static int co_trong_chon(const ViTriGhe *c, int n, int hang, int cot) {
    int i;
    for (i = 0; i < n; i++)
        if (c[i].hang == hang && c[i].cot == cot) return 1;
    return 0;
}

/* Trạng thái ghế luôn lấy từ danh sách liên kết, không lưu thêm ma trận. */
static void ve_so_do(const SuatChieu *s, int con_tro_hang, int con_tro_cot, const ViTriGhe *chon,
                     int so_chon, const char *nhan_chon) {
    int rong = 5 + s->soCot * 4 - 1, trai = (UI_RONG - rong) / 2, h, c;
    char ngay[16], gia[32], ten[180], thong_tin[320];

    nl_iso_sang_vn(s->ngay, ngay, sizeof ngay);
    nl_tien(s->giaVe, gia, sizeof gia);
    nl_cat_hien_thi(s->tenPhim, 24, ten, sizeof ten);
    snprintf(thong_tin, sizeof thong_tin, "%s • %s • Phòng %s • %s %s • %s đ/vé", ten, s->maSuat,
             s->phong, ngay, s->gio, gia);
    ui_o(3, 2, 74, M_CHU, M_NEN, 2, thong_tin);
    ui_o(4, 2, 74, M_NHAN, M_NEN, 2, "══════════ MÀN HÌNH ══════════");

    for (c = 1; c <= s->soCot; c++) ui_in(5, trai + 5 + (c - 1) * 4, M_MO, M_NEN, "%2d", c);

    for (h = 0; h < s->soHang; h++) {
        ui_in(6 + h, trai + 2, M_MO, M_NEN, "%c", 'A' + h);
        for (c = 1; c <= s->soCot; c++) {
            int x = trai + 5 + (c - 1) * 4, y = 6 + h;
            int da_dat = ve_ghe_da_dat(s, h, c), da_chon = co_trong_chon(chon, so_chon, h, c);
            int fg = da_dat ? M_DO : M_XANH, bg = M_NEN;
            const char *o = da_dat ? "[X]" : "[ ]";
            if (da_chon) {
                o = "[*]";
                fg = M_TOI;
                bg = M_VANG;
            }
            if (h == con_tro_hang && c == con_tro_cot) {
                fg = M_TOI;
                bg = da_dat ? M_DO : M_TRANG;
            }
            ui_in(y, x, fg, bg, "%s", o);
        }
    }

    ui_in(16, 4, M_XANH, M_NEN, "[ ]");
    ui_in(16, 8, M_CHU, M_NEN, "Trống");
    ui_in(16, 16, M_DO, M_NEN, "[X]");
    ui_in(16, 20, M_CHU, M_NEN, "Đã đặt");
    if (nhan_chon) {
        ui_in(16, 29, M_TOI, M_VANG, "[*]");
        ui_in(16, 33, M_CHU, M_NEN, "%s", nhan_chon);
    }
}

static void ten_cac_ghe(const ViTriGhe *c, int n, char *ra, size_t co) {
    size_t da = 0;
    int i;
    ra[0] = '\0';
    for (i = 0; i < n; i++) {
        char ma[8];
        int them;
        ve_ma_ghe(c[i].hang, c[i].cot, ma, sizeof ma);
        them = snprintf(ra + da, co - da, "%s%s", i ? ", " : "", ma);
        if (them < 0 || (size_t)them >= co - da) break;
        da += (size_t)them;
    }
}

static void di_chuyen(const SuatChieu *s, int phim, int *h, int *c) {
    if (phim == K_LEN && *h > 0) (*h)--;
    if (phim == K_XUONG && *h < s->soHang - 1) (*h)++;
    if (phim == K_TRAI && *c > 1) (*c)--;
    if (phim == K_PHAI && *c < s->soCot) (*c)++;
    if (phim == K_HOME) *c = 1;
    if (phim == K_END) *c = s->soCot;
}

/* Hỏi số ghế liền nhau tại dòng cuối. Trả về 0 nếu người dùng bỏ qua. */
static int hoi_so_ghe(const SuatChieu *s, int *k) {
    char bd[8] = "";
    long v;
    ui_o(HANG_CUOI, 2, 74, M_CHU, M_NEN, 0, "");
    ui_in(HANG_CUOI, 3, M_NHAN, M_NEN, "Số ghế liền nhau cần tìm (Enter để tìm, Esc để bỏ qua):");
    if (ui_nhap(HANG_CUOI, 60, 4, bd, sizeof bd, 3, UI_NHAP_SO) != K_ENTER) return 0;
    if (!nl_so_nguyen(bd, &v, 1, SC_GHE_TOI_DA)) {
        ui_bao_loi("Số ghế phải là số nguyên dương.");
        return 0;
    }
    if (v > s->soCot) {
        char tb[128];
        snprintf(tb, sizeof tb, "Mỗi hàng của phòng này chỉ có %d ghế.", s->soCot);
        ui_bao_loi(tb);
        return 0;
    }
    *k = (int)v;
    return 1;
}

/* ---------- Xem lịch chiếu và ghế trống ---------- */

static void man_xem_ghe(const UngDung *a) {
    const char *dd = "Lịch chiếu và ghế trống";
    SuatChieu *s;

    while ((s = chon_suat(a, dd)) != NULL) {
        ViTriGhe goi_y[SC_COT_TOI_DA];
        int so_goi_y = 0, h = 0, c = 1, phim = 0, k = 0;
        char tb[128] = "";
        int mau_tb = M_MO;

        while (phim != K_ESC) {
            char dong[160], ma[8];
            const GheDat *g = ve_tim(s, h, c);

            khung(a, dd, "←↑↓→ Xem ghế   T Tìm ghế liền nhau   N Dãy tiếp theo   Esc Quay lại");
            ve_so_do(s, h, c, goi_y, so_goi_y, "Dãy tìm được");
            snprintf(dong, sizeof dong, "Trống %d   •   Đã đặt %d   •   Tổng %d ghế", ve_so_trong(s),
                     ve_dem(s), s->soHang * s->soCot);
            ui_o(17, 3, 72, M_CHU, M_NEN, 0, dong);
            ve_ma_ghe(h, c, ma, sizeof ma);
            snprintf(dong, sizeof dong, "Ghế %s: %s", ma, g ? "đã có người đặt" : "còn trống");
            ui_o(18, 3, 72, g ? M_DO : M_XANH, M_NEN, 0, dong);
            ui_o(19, 3, 72, mau_tb, M_NEN, 0, tb);
            ui_xuat();

            phim = ui_doc_phim();
            di_chuyen(s, phim, &h, &c);
            if (phim == 't' || phim == 'T' || phim == 'n' || phim == 'N') {
                int tu_hang = 0, tu_cot = 1, rh, rc;
                if (phim == 't' || phim == 'T' || k == 0) {
                    so_goi_y = 0;
                    if (!hoi_so_ghe(s, &k)) {
                        k = 0;
                        tb[0] = '\0';
                        continue;
                    }
                } else if (so_goi_y > 0) {
                    tu_hang = goi_y[0].hang;
                    tu_cot = goi_y[0].cot + 1;
                }
                if (ve_tim_day_ghe(s, tu_hang, tu_cot, k, &rh, &rc)) {
                    int i;
                    char ten[64];
                    so_goi_y = k;
                    for (i = 0; i < k; i++) {
                        goi_y[i].hang = rh;
                        goi_y[i].cot = rc + i;
                    }
                    h = rh;
                    c = rc;
                    ten_cac_ghe(goi_y, so_goi_y, ten, sizeof ten);
                    snprintf(tb, sizeof tb, "Tìm thấy %d ghế liền nhau: %s", k, ten);
                    mau_tb = M_VANG;
                } else {
                    so_goi_y = 0;
                    snprintf(tb, sizeof tb, "%s %d ghế trống liền nhau.",
                             tu_cot > 1 || tu_hang > 0 ? "Không còn dãy khác có" : "Không có", k);
                    mau_tb = M_DO;
                }
            }
        }
    }
}

/* ---------- Thêm lịch chiếu ---------- */

enum { F_SUAT, F_PHIM, F_TEN, F_PHONG, F_NGAY, F_GIO, F_PHUT, F_HANG, F_COT, F_GIA, SO_TRUONG };

typedef struct {
    const char *nhan, *goi_y;
    int toi_da, co;
} TruongNhap;

static const TruongNhap TRUONG[SO_TRUONG] = {
    {"Mã suất chiếu", "Tối đa 9 ký tự, ví dụ S05", 9, UI_NHAP_HOA},
    {"Mã phim", "Tối đa 9 ký tự, ví dụ P04", 9, UI_NHAP_HOA},
    {"Tên phim", "Tối đa 40 ký tự", 40, 0},
    {"Phòng", "Tối đa 9 ký tự, ví dụ P1", 9, UI_NHAP_HOA},
    {"Ngày chiếu", "dd/mm/yyyy, ví dụ 15/11/2026", 10, 0},
    {"Giờ chiếu", "HH:MM, ví dụ 19:30", 5, 0},
    {"Thời lượng", "Số phút, từ 1 đến 600", 3, UI_NHAP_SO},
    {"Số hàng ghế", "Từ 1 đến 10", 2, UI_NHAP_SO},
    {"Số ghế mỗi hàng", "Từ 1 đến 15", 2, UI_NHAP_SO},
    {"Giá vé", "Đồng, từ 1.000 đến 10.000.000", 8, UI_NHAP_SO},
};

static const char *kiem_tra_truong(int i, const char *v) {
    long so;
    char tam[16];
    switch (i) {
    case F_SUAT: case F_PHIM: case F_PHONG:
        return *v ? NULL : "Mã không được để trống.";
    case F_TEN:
        return nl_ten_hop_le(v) ? NULL : "Tên phim không được để trống.";
    case F_NGAY:
        return nl_ngay_vn_sang_iso(v, tam, sizeof tam) ? NULL : "Ngày không hợp lệ. Hãy nhập dd/mm/yyyy.";
    case F_GIO:
        return nl_gio_hop_le(v) ? NULL : "Giờ không hợp lệ. Hãy nhập HH:MM, ví dụ 19:30.";
    case F_PHUT:
        return nl_so_nguyen(v, &so, 1, 600) ? NULL : "Thời lượng phải từ 1 đến 600 phút.";
    case F_HANG:
        return nl_so_nguyen(v, &so, 1, SC_HANG_TOI_DA) ? NULL : "Số hàng phải từ 1 đến 10.";
    case F_COT:
        return nl_so_nguyen(v, &so, 1, SC_COT_TOI_DA) ? NULL : "Số ghế mỗi hàng phải từ 1 đến 15.";
    case F_GIA:
        return nl_so_nguyen(v, &so, 1000, 10000000) ? NULL : "Giá vé phải từ 1.000 đến 10.000.000 đồng.";
    default:
        return NULL;
    }
}

static void man_them_suat(UngDung *a) {
    char gt[SO_TRUONG][180];
    int dang = 0, i;
    const char *loi = NULL;
    char loi_bd[200];

    memset(gt, 0, sizeof gt);
    for (;;) {
        int phim;

        khung(a, "Thêm lịch chiếu", "Enter Tiếp   ↑ Trường trước   Esc Hủy");
        ui_in(3, 3, M_NHAN, M_NEN, "Nhập thông tin suất chiếu");
        for (i = 0; i < SO_TRUONG; i++) {
            int y = 5 + i;
            ui_o(y, 3, 18, i == dang ? M_VANG : M_CHU, M_NEN, 0, TRUONG[i].nhan);
            ui_o(y, 22, 42, M_TRANG, M_O, 0, gt[i]);
            if (i == dang) ui_o(16, 3, 72, M_MO, M_NEN, 0, TRUONG[i].goi_y);
        }
        if (loi) ui_o(18, 3, 72, M_DO, M_NEN, 0, loi);

        phim = ui_nhap(5 + dang, 22, 42, gt[dang], sizeof gt[dang], TRUONG[dang].toi_da, TRUONG[dang].co);
        if (phim == K_ESC) return;
        if (phim == K_DOI_CO) continue;
        if (phim == K_LEN) {
            if (dang > 0) dang--;
            loi = NULL;
            continue;
        }

        nl_cat_khoang_trang(gt[dang]);
        loi = kiem_tra_truong(dang, gt[dang]);
        if (loi) continue;

        /* Phim đã có thì tự điền tên để thống kê không bị tách thành hai phim. */
        if (dang == F_PHIM && !gt[F_TEN][0]) {
            const char *ten = sc_ten_phim(a->ds, gt[F_PHIM]);
            if (ten) snprintf(gt[F_TEN], sizeof gt[F_TEN], "%s", ten);
        }
        if (dang < SO_TRUONG - 1) {
            dang++;
            continue;
        }

        /* Đã đủ trường: kiểm tra lại toàn bộ rồi thêm. */
        for (i = 0; i < SO_TRUONG; i++) {
            loi = kiem_tra_truong(i, gt[i]);
            if (loi) {
                dang = i;
                break;
            }
        }
        if (loi) continue;
        {
            SuatChieu m;
            const SuatChieu *trung = NULL;
            long so;
            KetQuaSuat kq;

            memset(&m, 0, sizeof m);
            snprintf(m.maSuat, sizeof m.maSuat, "%s", gt[F_SUAT]);
            snprintf(m.maPhim, sizeof m.maPhim, "%s", gt[F_PHIM]);
            snprintf(m.tenPhim, sizeof m.tenPhim, "%s", gt[F_TEN]);
            snprintf(m.phong, sizeof m.phong, "%s", gt[F_PHONG]);
            nl_ngay_vn_sang_iso(gt[F_NGAY], m.ngay, sizeof m.ngay);
            snprintf(m.gio, sizeof m.gio, "%s", gt[F_GIO]);
            nl_so_nguyen(gt[F_PHUT], &so, 1, 600);
            m.thoiLuong = (int)so;
            nl_so_nguyen(gt[F_HANG], &so, 1, SC_HANG_TOI_DA);
            m.soHang = (int)so;
            nl_so_nguyen(gt[F_COT], &so, 1, SC_COT_TOI_DA);
            m.soCot = (int)so;
            nl_so_nguyen(gt[F_GIA], &m.giaVe, 1000, 10000000);

            kq = sc_them(&a->ds, &m, &trung);
            switch (kq) {
            case SC_OK:
                a->chua_luu = 1;
                khung(a, "Thêm lịch chiếu", NULL);
                snprintf(loi_bd, sizeof loi_bd, "Đã thêm suất chiếu %s.", m.maSuat);
                ui_bao_xong(loi_bd);
                return;
            case SC_TRUNG_MA:
                loi = "Mã suất chiếu này đã tồn tại.";
                dang = F_SUAT;
                break;
            case SC_PHIM_KHAC_TEN:
                snprintf(loi_bd, sizeof loi_bd, "Mã phim này đang là \"%s\".",
                         sc_ten_phim(a->ds, m.maPhim));
                loi = loi_bd;
                dang = F_TEN;
                break;
            case SC_TRUNG_LICH:
                snprintf(loi_bd, sizeof loi_bd, "Trùng giờ với suất %s cùng phòng (%s, %d phút).",
                         trung->maSuat, trung->gio, trung->thoiLuong);
                loi = loi_bd;
                dang = F_GIO;
                break;
            default:
                loi = "Không thêm được suất chiếu.";
                break;
            }
        }
    }
}

/* ---------- Đặt vé ---------- */

/* Trả về 1 nếu đã đặt thành công. */
static int form_dat_ve(UngDung *a, SuatChieu *s, const ViTriGhe *chon, int so_chon) {
    char ten[100] = "", sdt[12] = "", ds_ghe[200], tien[32], tb[420];
    const char *loi = NULL;
    int dang = 0;

    ten_cac_ghe(chon, so_chon, ds_ghe, sizeof ds_ghe);
    nl_tien((long long)so_chon * s->giaVe, tien, sizeof tien);

    for (;;) {
        int phim;
        khung(a, "Đặt vé › Thông tin khách", "Enter Tiếp   ↑ Trường trước   Esc Quay lại sơ đồ");
        ui_in(4, 3, M_NHAN, M_NEN, "Thông tin vé");
        ui_o(5, 3, 72, M_CHU, M_NEN, 0, s->tenPhim);
        snprintf(tb, sizeof tb, "Suất %s  •  Phòng %s  •  %s", s->maSuat, s->phong, s->gio);
        ui_o(6, 3, 72, M_MO, M_NEN, 0, tb);
        snprintf(tb, sizeof tb, "Ghế: %s", ds_ghe);
        ui_o(7, 3, 72, M_VANG, M_NEN, 0, tb);
        snprintf(tb, sizeof tb, "Thành tiền: %s đ", tien);
        ui_o(8, 3, 72, M_VANG, M_NEN, 0, tb);

        ui_in(10, 3, M_NHAN, M_NEN, "Thông tin khách hàng");
        ui_o(12, 3, 16, dang == 0 ? M_VANG : M_CHU, M_NEN, 0, "Họ và tên");
        ui_o(12, 20, 34, M_TRANG, M_O, 0, ten);
        ui_o(14, 3, 16, dang == 1 ? M_VANG : M_CHU, M_NEN, 0, "Số điện thoại");
        ui_o(14, 20, 34, M_TRANG, M_O, 0, sdt);
        if (loi) ui_o(16, 3, 72, M_DO, M_NEN, 0, loi);

        phim = dang == 0 ? ui_nhap(12, 20, 34, ten, sizeof ten, 30, 0)
                         : ui_nhap(14, 20, 34, sdt, sizeof sdt, 11, UI_NHAP_SO);
        if (phim == K_ESC) return 0;
        if (phim == K_DOI_CO) continue;
        if (phim == K_LEN) {
            dang = 0;
            loi = NULL;
            continue;
        }
        nl_cat_khoang_trang(ten);
        if (dang == 0) {
            loi = nl_ten_hop_le(ten) ? NULL : "Họ và tên không được để trống.";
            if (!loi) dang = 1;
            continue;
        }
        if (!nl_sdt_hop_le(sdt)) {
            loi = "Số điện thoại phải có từ 9 đến 11 chữ số.";
            continue;
        }
        if (!nl_ten_hop_le(ten)) {
            loi = "Họ và tên không được để trống.";
            dang = 0;
            continue;
        }
        loi = NULL;
        ui_o(16, 3, 72, M_DO, M_NEN, 0, "");

        snprintf(tb, sizeof tb, "Đặt %d vé cho %s?\nGhế: %s\nThành tiền: %s đ", so_chon, ten, ds_ghe,
                 tien);
        if (!ui_xac_nhan(tb, "Đặt vé", "Quay lại")) continue;
        if (ve_dat_nhieu(s, chon, so_chon, ten, sdt, NULL) != VE_OK) {
            ui_bao_loi("Không đặt được vé. Hãy kiểm tra lại các ghế đã chọn.");
            return 0;
        }
        a->chua_luu = 1;
        khung(a, "Đặt vé", NULL);
        snprintf(tb, sizeof tb, "Đã đặt %d vé cho %s.\nThành tiền: %s đ", so_chon, ten, tien);
        ui_bao_xong(tb);
        return 1;
    }
}

static void man_dat_ve(UngDung *a) {
    const char *dd = "Đặt vé";
    SuatChieu *s = chon_suat(a, dd);
    ViTriGhe chon[SC_GHE_TOI_DA];
    int so_chon = 0, h = 0, c = 1, i;
    char tb[128] = "";

    if (!s) return;
    for (;;) {
        char ds_ghe[64 + SC_GHE_TOI_DA * 6], tien[32], dong[320];
        int phim;

        khung(a, dd, "←↑↓→ Di chuyển   Space Chọn   T Tìm ghế   Enter Đặt   Esc Thoát");
        ve_so_do(s, h, c, chon, so_chon, "Đang chọn");
        ten_cac_ghe(chon, so_chon, ds_ghe, sizeof ds_ghe);
        nl_tien((long long)so_chon * s->giaVe, tien, sizeof tien);
        snprintf(dong, sizeof dong, "Đã chọn: %s", so_chon ? ds_ghe : "(chưa có ghế)");
        ui_o(17, 3, 50, M_CHU, M_NEN, 0, dong);
        snprintf(dong, sizeof dong, "Tạm tính: %s đ", tien);
        ui_o(17, 54, 21, M_VANG, M_NEN, 1, dong);
        ui_o(18, 3, 72, M_DO, M_NEN, 0, tb);
        ui_xuat();

        phim = ui_doc_phim();
        tb[0] = '\0';
        di_chuyen(s, phim, &h, &c);
        if (phim == K_ESC) return;
        if (phim == ' ') {
            int vt = -1;
            for (i = 0; i < so_chon; i++)
                if (chon[i].hang == h && chon[i].cot == c) vt = i;
            if (ve_ghe_da_dat(s, h, c)) {
                char ma[8];
                ve_ma_ghe(h, c, ma, sizeof ma);
                snprintf(tb, sizeof tb, "Ghế %s đã có người đặt.", ma);
            } else if (vt >= 0) {
                memmove(chon + vt, chon + vt + 1, sizeof chon[0] * (size_t)(so_chon - vt - 1));
                so_chon--;
            } else {
                chon[so_chon].hang = h;
                chon[so_chon].cot = c;
                so_chon++;
            }
        }
        if (phim == 't' || phim == 'T') {
            int k, rh, rc;
            if (hoi_so_ghe(s, &k)) {
                if (ve_tim_day_ghe(s, 0, 1, k, &rh, &rc)) {
                    so_chon = k;
                    for (i = 0; i < k; i++) {
                        chon[i].hang = rh;
                        chon[i].cot = rc + i;
                    }
                    h = rh;
                    c = rc;
                } else {
                    snprintf(tb, sizeof tb, "Không có %d ghế trống liền nhau.", k);
                }
            }
        }
        if (phim == K_ENTER) {
            if (so_chon == 0) {
                snprintf(tb, sizeof tb, "Hãy nhấn Space để chọn ít nhất một ghế.");
            } else if (form_dat_ve(a, s, chon, so_chon)) {
                return;
            }
        }
    }
}

/* ---------- Hủy vé ---------- */

static void man_huy_ve(UngDung *a) {
    const char *dd = "Hủy vé";
    SuatChieu *s = chon_suat(a, dd);
    int h = 0, c = 1;
    char tb[128] = "";

    if (!s) return;
    if (s->dsGhe) {
        h = s->dsGhe->hang;
        c = s->dsGhe->cot;
    }
    for (;;) {
        const GheDat *g = ve_tim(s, h, c);
        char ma[8], dong[320];
        int phim;

        ve_ma_ghe(h, c, ma, sizeof ma);
        khung(a, dd, "←↑↓→ Di chuyển   Enter Hủy vé ghế này   Esc Thoát");
        ve_so_do(s, h, c, NULL, 0, NULL);
        if (g) {
            snprintf(dong, sizeof dong, "Ghế %s  •  %s  •  %s", ma, g->tenKhach, g->sdt);
            ui_o(17, 3, 72, M_CHU, M_NEN, 0, dong);
        } else {
            snprintf(dong, sizeof dong, "Ghế %s còn trống.", ma);
            ui_o(17, 3, 72, M_MO, M_NEN, 0, dong);
        }
        ui_o(18, 3, 72, M_DO, M_NEN, 0, tb);
        ui_xuat();

        phim = ui_doc_phim();
        tb[0] = '\0';
        di_chuyen(s, phim, &h, &c);
        if (phim == K_ESC) return;
        if (phim == K_ENTER || phim == ' ') {
            if (!g) {
                snprintf(tb, sizeof tb, "Ghế %s chưa có người đặt.", ma);
            } else {
                char hoi[320], tien[32];
                nl_tien(s->giaVe, tien, sizeof tien);
                snprintf(hoi, sizeof hoi, "Hủy vé ghế %s của %s?\nHoàn lại: %s đ", ma, g->tenKhach, tien);
                if (ui_xac_nhan(hoi, "Hủy vé", "Giữ vé") && ve_huy(s, h, c) == VE_OK) {
                    a->chua_luu = 1;
                    khung(a, dd, NULL);
                    ve_so_do(s, h, c, NULL, 0, NULL);
                    snprintf(hoi, sizeof hoi, "Đã hủy vé ghế %s.", ma);
                    ui_bao_xong(hoi);
                }
            }
        }
    }
}

/* ---------- Thống kê ---------- */

static void ve_thanh(int hang, int cot, int rong, long long gia_tri, long long toi_da) {
    char day[256] = "", rng[256] = "";
    int n = 0, i;
    if (toi_da > 0) n = (int)(gia_tri * rong / toi_da);
    if (gia_tri > 0 && n == 0) n = 1;
    for (i = 0; i < n; i++) strcat(day, "━");
    for (i = n; i < rong; i++) strcat(rng, "─");
    ui_in(hang, cot, M_NHAN, M_NEN, "%s", day);
    ui_in(hang, cot + n, M_MO, M_NEN, "%s", rng);
}

static void man_thong_ke(const UngDung *a) {
    enum { SO_DONG = 14, RONG_THANH = 19 };
    int tab = 0, cuon = 0;

    for (;;) {
        char tien[32], dong[64];
        int tong = 0, i = 0, phim, y = 6;
        long long lon_nhat = 0;

        khung(a, "Thống kê doanh thu", "←→ Đổi bảng   ↑↓ Cuộn   Esc Quay lại");
        ui_o(3, 3, 14, tab == 0 ? M_TOI : M_CHU, tab == 0 ? M_VANG : M_O, 2, "Theo suất");
        ui_o(3, 18, 14, tab == 1 ? M_TOI : M_CHU, tab == 1 ? M_VANG : M_O, 2, "Theo phim");

        if (tab == 0) {
            const SuatChieu *s;
            ui_o(5, 3, 6, M_NHAN, M_NEN, 0, "Mã");
            ui_o(5, 10, 22, M_NHAN, M_NEN, 0, "Phim");
            ui_o(5, 33, 5, M_NHAN, M_NEN, 1, "Vé");
            ui_o(5, 39, 13, M_NHAN, M_NEN, 1, "Doanh thu");
            for (s = a->ds; s; s = s->next) {
                long long dt = tk_doanh_thu_suat(s);
                if (dt > lon_nhat) lon_nhat = dt;
                tong++;
            }
            for (s = a->ds; s; s = s->next, i++) {
                char ve[16];
                if (i < cuon || i >= cuon + SO_DONG) continue;
                nl_tien(tk_doanh_thu_suat(s), tien, sizeof tien);
                snprintf(ve, sizeof ve, "%d", ve_dem(s));
                ui_o(y, 3, 6, M_CHU, M_NEN, 0, s->maSuat);
                ui_o(y, 10, 22, M_CHU, M_NEN, 0, s->tenPhim);
                ui_o(y, 33, 5, M_CHU, M_NEN, 1, ve);
                ui_o(y, 39, 13, M_VANG, M_NEN, 1, tien);
                ve_thanh(y, 55, RONG_THANH, tk_doanh_thu_suat(s), lon_nhat);
                y++;
            }
        } else {
            PhimDoanhThu *ds = NULL, *p;
            ui_o(5, 3, 6, M_NHAN, M_NEN, 0, "Mã");
            ui_o(5, 10, 18, M_NHAN, M_NEN, 0, "Phim");
            ui_o(5, 29, 5, M_NHAN, M_NEN, 1, "Suất");
            ui_o(5, 35, 4, M_NHAN, M_NEN, 1, "Vé");
            ui_o(5, 40, 13, M_NHAN, M_NEN, 1, "Doanh thu");
            if (tk_theo_phim(a->ds, &ds) != 0) {
                ui_bao_loi("Không đủ bộ nhớ để lập thống kê.");
                return;
            }
            for (p = ds; p; p = p->next) {
                if (p->doanhThu > lon_nhat) lon_nhat = p->doanhThu;
                tong++;
            }
            for (p = ds; p; p = p->next, i++) {
                char so[16];
                if (i < cuon || i >= cuon + SO_DONG) continue;
                nl_tien(p->doanhThu, tien, sizeof tien);
                ui_o(y, 3, 6, M_CHU, M_NEN, 0, p->maPhim);
                ui_o(y, 10, 18, M_CHU, M_NEN, 0, p->tenPhim);
                snprintf(so, sizeof so, "%d", p->soSuat);
                ui_o(y, 29, 5, M_CHU, M_NEN, 1, so);
                snprintf(so, sizeof so, "%d", p->soVe);
                ui_o(y, 35, 4, M_CHU, M_NEN, 1, so);
                ui_o(y, 40, 13, M_VANG, M_NEN, 1, tien);
                ve_thanh(y, 55, RONG_THANH, p->doanhThu, lon_nhat);
                y++;
            }
            tk_giai_phong(&ds);
        }
        if (tong == 0) ui_o(7, 3, 72, M_MO, M_NEN, 2, "Chưa có dữ liệu.");

        nl_tien(tk_tong_doanh_thu(a->ds), tien, sizeof tien);
        snprintf(dong, sizeof dong, "%d vé đã bán", tk_tong_ve(a->ds));
        ui_o(HANG_CUOI, 3, 30, M_CHU, M_NEN, 0, dong);
        snprintf(dong, sizeof dong, "Tổng doanh thu: %s đ", tien);
        ui_o(HANG_CUOI, 34, 41, M_VANG, M_NEN, 1, dong);
        ui_xuat();

        phim = ui_doc_phim();
        if (phim == K_ESC) return;
        if (phim == K_TRAI || phim == K_PHAI || phim == K_TAB) {
            tab = !tab;
            cuon = 0;
        }
        if (phim == K_LEN && cuon > 0) cuon--;
        if (phim == K_XUONG && cuon + SO_DONG < tong) cuon++;
    }
}

/* ---------- Lưu và đọc tệp ---------- */

static int luu_tep(UngDung *a) {
    char loi[256];
    if (tep_luu(a->ds, a->tep, loi, sizeof loi) != TEP_OK) {
        ui_bao_loi(loi);
        return 0;
    }
    a->chua_luu = 0;
    return 1;
}

static void man_luu(UngDung *a) {
    char tb[160];
    if (!luu_tep(a)) return;
    khung(a, "Lưu dữ liệu", NULL);
    snprintf(tb, sizeof tb, "Đã lưu %d suất chiếu và %d vé vào %s.", sc_dem(a->ds), tk_tong_ve(a->ds), a->tep);
    ui_bao_xong(tb);
}

static void man_doc(UngDung *a) {
    SuatChieu *moi = NULL;
    char loi[256];
    KetQuaTep kq;

    if (a->chua_luu &&
        !ui_xac_nhan("Thay đổi chưa lưu sẽ bị mất.\nVẫn đọc lại dữ liệu từ tệp?", "Đọc lại", "Quay lại"))
        return;
    kq = tep_doc(&moi, a->tep, loi, sizeof loi);
    if (kq != TEP_OK) {
        char tb[320];
        snprintf(tb, sizeof tb, "%s\nDữ liệu đang mở được giữ nguyên.", loi);
        ui_bao_loi(tb);
        return;
    }
    sc_giai_phong(&a->ds);
    a->ds = moi;
    a->chua_luu = 0;
    khung(a, "Đọc lại dữ liệu", NULL);
    snprintf(loi, sizeof loi, "Đã đọc %d suất chiếu và %d vé.", sc_dem(a->ds), tk_tong_ve(a->ds));
    ui_bao_xong(loi);
}

/* Đọc khi mở chương trình. Tệp hỏng được sao lưu để lần lưu sau không ghi đè mất. */
static void doc_luc_mo(UngDung *a) {
    char loi[256];
    KetQuaTep kq = tep_doc(&a->ds, a->tep, loi, sizeof loi);
    if (kq == TEP_LOI) {
        char sao_luu[280], tb[640];
        snprintf(sao_luu, sizeof sao_luu, "%s.loi", a->tep);
        CopyFileA(a->tep, sao_luu, FALSE);
        khung(a, "Khởi động", NULL);
        snprintf(tb, sizeof tb, "%s\nĐã sao lưu tệp thành %s.\nChương trình bắt đầu với dữ liệu trống.", loi, sao_luu);
        ui_bao_loi(tb);
    }
}

/* ---------- Menu chính ---------- */

static void ve_hop_vien(int hang, int cot, int rong, int cao, const char *tieu_de, int mau) {
    char ngang[256] = "";
    int i, con = rong - 5 - nl_do_rong(tieu_de);

    for (i = 0; i < con; i++) strcat(ngang, "─");
    ui_in(hang, cot, mau, M_NEN, "┌─ ");
    ui_in(hang, cot + 3, M_NHAN, M_NEN, "%s", tieu_de);
    ui_in(hang, cot + 3 + nl_do_rong(tieu_de), mau, M_NEN, " %s┐", ngang);
    for (i = 1; i < cao - 1; i++) {
        ui_in(hang + i, cot, mau, M_NEN, "│");
        ui_in(hang + i, cot + rong - 1, mau, M_NEN, "│");
    }
    ngang[0] = '\0';
    for (i = 0; i < rong - 2; i++) strcat(ngang, "─");
    ui_in(hang + cao - 1, cot, mau, M_NEN, "└%s┘", ngang);
}

static void ve_dai_phim(int hang) {
    char dai[128] = "";
    int i;
    for (i = 0; i < 19; i++) strcat(dai, " ■  ");
    ui_in(hang, 1, 243, M_TOI, "%s", dai);
}

static void ve_tong_quan(const UngDung *a) {
    char tien[32], so[32];
    int tong_ghe = tk_tong_ghe(a->ds), ve = tk_tong_ve(a->ds);
    int ti_le = tong_ghe ? ve * 100 / tong_ghe : 0;

    ve_hop_vien(9, 41, 34, 11, "TỔNG QUAN", M_VIEN);
    snprintf(so, sizeof so, "%d", sc_dem(a->ds));
    ui_o(10, 43, 15, M_MO, M_NEN, 0, "Suất chiếu");
    ui_o(10, 58, 15, M_CHU, M_NEN, 1, so);
    snprintf(so, sizeof so, "%d", ve);
    ui_o(11, 43, 15, M_MO, M_NEN, 0, "Vé đã bán");
    ui_o(11, 58, 15, M_CHU, M_NEN, 1, so);
    snprintf(so, sizeof so, "%d", tong_ghe - ve);
    ui_o(12, 43, 15, M_MO, M_NEN, 0, "Ghế còn trống");
    ui_o(12, 58, 15, M_CHU, M_NEN, 1, so);
    nl_tien(tk_tong_doanh_thu(a->ds), tien, sizeof tien);
    snprintf(so, sizeof so, "%s đ", tien);
    ui_o(14, 43, 15, M_MO, M_NEN, 0, "Doanh thu");
    ui_o(14, 58, 15, M_VANG, M_NEN, 1, so);
    snprintf(so, sizeof so, "%d%%", ti_le);
    ui_o(16, 43, 15, M_MO, M_NEN, 0, "Lấp đầy");
    ui_o(16, 58, 15, M_XANH, M_NEN, 1, so);
    ve_thanh(17, 43, 30, ve, tong_ghe);
}

/* Bảng hiệu rạp: rèm sân khấu hai bên, chữ lớn ở giữa và dải phim bên dưới. */
static void ve_bang_hieu(void) {
    static const char *const CHU[] = {
        "█▀▀ █ █▄ █ █▀▀ █▀▄▀█ ▄▀█",
        "█   █ █ ▀█ █▀▀ █ ▀ █ █▀█",
        "▀▀▀ ▀ ▀  ▀ ▀▀▀ ▀   ▀ ▀ ▀",
    };
    static const int MAU_CHU[] = {229, 221, 209};
    int i, j;

    for (i = 0; i < 5; i++) {
        char day[64] = "";
        int dac = 8 - i * 2 > 1 ? 8 - i * 2 : 1, rong = dac + 3;
        for (j = 0; j < dac; j++) strcat(day, "█");
        ui_in(3 + i, 1, 124, M_NEN, "%s", day);
        ui_in(3 + i, 1 + dac, 124, M_NEN, "▓▒░");
        ui_in(3 + i, 77 - rong, 124, M_NEN, "░▒▓");
        ui_in(3 + i, 80 - rong, 124, M_NEN, "%s", day);
    }
    for (i = 0; i < 3; i++) ui_o(4 + i, 16, 46, MAU_CHU[i], M_NEN, 2, CHU[i]);
    ui_o(7, 16, 46, M_MO, M_NEN, 2, "Q U Ả N   L Ý   Đ Ặ T   V É");
    ve_dai_phim(8);
    ve_dai_phim(20);
}

void man_chay(const char *tep_du_lieu) {
    static const char *const MUC[] = {
        "Lịch chiếu và ghế trống", "Thêm lịch chiếu", "Đặt vé", "Hủy vé",
        "Thống kê doanh thu", "Lưu dữ liệu", "Đọc lại dữ liệu", "Thoát",
    };
    enum { SO_MUC = 8 };
    UngDung a;
    int chon = 0;

    a.ds = NULL;
    a.chua_luu = 0;
    a.tep = tep_du_lieu;
    doc_luc_mo(&a);

    for (;;) {
        int i, phim, thuc_hien = 0;

        khung(&a, "Menu chính", "↑↓ Di chuyển   Enter Chọn   1-8 Chọn nhanh   Esc Thoát");
        ve_bang_hieu();
        ve_hop_vien(9, 3, 36, 11, "CHỨC NĂNG", M_VIEN);
        for (i = 0; i < SO_MUC; i++) {
            int y = 10 + i + (i == SO_MUC - 1), dang = i == chon, thoat = i == SO_MUC - 1;
            int bg = dang ? M_VANG : M_NEN, fg = dang ? M_TOI : (thoat ? M_DO : M_CHU);
            ui_o(y, 4, 34, fg, bg, 0, "");
            ui_in(y, 5, fg, bg, "%s", dang ? "►" : " ");
            ui_in(y, 7, dang ? M_TOI : (thoat ? M_DO : M_VANG), bg, "%d", i + 1);
            ui_in(y, 10, fg, bg, "%s", MUC[i]);
        }
        ve_tong_quan(&a);
        ui_xuat();

        phim = ui_doc_phim();
        if (phim == K_LEN) chon = (chon + SO_MUC - 1) % SO_MUC;
        if (phim == K_XUONG) chon = (chon + 1) % SO_MUC;
        if (phim == K_ENTER) thuc_hien = 1;
        if (phim >= '1' && phim <= '0' + SO_MUC) {
            chon = phim - '1';
            thuc_hien = 1;
        }
        if (phim == K_ESC) {
            chon = SO_MUC - 1;
            thuc_hien = 1;
        }
        if (!thuc_hien) continue;

        switch (chon) {
        case 0: man_xem_ghe(&a); break;
        case 1: man_them_suat(&a); break;
        case 2: man_dat_ve(&a); break;
        case 3: man_huy_ve(&a); break;
        case 4: man_thong_ke(&a); break;
        case 5: man_luu(&a); break;
        case 6: man_doc(&a); break;
        default: {
            if (a.chua_luu) {
                static const char *const NUT[] = {"Lưu và thoát", "Không lưu", "Quay lại"};
                int kq = ui_chon("Thoát chương trình", M_VANG, "Bạn có thay đổi chưa lưu.", NUT, 3, 0);
                if (kq == 2 || kq < 0) break;
                if (kq == 0 && !luu_tep(&a)) break;
            }
            sc_giai_phong(&a.ds);
            return;
        }
        }
    }
}
