#include <stdbool.h>
#include <stdint.h>

#include "ps2_input.h"
#include "vga_driver.h"

int main(void) {
    Keyboard kb;
    KeyEvent ev;

    vga_init();
    keyboard_init(&kb);

    while (1) {
        if (keyboard_read_event(&kb, &ev)) {
            if (ev.pressed) {
                clear_screen();

                // fill screen green when any key is pressed
                for (int y = 0; y < 240; y++) {
                    for (int x = 0; x < 320; x++) {
                        plot_pixel(x, y, 0x07E0);
                    }
                }

                wait_for_vsync();
            }
        }
    }

    return 0;
}