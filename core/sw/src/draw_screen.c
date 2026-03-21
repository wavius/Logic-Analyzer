#include "draw_screen.h"

#include "../../Computer_Systems/DE1-SoC/software/address_map_niosV.h"
#include "vga_driver.h"

// ----- Screen constants ----- //
#define SCREEN_W 320
#define SCREEN_H 240

// ----- Char buffer ----- //
#define CHAR_COLS 80
#define CHAR_ROWS 60

// HARD CODED CHANNEL VALUE
#define CHANNEL_LIMIT 16
/********************************
 *  Structs + global variables
 ********************************/
//-- screen sizing variables --//
static const int top_bar_height = 15;
static const int left_bar_width = 32;  // cause there are 8 major vertical grid ticks. (320 - 32)/8 = 36
static const int bottom_bar_height = 7;
static const int channel_area_height = SCREEN_H - (top_bar_height + bottom_bar_height);

//-- waveform / grid layout variables --//
static const int vertical_grid_ticks = 8;
static const int grid_spacing_x = (SCREEN_W - left_bar_width) / 8;
static const int waveform_margin_divisor = 4;
static const int waveform_min_margin = 1;

//-- UI color variables --//
static const uint16_t top_bar_color = 0x18e3;
static const uint16_t bottom_bar_color = 0x18e3;
static const uint16_t left_bar_color = 0x18e3;
static const uint16_t separator_color = 0x39c7;
static const uint16_t grid_color = 0x18e3;
static const uint16_t text_color = 0xd69a;

// color options for the signals drawn to screen
static const uint16_t channel_colors[16] = {
    0x5D6B, 0x8E24, 0x8E24, 0xC7C0,
    0xE7E0, 0xF580, 0xE3A0, 0xD820,
    0x72A9, 0x5249, 0x3A89, 0x44CB,
    0x5CFE, 0x65FF, 0x65D7, 0x61ED};

/********************************
 *  Helper Function Declarations (idk if we need )
 ********************************/
// ----- Basic Drawing Stuff ----- //
static void draw_hline(int x_start, int x_end, int y, uint16_t color);
static void draw_vline(int x, int y_start, int y_end, uint16_t color);
static void fill_rect(int x_cord, int y_cord, int w, int h, uint16_t color);
static int calculate_channel_height(const int lanes, const int available_height);
static void text_plot_char(int col, int row, char c);
static void text_draw_string(int col, int row, const char* text);
static void text_clear(void);
static void draw_channel_labels(const Channel* channels, int lanes);
static uint16_t dim_color(uint16_t color);
static void draw_channel_labels(const Channel* channels, const int lanes);

/********************************
 *  Helper Functions
 ********************************/
// draw a horizontal line
static void draw_hline(int x_start, int x_end, int y, uint16_t color) {
    for (int x = x_start; x <= x_end; x++)
        plot_pixel(x, y, color);
}

