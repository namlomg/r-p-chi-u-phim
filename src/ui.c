/* ui.c - Thư viện giao diện console. Chỉ tệp này gọi Console API và in mã điều khiển. */
#define WIN32_LEAN_AND_MEAN
#include "ui.h"

#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wctype.h>

#include "nhaplieu.h"

#define CO_BO_DEM 65536
#define CAO_TOI_THIEU (UI_CAO + 1)
#define RONG_TOI_THIEU (UI_RONG + 2)

static HANDLE h_ra, h_vao;
static DWORD che_do_ra_cu, che_do_vao_cu;
static char bo_dem[CO_BO_DEM];
static size_t do_dai;
static int cua_so_rong, cua_so_cao, goc_hang, goc_cot;
static int can_xoa_man = 1;

void ui_xuat(void) {
    static wchar_t rong[CO_BO_DEM];
    int con, da = 0;
    if (!do_dai) return;
    con = MultiByteToWideChar(CP_UTF8, 0, bo_dem, (int)do_dai, rong, CO_BO_DEM);
    do_dai = 0;
    while (con > 0) {
        DWORD viet = 0;
        if (!WriteConsoleW(h_ra, rong + da, (DWORD)(con > 8192 ? 8192 : con), &viet, NULL) || !viet)
            return;
        da += (int)viet;
        con -= (int)viet;
    }
}

static void them(const char *s) {
    size_t n = strlen(s);
    if (do_dai + n > CO_BO_DEM) ui_xuat();
    if (n > CO_BO_DEM) return;
    memcpy(bo_dem + do_dai, s, n);
    do_dai += n;
}

static void themf(const char *dinh_dang, ...) {
    char tam[512];
    va_list ds;
    va_start(ds, dinh_dang);
    vsnprintf(tam, sizeof tam, dinh_dang, ds);
    va_end(ds);
    them(tam);
}

static void dat_mau(int fg, int bg) { themf("\x1b[38;5;%d;48;5;%dm", fg, bg); }

static void di_toi(int hang, int cot) { themf("\x1b[%d;%dH", goc_hang + hang + 1, goc_cot + cot + 1); }

/* Trả về 1 nếu kích thước cửa sổ khác lần đo trước. */
static int do_cua_so(void) {
    CONSOLE_SCREEN_BUFFER_INFO tt;
    int r, c;
    if (!GetConsoleScreenBufferInfo(h_ra, &tt)) return 0;
    r = tt.srWindow.Right - tt.srWindow.Left + 1;
    c = tt.srWindow.Bottom - tt.srWindow.Top + 1;
    if (r == cua_so_rong && c == cua_so_cao) return 0;
    cua_so_rong = r;
    cua_so_cao = c;
    goc_cot = r > UI_RONG ? (r - UI_RONG) / 2 : 0;
    goc_hang = c > UI_CAO ? (c - UI_CAO) / 2 : 0;
    return 1;
}

