#include "vga_driver.h"

int main(void) {
    // TEST TO GET A RED LINE ON VGA
    vga_init();

    while (1) {
        for (int x = 0; x < 320; x++) {
            plot_pixel(x, 120, 0xFFFF);
        }

        wait_for_vsync();
    }

    return 0;
}
