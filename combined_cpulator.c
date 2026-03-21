/* --- START OF core/sw/draw_screen.h --- */
#ifndef DRAW_SCREEN_H
#define DRAW_SCREEN_H

#include <stdbool.h>
#include <stdint.h>

// ----- Structs ----- //
// all info needed to draw a singal
typedef struct {
    const uint8_t* samples;
    int count;
    // uint16_t color;  // bring back this feature later if I think of a better way to include it
    bool enabled;
    char label[5];
} Channel;

// ----- Logic analyzer UI ----- //
void draw_logic_ui_frame(const int lanes);
void draw_digital_waveform(const uint8_t* samples, const int count, int x0, int y0, int w, int h, uint16_t color);
void draw_signals(const Channel* channels, const int lanes);

#endif
/* --- END OF core/sw/draw_screen.h --- */

/* --- START OF core/sw/vga_driver.h --- */
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
/* --- END OF core/sw/vga_driver.h --- */

/* --- START OF core/sw/vga_driver.c --- */
// Removed include: core/sw/vga_driver.c

#include <string.h>  //for memset

// Removed include: core/sw/vga_driver.c

/********************************
 *  Structs
 ********************************/
struct frame_buffer_controller {
    volatile uint32_t frontBuffer;
    volatile uint32_t backBuffer;
    volatile uint16_t xRes;
    volatile uint16_t yRes;
    volatile uint32_t statusReg;
};

/********************************
 *  Global Variables
 ********************************/
volatile struct frame_buffer_controller* frameBuf =
    (struct frame_buffer_controller*)0xFF203020;
volatile int pixel_buffer_start;

// ----- Two frame buffers (double buffering) ----- //
uint16_t Buffer1[240][512];
uint16_t Buffer2[240][512];

/********************************
 *  Function Implementations
 ********************************/
// Initialize VGA system (from Lab 7)
void vga_init() {
    // Intialize front buffer to Buffer1, then swap to make it front
    frameBuf->backBuffer =
        (uint32_t)Buffer1;  // store in the back buffer the address of the 1st buffer
    wait_for_vsync();       // swap so buffer 1 address is held in front buffer

    // Set drawing pointer to front buffer and clear it
    pixel_buffer_start =
        frameBuf->frontBuffer;  // get front buffer, and then clear it
    clear_screen();

    // Intialize back buffer as Buffer2
    frameBuf->backBuffer = (uint32_t)Buffer2;

    //  clear the back buffer
    pixel_buffer_start = frameBuf->backBuffer;
    clear_screen();
}

// Wait for vertical sync and swap buffers
void wait_for_vsync() {
    frameBuf->frontBuffer = 1;        // request swap
    while (frameBuf->statusReg & 1);  // poll the status bit to see if the swap is done
    pixel_buffer_start =
        frameBuf->backBuffer;  // set a new main buffer to write into
}

// Clear entire screen
void clear_screen() {
    // 240 rows × 1024 bytes per row = 245760 bytes
    memset((void*)pixel_buffer_start, 0, 245760);  // write black into all the space in the front buffer
}

// Plot a single pixel
void plot_pixel(int x, int y, uint16_t color) {
    volatile uint16_t* pixel_addr;
    pixel_addr = (volatile uint16_t*)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_addr = color;
}

/*
// implemented for debugging purposes
int getXres() {
    return frameBuf->xRes;
}
*/

/* --- END OF core/sw/vga_driver.c --- */

/* --- START OF core/sw/draw_screen.c --- */
// Removed include: core/sw/draw_screen.c

// Removed include: core/sw/draw_screen.c

// ----- Screen constants ----- //
#define SCREEN_W 320
#define SCREEN_H 240

// HARD CODED CHANNEL VALUE
#define CHANNEL_LIMIT 16
/********************************
 *  Structs + global variables
 ********************************/