/* Font raster không có đủ ký tự tiếng Việt, nên đổi sang Consolas nếu cần. */
static void dam_bao_font(void) {
    CONSOLE_FONT_INFOEX f;
    f.cbSize = sizeof f;
    if (!GetCurrentConsoleFontEx(h_ra, FALSE, &f)) return;
    if (f.FontFamily & TMPF_TRUETYPE) return;
    f.nFont = 0;
    f.dwFontSize.X = 0;
    f.dwFontSize.Y = 18;
    f.FontFamily = FF_DONTCARE;
    f.FontWeight = FW_NORMAL;
    wcscpy(f.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(h_ra, FALSE, &f);
}

int ui_khoi_dong(void) {
    h_ra = GetStdHandle(STD_OUTPUT_HANDLE);
    h_vao = GetStdHandle(STD_INPUT_HANDLE);
    if (!GetConsoleMode(h_ra, &che_do_ra_cu) || !GetConsoleMode(h_vao, &che_do_vao_cu)) return -1;
    if (!SetConsoleMode(h_ra, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING |
                                  DISABLE_NEWLINE_AUTO_RETURN))
        return -1;
    SetConsoleMode(h_vao, ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS);
    dam_bao_font();
    SetConsoleTitleW(L"Rạp chiếu phim - Quản lý đặt vé");
    them("\x1b[?1049h\x1b[?25l");
    ui_xuat();
    do_cua_so();
    can_xoa_man = 1;
    return 0;
}

void ui_ket_thuc(void) {
    them("\x1b[0m\x1b[?25h\x1b[?1049l");
    ui_xuat();
    SetConsoleMode(h_ra, che_do_ra_cu);
    SetConsoleMode(h_vao, che_do_vao_cu);
}

int ui_doc_phim(void) {
    for (;;) {
        INPUT_RECORD bg;
        DWORD n;

        if (do_cua_so()) {
            can_xoa_man = 1;
            return K_DOI_CO;
        }
        if (WaitForSingleObject(h_vao, 150) != WAIT_OBJECT_0) continue;
        if (!ReadConsoleInputW(h_vao, &bg, 1, &n) || n == 0) continue;
        if (bg.EventType != KEY_EVENT || !bg.Event.KeyEvent.bKeyDown) continue;

        switch (bg.Event.KeyEvent.wVirtualKeyCode) {
        case VK_UP: return K_LEN;
        case VK_DOWN: return K_XUONG;
        case VK_LEFT: return K_TRAI;
        case VK_RIGHT: return K_PHAI;
        case VK_HOME: return K_HOME;
        case VK_END: return K_END;
        case VK_PRIOR: return K_PGUP;
        case VK_NEXT: return K_PGDN;
        case VK_RETURN: return K_ENTER;
        case VK_ESCAPE: return K_ESC;
        case VK_BACK: return K_BACK;
        case VK_TAB: return K_TAB;
        default: break;
        }
        {
            wchar_t ch = bg.Event.KeyEvent.uChar.UnicodeChar;
            if (ch >= 32 && ch != 127 && !(ch >= 0xD800 && ch <= 0xDFFF)) return (int)ch;
        }
    }
}

void ui_in(int hang, int cot, int fg, int bg, const char *dinh_dang, ...) {
    char tam[1024];
    va_list ds;
    va_start(ds, dinh_dang);
    vsnprintf(tam, sizeof tam, dinh_dang, ds);
    va_end(ds);
    dat_mau(fg, bg);
    di_toi(hang, cot);
    them(tam);
}

void ui_o(int hang, int cot, int rong, int fg, int bg, int can, const char *chuoi) {
    char cat[768];
    int w, trai = 0, phai, i;

    nl_cat_hien_thi(chuoi, rong, cat, sizeof cat);
    w = nl_do_rong(cat);
    if (can == 1) trai = rong - w;
    else if (can == 2) trai = (rong - w) / 2;
    phai = rong - w - trai;

    dat_mau(fg, bg);
    di_toi(hang, cot);
    for (i = 0; i < trai; i++) them(" ");
    them(cat);
    for (i = 0; i < phai; i++) them(" ");
}

static void lap(const char *s, int n) {
    while (n-- > 0) them(s);
}

/* Chờ đến khi cửa sổ đủ lớn để chứa khung. */
static void cho_du_rong(void) {
    while (cua_so_rong < RONG_TOI_THIEU || cua_so_cao < CAO_TOI_THIEU) {
        themf("\x1b[0m\x1b[2J\x1b[H\x1b[?25l");
        themf("Cửa sổ đang là %dx%d, chương trình cần ít nhất %dx%d.\r\n", cua_so_rong, cua_so_cao,
              RONG_TOI_THIEU, CAO_TOI_THIEU);
        them("Hãy phóng to cửa sổ để tiếp tục.");
        ui_xuat();
        while (ui_doc_phim() != K_DOI_CO) {
        }
    }
}

void ui_ke_ngang(int hang) {
    dat_mau(M_VIEN, M_NEN);
    di_toi(hang, 0);
    them("╠");
    lap("═", UI_RONG - 2);
    them("╣");
}

void ui_khung(const char *duong_dan, const char *trang_thai, int mau_tt, const char *phim_tat) {
    int h, rong_tt = trang_thai ? nl_do_rong(trang_thai) : 0;

    cho_du_rong();
    them("\x1b[?25l");
    if (can_xoa_man) {
        dat_mau(M_CHU, M_NEN);
        them("\x1b[2J");
        can_xoa_man = 0;
    }

    dat_mau(M_VIEN, M_NEN);
    di_toi(0, 0);
    them("╔");
    lap("═", UI_RONG - 2);
    them("╗");
    for (h = 1; h < UI_CAO - 1; h++) {
        dat_mau(M_VIEN, M_NEN);
        di_toi(h, 0);
        them("║");
        dat_mau(M_CHU, M_NEN);
        lap(" ", UI_RONG - 2);
        dat_mau(M_VIEN, M_NEN);
        them("║");
    }
    di_toi(UI_CAO - 1, 0);
    them("╚");
    lap("═", UI_RONG - 2);
    them("╝");

    ui_ke_ngang(2);
    ui_ke_ngang(UI_CAO - 3);

    ui_in(1, 3, M_VANG, M_NEN, "RẠP CHIẾU PHIM");
    if (duong_dan) {
        char cat[256];
        nl_cat_hien_thi(duong_dan, UI_RONG - 26 - rong_tt, cat, sizeof cat);
        ui_in(1, 18, M_MO, M_NEN, "›");
        ui_in(1, 20, M_CHU, M_NEN, "%s", cat);
    }
    if (trang_thai) ui_in(1, UI_RONG - 3 - rong_tt, mau_tt, M_NEN, "%s", trang_thai);
    if (phim_tat) ui_o(UI_CAO - 2, 3, UI_RONG - 6, M_MO, M_NEN, 0, phim_tat);
}

int ui_nhap(int hang, int cot, int rong, char *bd, size_t co, int toi_da, int co_nhap) {
    wchar_t w[256];
    int n, phim = K_ESC, ket_thuc = 0;

    if (toi_da > 255) toi_da = 255;
    n = MultiByteToWideChar(CP_UTF8, 0, bd, -1, w, 256);
    n = n > 0 ? n - 1 : 0;
    if (n > toi_da) n = toi_da;
    w[n] = 0;

    while (!ket_thuc) {
        int vi_tri;
        ui_o(hang, cot, rong, M_TRANG, M_O, 0, bd);
        vi_tri = nl_do_rong(bd);
        if (vi_tri >= rong) vi_tri = rong - 1;
        di_toi(hang, cot + vi_tri);
        them("\x1b[?25h");
        ui_xuat();

        phim = ui_doc_phim();
        switch (phim) {
        case K_ENTER: case K_ESC: case K_LEN: case K_XUONG: case K_TAB: case K_DOI_CO:
            ket_thuc = 1;
            break;
        case K_BACK:
            if (n > 0) w[--n] = 0;
            break;
        default:
            if (phim < 0x110000 && n < toi_da) {
                wchar_t ch = (wchar_t)phim;
                char thu[1024];
                int dai;
                if (co_nhap & UI_NHAP_HOA) {
                    ch = (wchar_t)towupper(ch);
                    if (!((ch >= L'A' && ch <= L'Z') || (ch >= L'0' && ch <= L'9'))) break;
                }
                if ((co_nhap & UI_NHAP_SO) && !(ch >= L'0' && ch <= L'9')) break;
                if (ch == L'|') break; /* dấu phân cách trong tệp dữ liệu */
                w[n] = ch;
                w[n + 1] = 0;
                dai = WideCharToMultiByte(CP_UTF8, 0, w, -1, thu, sizeof thu, NULL, NULL);
                if (dai <= 0 || (size_t)dai > co) {
                    w[n] = 0;
                    break;
                }
                n++;
            }
            break;
        }
        WideCharToMultiByte(CP_UTF8, 0, w, -1, bd, (int)co, NULL, NULL);
    }
    them("\x1b[?25l");
    return phim;
}

int ui_chon(const char *tieu_de, int mau, const char *noi_dung, const char *const *nut, int so_nut,
            int mac_dinh) {
    enum { RONG_HOP = 58, TOI_DA_DONG = 8 };
    char dong[TOI_DA_DONG][256];
    int so_dong = 0, cao, tren, trai = (UI_RONG - RONG_HOP) / 2, chon = mac_dinh, i;
    const char *p = noi_dung;

    while (*p && so_dong < TOI_DA_DONG) {
        const char *het = strchr(p, '\n');
        size_t n = het ? (size_t)(het - p) : strlen(p);
        if (n >= sizeof dong[0]) n = sizeof dong[0] - 1;
        memcpy(dong[so_dong], p, n);
        dong[so_dong][n] = '\0';
        so_dong++;
        p = het ? het + 1 : p + n;
    }
    cao = so_dong + 5;
    tren = (UI_CAO - cao) / 2;

    for (;;) {
        int tong = 0, x, phim;

        cho_du_rong();
        dat_mau(mau, M_O);
        di_toi(tren, trai);
        them("┌");
        lap("─", RONG_HOP - 2);
        them("┐");
        ui_in(tren, trai + 2, mau, M_O, " %s ", tieu_de);
        for (i = 1; i < cao - 1; i++) {
            dat_mau(mau, M_O);
            di_toi(tren + i, trai);
            them("│");
            ui_o(tren + i, trai + 1, RONG_HOP - 2, M_CHU, M_O, 0, "");
            dat_mau(mau, M_O);
            di_toi(tren + i, trai + RONG_HOP - 1);
            them("│");
        }
        dat_mau(mau, M_O);
        di_toi(tren + cao - 1, trai);
        them("└");
        lap("─", RONG_HOP - 2);
        them("┘");

        for (i = 0; i < so_dong; i++)
            ui_o(tren + 2 + i - 0, trai + 3, RONG_HOP - 6, M_CHU, M_O, 2, dong[i]);

        for (i = 0; i < so_nut; i++) tong += nl_do_rong(nut[i]) + 4;
        tong += (so_nut - 1) * 2;
        x = trai + (RONG_HOP - tong) / 2;
        for (i = 0; i < so_nut; i++) {
            int w = nl_do_rong(nut[i]) + 4;
            if (i == chon) ui_in(tren + cao - 2, x, M_TOI, M_VANG, "  %s  ", nut[i]);
            else ui_in(tren + cao - 2, x, M_CHU, M_NEN, "  %s  ", nut[i]);
            x += w + 2;
        }
        ui_xuat();

        phim = ui_doc_phim();
        if (phim == K_ENTER) return chon;
        if (phim == K_ESC) return -1;
        if (phim == K_TRAI && chon > 0) chon--;
        if ((phim == K_PHAI || phim == K_TAB) && chon < so_nut - 1) chon++;
        if (phim == K_DOI_CO) {
            dat_mau(M_CHU, M_NEN);
            them("\x1b[2J");
            can_xoa_man = 1;
        }
    }
}

void ui_bao_loi(const char *noi_dung) {
    const char *nut[] = {"Đã hiểu"};
    ui_chon("Không thực hiện được", M_DO, noi_dung, nut, 1, 0);
}

void ui_bao_xong(const char *noi_dung) {
    const char *nut[] = {"OK"};
    ui_chon("Thành công", M_XANH, noi_dung, nut, 1, 0);
}

int ui_xac_nhan(const char *noi_dung, const char *co, const char *khong) {
    const char *nut[2];
    nut[0] = co;
    nut[1] = khong;
    return ui_chon("Xác nhận", M_VANG, noi_dung, nut, 2, 1) == 0;
}
