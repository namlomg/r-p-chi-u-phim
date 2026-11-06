/* kiemthu.c - Kiểm thử tự động phần nghiệp vụ, không cần giao diện. Chạy: build.bat test */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/nhaplieu.h"
#include "../src/suatchieu.h"
#include "../src/tepdulieu.h"
#include "../src/thongke.h"
#include "../src/ve.h"

static int so_dat, so_loi;

#define KT(dk)                                                       \
    do {                                                             \
        if (dk) so_dat++;                                            \
        else {                                                       \
            so_loi++;                                                \
            printf("  LOI dong %d: %s\n", __LINE__, #dk);            \
        }                                                            \
    } while (0)

static SuatChieu mau(const char *ma, const char *phim, const char *ten, const char *phong,
                     const char *ngay, const char *gio, int phut, int hang, int cot, long gia) {
    SuatChieu s;
    memset(&s, 0, sizeof s);
    snprintf(s.maSuat, sizeof s.maSuat, "%s", ma);
    snprintf(s.maPhim, sizeof s.maPhim, "%s", phim);
    snprintf(s.tenPhim, sizeof s.tenPhim, "%s", ten);
    snprintf(s.phong, sizeof s.phong, "%s", phong);
    snprintf(s.ngay, sizeof s.ngay, "%s", ngay);
    snprintf(s.gio, sizeof s.gio, "%s", gio);
    s.thoiLuong = phut;
    s.soHang = hang;
    s.soCot = cot;
    s.giaVe = gia;
    return s;
}

static void kt_lich(void) {
    SuatChieu *ds = NULL, m;
    const SuatChieu *trung = NULL;

    puts("Lich chieu");
    m = mau("S01", "P01", "Phim A", "P1", "2026-11-10", "19:00", 120, 8, 10, 80000);
    KT(sc_them(&ds, &m, NULL) == SC_OK);
    KT(sc_them(&ds, &m, NULL) == SC_TRUNG_MA);

    m = mau("S02", "P02", "Phim B", "P1", "2026-11-10", "20:00", 90, 8, 10, 80000);
    KT(sc_them(&ds, &m, &trung) == SC_TRUNG_LICH);
    KT(trung && strcmp(trung->maSuat, "S01") == 0);

    /* Vừa hết suất trước thì không tính là trùng; phòng khác cũng không trùng. */
    m = mau("S03", "P02", "Phim B", "P1", "2026-11-10", "21:00", 90, 8, 10, 80000);
    KT(sc_them(&ds, &m, NULL) == SC_OK);
    m = mau("S04", "P02", "Phim B", "P2", "2026-11-10", "19:30", 90, 8, 10, 80000);
    KT(sc_them(&ds, &m, NULL) == SC_OK);

    /* Suất qua đêm vẫn phải bị phát hiện trùng với suất sáng sớm hôm sau. */
    m = mau("S05", "P03", "Phim C", "P3", "2026-12-31", "23:30", 120, 8, 10, 80000);
    KT(sc_them(&ds, &m, NULL) == SC_OK);
    m = mau("S06", "P03", "Phim C", "P3", "2027-01-01", "01:00", 90, 8, 10, 80000);
    KT(sc_them(&ds, &m, NULL) == SC_TRUNG_LICH);

    m = mau("S07", "P01", "Tên khác", "P4", "2026-11-11", "10:00", 90, 8, 10, 80000);
    KT(sc_them(&ds, &m, NULL) == SC_PHIM_KHAC_TEN);

    m = mau("S08", "P08", "Phim D", "P5", "2026-02-30", "10:00", 90, 8, 10, 80000);
    KT(sc_them(&ds, &m, NULL) == SC_KHONG_HOP_LE);
    m = mau("S08", "P08", "Phim D", "P5", "2026-02-20", "24:00", 90, 8, 10, 80000);
    KT(sc_them(&ds, &m, NULL) == SC_KHONG_HOP_LE);
    m = mau("S08", "P08", "Phim D", "P5", "2026-02-20", "10:00", 90, 11, 10, 80000);
    KT(sc_them(&ds, &m, NULL) == SC_KHONG_HOP_LE);
    m = mau("S08", "P08", "Phim D", "P5", "2026-02-20", "10:00", 90, 8, 16, 80000);
    KT(sc_them(&ds, &m, NULL) == SC_KHONG_HOP_LE);

    /* Danh sách luôn theo thứ tự thời gian. */
    KT(strcmp(ds->maSuat, "S01") == 0);
    KT(strcmp(ds->next->maSuat, "S04") == 0);
    KT(sc_dem(ds) == 4);
    sc_giai_phong(&ds);
    KT(ds == NULL);
}

static void kt_ve(void) {
    SuatChieu s = mau("S01", "P01", "Phim", "P1", "2026-11-10", "19:00", 120, 3, 10, 80000);
    ViTriGhe ba[3] = {{0, 5}, {0, 6}, {0, 5}};
    ViTriGhe hai[2] = {{0, 5}, {0, 6}};
    ViTriGhe mot[1] = {{0, 5}};
    int loi = -1;

    puts("Dat va huy ve");
    KT(ve_them_ghe(&s, 0, 5, "An", "0901234567") == VE_OK);
    KT(ve_them_ghe(&s, 0, 5, "Binh", "0901234567") == VE_DA_DAT);
    KT(ve_them_ghe(&s, 3, 1, "An", "0901234567") == VE_SAI_VI_TRI);
    KT(ve_them_ghe(&s, 0, 11, "An", "0901234567") == VE_SAI_VI_TRI);
    KT(ve_them_ghe(&s, 0, 0, "An", "0901234567") == VE_SAI_VI_TRI);
    KT(ve_them_ghe(&s, 1, 1, "  ", "0901234567") == VE_THONG_TIN_SAI);
    KT(ve_them_ghe(&s, 1, 1, "An", "12") == VE_THONG_TIN_SAI);
    KT(ve_huy(&s, 0, 5) == VE_OK);

    /* Một ghế lỗi trong lần đặt thì không ghế nào được đặt. */
    KT(ve_dat_nhieu(&s, ba, 3, "An", "0901234567", &loi) == VE_TRUNG_LUA_CHON);
    KT(loi == 2);
    KT(ve_dem(&s) == 0);
    KT(ve_them_ghe(&s, 0, 6, "Binh", "0901234567") == VE_OK);
    KT(ve_dat_nhieu(&s, hai, 2, "An", "0901234567", &loi) == VE_DA_DAT);
    KT(loi == 1);
    KT(ve_dem(&s) == 1);
    KT(ve_huy(&s, 0, 6) == VE_OK);
    KT(ve_dat_nhieu(&s, hai, 2, "An", "0901234567", NULL) == VE_OK);
    KT(ve_dem(&s) == 2);
    KT(ve_dat_nhieu(&s, mot, 1, "An", "0901234567", NULL) == VE_DA_DAT);

    /* Hủy nút đầu, giữa, cuối đều giữ đúng danh sách và doanh thu. */
    ve_giai_phong(&s);
    KT(ve_them_ghe(&s, 0, 3, "C", "0901234567") == VE_OK);
    KT(ve_them_ghe(&s, 0, 1, "A", "0901234567") == VE_OK);
    KT(ve_them_ghe(&s, 0, 2, "B", "0901234567") == VE_OK);
    KT(ve_them_ghe(&s, 1, 1, "D", "0901234567") == VE_OK);
    KT(s.dsGhe->cot == 1 && s.dsGhe->next->cot == 2 && s.dsGhe->next->next->cot == 3);
    KT(tk_doanh_thu_suat(&s) == 320000);
    KT(ve_huy(&s, 0, 1) == VE_OK); /* đầu */
    KT(s.dsGhe->cot == 2);
    KT(tk_doanh_thu_suat(&s) == 240000);
    KT(ve_huy(&s, 0, 3) == VE_OK); /* giữa */
    KT(s.dsGhe->next->hang == 1);
    KT(ve_huy(&s, 1, 1) == VE_OK); /* cuối */
    KT(s.dsGhe->next == NULL);
    KT(tk_doanh_thu_suat(&s) == 80000);
    KT(ve_huy(&s, 2, 2) == VE_CHUA_DAT);
    KT(ve_huy(&s, 9, 9) == VE_SAI_VI_TRI);
    KT(ve_so_trong(&s) == 29);
    ve_giai_phong(&s);
}

static void kt_de_quy(void) {
    SuatChieu s = mau("S01", "P01", "Phim", "P1", "2026-11-10", "19:00", 120, 2, 10, 80000);
    int h = -1, c = -1, i;

    puts("De quy day ghe lien ke");
    KT(ve_day_trong(&s, 0, 1, 10));
    KT(!ve_day_trong(&s, 0, 1, 11));
    KT(ve_day_trong(&s, 0, 8, 3));
    KT(!ve_day_trong(&s, 0, 8, 4)); /* không được vượt ra ngoài hàng */
    KT(ve_day_trong(&s, 0, 1, 0));

    /* Hàng A chỉ trống cột 8-10, nên 4 ghế liền nhau phải nằm ở hàng B. */
    for (i = 1; i <= 7; i++) ve_them_ghe(&s, 0, i, "X", "0901234567");
    KT(ve_tim_day_ghe(&s, 0, 1, 3, &h, &c) && h == 0 && c == 8);
    KT(ve_tim_day_ghe(&s, 0, 1, 4, &h, &c) && h == 1 && c == 1);

    /* Dãy bị ghế đã đặt chia cắt thì phải bỏ qua. */
    ve_them_ghe(&s, 1, 3, "X", "0901234567");
    KT(ve_tim_day_ghe(&s, 0, 1, 4, &h, &c) && h == 1 && c == 4);
    KT(ve_tim_day_ghe(&s, 0, 1, 7, &h, &c) && h == 1 && c == 4);
    KT(!ve_tim_day_ghe(&s, 0, 1, 8, &h, &c));
    KT(!ve_tim_day_ghe(&s, 0, 1, 11, &h, &c));
    KT(!ve_tim_day_ghe(&s, 0, 1, 0, &h, &c));
    ve_giai_phong(&s);
}

static void kt_thong_ke(void) {
    SuatChieu *ds = NULL, m, *s;
    PhimDoanhThu *pdt = NULL;

    puts("Thong ke");
    m = mau("S01", "P01", "Phim A", "P1", "2026-11-10", "09:00", 90, 5, 5, 50000);
    sc_them(&ds, &m, NULL);
    m = mau("S02", "P01", "Phim A", "P1", "2026-11-10", "12:00", 90, 5, 5, 70000);
    sc_them(&ds, &m, NULL);
    m = mau("S03", "P02", "Phim B", "P1", "2026-11-10", "15:00", 90, 5, 5, 100000);
    sc_them(&ds, &m, NULL);
    m = mau("S04", "P03", "Phim C", "P1", "2026-11-10", "18:00", 90, 5, 5, 100000);
    sc_them(&ds, &m, NULL);

    s = sc_tim(ds, "S01");
    ve_them_ghe(s, 0, 1, "A", "0901234567");
    ve_them_ghe(s, 0, 2, "A", "0901234567");
    s = sc_tim(ds, "S02");
    ve_them_ghe(s, 0, 1, "A", "0901234567");
    s = sc_tim(ds, "S03");
    ve_them_ghe(s, 0, 1, "A", "0901234567");
    ve_them_ghe(s, 0, 2, "A", "0901234567");
    ve_them_ghe(s, 0, 3, "A", "0901234567");

    KT(tk_tong_doanh_thu(ds) == 470000);
    KT(tk_tong_ve(ds) == 6);
    KT(tk_tong_ghe(ds) == 100);
    KT(tk_theo_phim(ds, &pdt) == 0);
    KT(pdt && strcmp(pdt->maPhim, "P02") == 0 && pdt->doanhThu == 300000);
    KT(pdt && pdt->next && strcmp(pdt->next->maPhim, "P01") == 0 &&
       pdt->next->doanhThu == 170000 && pdt->next->soSuat == 2 && pdt->next->soVe == 3);
    KT(pdt && pdt->next && pdt->next->next && pdt->next->next->doanhThu == 0);
    tk_giai_phong(&pdt);
    sc_giai_phong(&ds);
}

static void ghi_tep(const char *ten, const char *nd) {
    FILE *f = fopen(ten, "wb");
    if (!f) exit(2);
    fputs(nd, f);
    fclose(f);
}

static void kt_tep(void) {
    const char *tep = "kiemthu_tam.txt";
    SuatChieu *ds = NULL, *doc = NULL, m, *s;
    char loi[256] = "";

    puts("Luu va doc tep");
    remove(tep);
    KT(tep_doc(&doc, tep, loi, sizeof loi) == TEP_KHONG_CO);

    m = mau("S01", "P01", "Mùa Hoa Phượng", "P1", "2026-11-10", "19:30", 120, 8, 10, 80000);
    sc_them(&ds, &m, NULL);
    m = mau("S02", "P02", "Phim rỗng", "P2", "2026-11-11", "09:00", 90, 5, 6, 65000);
    sc_them(&ds, &m, NULL);
    s = sc_tim(ds, "S01");
    ve_them_ghe(s, 0, 5, "Nguyễn Văn An", "0901234567");
    ve_them_ghe(s, 7, 10, "Trần Thị Bình Minh", "0912345678");

    KT(tep_luu(ds, tep, loi, sizeof loi) == TEP_OK);
    KT(tep_doc(&doc, tep, loi, sizeof loi) == TEP_OK);
    KT(sc_dem(doc) == 2);
    s = sc_tim(doc, "S01");
    KT(s && strcmp(s->tenPhim, "Mùa Hoa Phượng") == 0);
    KT(s && ve_dem(s) == 2 && s->soHang == 8 && s->soCot == 10 && s->giaVe == 80000);
    KT(s && ve_tim(s, 0, 5) && strcmp(ve_tim(s, 0, 5)->tenKhach, "Nguyễn Văn An") == 0);
    KT(tk_tong_doanh_thu(doc) == tk_tong_doanh_thu(ds));
    KT(sc_tim(doc, "S02") && sc_tim(doc, "S02")->dsGhe == NULL);
    sc_giai_phong(&doc);

    /* Tệp hỏng phải báo lỗi và không làm thay đổi danh sách đang có. */
    {
        static const char *hong[] = {
            "day khong phai du lieu\n",
            "S01|P01|Phim|P1|2026-11-10|19:30|120|8|10|80000|2\n0|5|An|0901234567\n",
            "S01|P01|Phim|P1|2026-13-10|19:30|120|8|10|80000|0\n",
            "S01|P01|Phim|P1|2026-11-10|19:30|120|8|99|80000|0\n",
            "S01|P01|Phim|P1|2026-11-10|19:30|120|8|10|abc|0\n",
            "S01|P01|Phim|P1|2026-11-10|19:30|120|8|10|80000|1\n8|1|An|0901234567\n",
            "S01|P01|Phim|P1|2026-11-10|19:30|120|8|10|80000|2\n0|1|An|0901234567\n0|1|An|0901234567\n",
            "S01|P01|Phim|P1|2026-11-10|19:30|120|8|10|80000|0\nS01|P01|Phim|P2|2026-11-11|19:30|120|8|10|80000|0\n",
        };
        size_t i;
        SuatChieu *giu = ds;
        for (i = 0; i < sizeof hong / sizeof hong[0]; i++) {
            SuatChieu *ra = giu;
            ghi_tep(tep, hong[i]);
            KT(tep_doc(&ra, tep, loi, sizeof loi) == TEP_LOI);
            KT(ra == giu);
        }
    }

    /* Tệp sửa bằng Notepad có BOM, xuống dòng CRLF và dòng trống vẫn đọc được. */
    ghi_tep(tep, "\xEF\xBB\xBFS01|P01|Phim|P1|2026-11-10|19:30|120|8|10|80000|1\r\n0|1|An|0901234567\r\n\r\n");
    KT(tep_doc(&doc, tep, loi, sizeof loi) == TEP_OK);
    KT(doc && ve_dem(doc) == 1);
    sc_giai_phong(&doc);

    remove(tep);
    sc_giai_phong(&ds);
}

static void kt_nhap_lieu(void) {
    char ra[64];
    long v;

    puts("Nhap lieu va UTF-8");
    KT(nl_ngay_vn_sang_iso("29/02/2028", ra, sizeof ra) && strcmp(ra, "2028-02-29") == 0);
    KT(!nl_ngay_vn_sang_iso("29/02/2027", ra, sizeof ra));
    KT(!nl_ngay_vn_sang_iso("10/11/2026x", ra, sizeof ra));
    KT(!nl_so_nguyen("12a", &v, 0, 100));
    KT(!nl_so_nguyen("-1", &v, -5, 100));
    KT(!nl_so_nguyen("99999999999999999999", &v, 0, 100));
    KT(!nl_so_nguyen("", &v, 0, 100));
    KT(nl_so_nguyen("42", &v, 0, 100) && v == 42);
    KT(nl_do_rong("Nguyễn") == 6);
    KT(nl_do_rong("Nguye\xCC\x82\xCC\x83n") == 6); /* dấu tổ hợp không chiếm thêm ô */
    nl_cat_hien_thi("Nguyễn Văn An", 8, ra, sizeof ra);
    KT(strcmp(ra, "Nguyễn …") == 0);
    KT(nl_do_rong(ra) == 8);
    nl_cat_hien_thi("Ngắn", 8, ra, sizeof ra);
    KT(strcmp(ra, "Ngắn") == 0);
    nl_tien(160000, ra, sizeof ra);
    KT(strcmp(ra, "160.000") == 0);
    nl_tien(0, ra, sizeof ra);
    KT(strcmp(ra, "0") == 0);
    nl_tien(1234567890LL, ra, sizeof ra);
    KT(strcmp(ra, "1.234.567.890") == 0);
    strcpy(ra, "  An  ");
    nl_cat_khoang_trang(ra);
    KT(strcmp(ra, "An") == 0);
}

int main(void) {
    kt_lich();
    kt_ve();
    kt_de_quy();
    kt_thong_ke();
    kt_tep();
    kt_nhap_lieu();
    printf("\nKet qua: %d dat, %d loi\n", so_dat, so_loi);
    return so_loi ? 1 : 0;
}