//-- screen sizing variables --//
const int top_bar_height = 15;
const int left_bar_width = 32;  // cause there are 8 major vertical grid ticks. (320 - 32)/8 = 36
const int bottom_bar_height = 7;
const int channel_area_height = SCREEN_H - (top_bar_height + bottom_bar_height);

//-- waveform / grid layout variables --//
const int vertical_grid_ticks = 8;
const int grid_spacing_x = (SCREEN_W - left_bar_width) / 8;
const int waveform_margin_divisor = 4;
const int waveform_min_margin = 1;

//-- UI color variables --//
const uint16_t top_bar_color = 0x18e3;
const uint16_t bottom_bar_color = 0x18e3;
const uint16_t left_bar_color = 0x18e3;
const uint16_t separator_color = 0x39c7;
const uint16_t grid_color = 0x18e3;

// color options for the signals drawn to screen
const uint16_t channel_colors[16] = {
    0x5D6B, 0x8E24, 0x8E24, 0xC7C0,
    0xE7E0, 0xF580, 0xE3A0, 0xD820,
    0x72A9, 0x5249, 0x3A89, 0x44CB,
    0x5CFE, 0x65FF, 0x65D7, 0x61ED};

/********************************
 *  Helper Function Declarations (idk if we need )
 ********************************/
// ----- Basic Drawing Stuff ----- //
void draw_hline(int x_start, int x_end, int y, uint16_t color);
void draw_vline(int x, int y_start, int y_end, uint16_t color);
void fill_rect(int x_cord, int y_cord, int w, int h, uint16_t color);
int calculate_channel_height(const int lanes, const int available_height);
void draw_text();  // temp, unfinished

/********************************
 *  Helper Functions
 ********************************/
// draw a horizontal line
void draw_hline(int x_start, int x_end, int y, uint16_t color) {
    for (int x = x_start; x <= x_end; x++)
        plot_pixel(x, y, color);
}

// draw a vertical line
void draw_vline(int x, int y_start, int y_end, uint16_t color) {
    // swap for negative lines
    if (y_start > y_end) {
        int temp = y_start;
        y_start = y_end;
        y_end = temp;
    }

    for (int y = y_start; y <= y_end; y++)
        plot_pixel(x, y, color);
}

// draw a filled rectangle (to draw bars / labels)
void fill_rect(int x_cord, int y_cord, int w, int h, uint16_t color) {
    // note: x_cord and y_cord define the top left corner of the rectangle being drawn
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            plot_pixel(x_cord + i, y_cord + j, color);
}

// if want n channels, draw n - 1 lines spaced (available_height / lanes) apart
int calculate_channel_height(const int lanes, const int available_height) {
    if (lanes <= 0)
        return 0;
    return available_height / lanes;
}

// draw text to screen
void draw_text() {
    // draw title
    // draw channel labels
    // draw grid numbers
    // draw wtv else
}

/********************************
 *  Function Implementations
 ********************************/
// draws the main static part of the background
void draw_logic_ui_frame(const int lanes) {
    if (lanes <= 0 || (lanes > CHANNEL_LIMIT))
        return;

    // Top bar
    fill_rect(0, 0, SCREEN_W, top_bar_height, top_bar_color);

    // bottom bar
    fill_rect(0, SCREEN_H - bottom_bar_height, SCREEN_W, bottom_bar_height, bottom_bar_color);

    // Left label column
    fill_rect(0, top_bar_height, left_bar_width, channel_area_height, left_bar_color);

    // Channel separators
    int spacing = calculate_channel_height(lanes, channel_area_height);
    for (int i = 1; i < lanes; i++) {
        int y = top_bar_height + i * spacing;
        draw_hline(0, SCREEN_W - 1, y, separator_color);
    }

    // Vertical grid lines
    for (int x = left_bar_width; x < SCREEN_W; x += grid_spacing_x)
        draw_vline(x, top_bar_height, SCREEN_H - bottom_bar_height - 1, grid_color);

    draw_text();
}

