#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include <stdint.h>

// initalize the vga and get it ready to start drawing
void vga_init();

// plots pixel at coordinates x, y and given color on the screen
void plot_pixel(int x, int y, uint16_t color);

// waits for the vertical sync of the vga buffer (terrible description, fix this)
void wait_for_vsync();

// clears whole screen
void clear_screen();

int getXres();

#endif