// draw a vertical line
static void draw_vline(int x, int y_start, int y_end, uint16_t color) {
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
static void fill_rect(int x_cord, int y_cord, int w, int h, uint16_t color) {
    // note: x_cord and y_cord define the top left corner of the rectangle being drawn
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            plot_pixel(x_cord + i, y_cord + j, color);
}

// if want n channels, draw n - 1 lines spaced (available_height / lanes) apart
static int calculate_channel_height(const int lanes, const int available_height) {
    if (lanes <= 0)
        return 0;
    return available_height / lanes;
}

// store a single char in a string buffer
static void text_plot_char(int col, int row, char c) {
    if (col < 0 || col >= CHAR_COLS || row < 0 || row >= CHAR_ROWS)
        return;

    // pointer to controller
    volatile int* ctrl = (int*)CHAR_BUF_CTRL_BASE;

    // first register = character buffer address
    volatile char* char_buf = (volatile char*)(ctrl[0]);

    char_buf[row * CHAR_COLS + col] = c;
}

// draw text to screen
static void text_draw_string(int col, int row, const char* text) {
    if (text == 0)
        return;

    int cur_col = col;
    int cur_row = row;

    while (*text) {
        if (*text == '\n') {
            cur_row++;
            cur_col = col;
        } else {
            text_plot_char(cur_col, cur_row, *text);
            cur_col++;

            if (cur_col >= CHAR_COLS) {
                cur_col = col;
                cur_row++;
            }
        }

        if (cur_row >= CHAR_ROWS)
            break;

        text++;
    }
}

// clear buffer or smth? idk
static void text_clear(void) {
    volatile int* ctrl = (int*)CHAR_BUF_CTRL_BASE;
    volatile char* char_buf = (volatile char*)(ctrl[0]);

    for (int row = 0; row < CHAR_ROWS; row++) {
        for (int col = 0; col < CHAR_COLS; col++) {
            char_buf[row * CHAR_COLS + col] = ' ';
        }
    }
}

// for dimming the color of text drawn to screen (used for channel labels for example)
static uint16_t dim_color(uint16_t color) {
    uint16_t r = (color >> 11) & 0x1F;
    uint16_t g = (color >> 5) & 0x3F;
    uint16_t b = color & 0x1F;

    r >>= 1;
    g >>= 1;
    b >>= 1;

    return (r << 11) | (g << 5) | b;
}

// draws labels
static void draw_channel_labels(const Channel* channels, const int lanes) {
    if (channels == 0 || lanes <= 0 || lanes > CHANNEL_LIMIT)
        return;

    int lane_height = calculate_channel_height(lanes, channel_area_height);

    // character placement inside left panel
    // 80 cols over 320 px => 4 px per char cell
    // 60 rows over 240 px => 4 px per char cell
    const int text_col = 2;  // a little padding from left side
    const int stripe_width = 2;

    for (int i = 0; i < lanes; i++) {
        int y_top = top_bar_height + i * lane_height;

        // center label vertically in lane
        int label_row = (y_top + lane_height / 2) / 4;

        uint16_t stripe_color = channels[i].enabled
                                    ? channel_colors[i]
                                    : dim_color(channel_colors[i]);

        uint16_t label_bg = channels[i].enabled
                                ? left_bar_color
                                : dim_color(left_bar_color);

        // fill the label area background per-lane
        fill_rect(0, y_top, left_bar_width, lane_height, label_bg);

        // draw color stripe at far left
        fill_rect(0, y_top, stripe_width, lane_height, stripe_color);

        // clear a small text band inside the label area so text is readable
        // optional but helps consistency
        // here we just rely on the background already drawn

        // draw the channel name
        if (channels[i].label[0] != '\0') {  // make sure string isn't empty (first element would be the null terminator )
            text_draw_string(text_col, label_row, channels[i].label);
        }
    }
}

/********************************
 *  Function Implementations
 ********************************/
// draws the main static part of the background
void draw_logic_ui_frame(const Channel* channels, const int lanes) {
    if (lanes <= 0 || (lanes > CHANNEL_LIMIT))
        return;

    // Top bar
    fill_rect(0, 0, SCREEN_W, top_bar_height, top_bar_color);

    // bottom bar
    fill_rect(0, SCREEN_H - bottom_bar_height, SCREEN_W, bottom_bar_height, bottom_bar_color);

    // Left label column
    fill_rect(0, top_bar_height, left_bar_width, channel_area_height, left_bar_color);

    // Vertical grid lines
    for (int x = left_bar_width; x < SCREEN_W; x += grid_spacing_x)
        draw_vline(x, top_bar_height, SCREEN_H - bottom_bar_height - 1, grid_color);

    // Channel separators
    int spacing = calculate_channel_height(lanes, channel_area_height);
    for (int i = 1; i < lanes; i++) {
        int y = top_bar_height + i * spacing;
        draw_hline(0, SCREEN_W - 1, y, separator_color);
    }

    // labels + stripes
    draw_channel_labels(channels, lanes);
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

// draw signals (no zooming and scrolling logic, just prints start of given sample buffer)
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

// handle zooming logic by determining the sample window for each enabled channel and prints it out using draw_digital_waveform(...)
void draw_logic_view(const VisualizerState* state, const Channel* channels, int lanes) {
    if (state == 0 || channels == 0 || lanes <= 0)
        return;

    uint32_t start = state->start_sample;
    uint32_t end = visualizer_get_end_sample(state);

    if (end <= start)
        return;

    uint32_t visible_count = end - start;

    int lane_height = calculate_channel_height(lanes, channel_area_height);
    int x_start = left_bar_width;
    int waveform_width = SCREEN_W - left_bar_width;

    for (int i = 0; i < lanes; i++) {
        if (!channels[i].enabled)
            continue;

        int y_top = top_bar_height + i * lane_height;

        draw_digital_waveform(
            &channels[i].samples[start],  // shifted pointer
            visible_count,
            x_start,
            y_top,
            waveform_width,
            lane_height,
            channel_colors[i]);
    }
}