// based on recieved array samples and count (the amount of cycles), draws any given digital waveform
void draw_digital_waveform(const uint8_t* samples, const int count, int x_start, int y_top, int draw_width, int draw_heigth, uint16_t color) {
    if (draw_width <= 0 || draw_heigth <= 0)
        return;

    int margin = draw_heigth / waveform_margin_divisor;
    if (margin < waveform_min_margin)
        margin = waveform_min_margin;

    int y_high = y_top + margin;
    int y_low = y_top + draw_heigth - 1 - margin;

    if (y_low < y_high)
        y_low = y_high;

    if (count <= 0 || samples == 0) {  // given there is no waveform given, just draw a horizontal line
        draw_hline(x_start, x_start + draw_width - 1, y_low, color);
        return;
    }

    int prev = samples[0] ? 1 : 0;
    int prev_y = prev ? y_high : y_low;

    for (int x = 0; x < draw_width; x++) {
        int idx = (x * count) / draw_width;  // automatically scale the amount of samples given to fit the screen
        if (idx >= count)
            idx = count - 1;

        int cur = samples[idx] ? 1 : 0;
        int y = cur ? y_high : y_low;
        int screen_x = x_start + x;

        // vertical edge
        if (x > 0 && cur != prev)
            draw_vline(screen_x, prev_y, y, color);

        // horizontal segment
        plot_pixel(screen_x, y, color);

        prev = cur;
        prev_y = y;
    }
}

// draw signals
void draw_signals(const Channel* channels, const int lanes) {
    if (channels == 0 || lanes <= 0 || lanes > CHANNEL_LIMIT)
        return;

    int lane_height = calculate_channel_height(lanes, channel_area_height);
    int x_start = left_bar_width;
    int waveform_width = SCREEN_W - left_bar_width;

    // draw the signals
    for (int i = 0; i < lanes; i++) {
        if (!channels[i].enabled)
            continue;

        int y_top = top_bar_height + i * lane_height;
        draw_digital_waveform(channels[i].samples, channels[i].count, x_start, y_top, waveform_width, lane_height, channel_colors[i]);
    }
}
/* --- END OF core/sw/draw_screen.c --- */

/* --- START OF core/sw/main.c --- */
#include <stdint.h>

// Removed include: core/sw/main.c
// Removed include: core/sw/main.c

/********************************
 *  Structs + global variables
 ********************************/

/********************************
 *  Main Program
 ********************************/
// NOTE: CURRENTLY HARD CODED FOR 16 CHANNELS. DO NOT ENTER MORE. WILL LEAD TO UNDEFINED BEHAVIOUR
int main(void) {
    vga_init();  // must always be done first
    const int lanes = 8;

    // ----- Fake sample data (0 = low, 1 = high) ----- //
    static uint8_t ch0[64] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};

    static uint8_t ch1[96] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};

    static uint8_t ch2[64] = {
        0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
        1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0};

    static uint8_t ch3[64] = {
        0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1,
        0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0,
        0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0,
        1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0};

    static Channel channels[16] = {
        {ch0, 64, true, "CH0"},
        {ch1, 96, true, "CH1"},
        {ch2, 64, true, "CH2"},
        {ch3, 64, true, "CH3"},
        {0, 0, true, "CH4"},
        {0, 0, false, "CH5"},
        {0, 0, false, "CH6"},
        {0, 0, false, "CH7"},
        {0, 0, false, "CH8"},
        {0, 0, false, "CH9"},
        {0, 0, false, "CH10"},
        {0, 0, false, "CH11"},
        {0, 0, false, "CH12"},
        {0, 0, false, "CH13"},
        {0, 0, false, "CH14"},
        {0, 0, false, "CH15"}};

    while (1) {
        clear_screen();
        draw_logic_ui_frame(lanes);
        draw_signals(channels, lanes);
        wait_for_vsync();
    }

    return 0;
}
/* --- END OF core/sw/main.c --- */
