#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include <stdint.h>

// ----- Core VGA ----- //
void vga_init();
void plot_pixel(int x, int y, uint16_t color);
void wait_for_vsync();
void clear_screen();

// ----- Other ----- //
// int getXres(); //implemented for debugging purposes

#endif