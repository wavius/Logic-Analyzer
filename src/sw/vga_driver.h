#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include <stdint.h>

// ----- Core VGA ----- //
void vga_init();
void plot_pixel(int x, int y, uint16_t color);
void wait_for_vsync();
void clear_screen();

// ----- Basic Drawing Stuff ----- //
void draw_hline(int x1, int x2, int y, uint16_t color);
void draw_vline(int x, int y1, int y2, uint16_t color);
void fill_rect(int x, int y, int w, int h, uint16_t color);

// ----- Logic analyzer UI ----- //
void draw_logic_ui_frame();
void draw_digital_waveform(const uint8_t* samples, int count, int x0, int y0, int w, int h, uint16_t color);

// ----- Other ----- //
// int getXres(); //implemented for debugging purposes

#endif