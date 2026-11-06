/* main.c - Điểm vào của chương trình Quản lý đặt vé xem phim. */
#include <stdio.h>

#include "manhinh.h"
#include "ui.h"

int main(void) {
    if (ui_khoi_dong() != 0) {
        fputs("Chuong trinh can chay trong cua so console cua Windows 10 tro len.\n", stderr);
        return 1;
    }
    man_chay("data.txt");
    ui_ket_thuc();
    return 0;
}
