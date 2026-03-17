#include "../software/address_map_niosV.h"
#include "vga_driver.h"

int main(void) {
    // TEST TO GET A RED LINE ON VGA
    vga_init();
    clear_screen();

    // draw a red horizontal line
    for (int i = 0; i < 320; i++)
        plot_pixel(i, 120, 0xF800);

    while (1);
    return 0;
}
