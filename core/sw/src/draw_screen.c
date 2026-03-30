#include "draw_screen.h"

#include "address_map_niosV.h"
#include "vga_driver.h"

// ----- Screen constants ----- //
#define SCREEN_W 320
#define SCREEN_H 240

// ----- Char buffer ----- //
#define CHAR_COLS 80
#define CHAR_ROWS 60

/********************************
 *  Structs + global variables
 ********************************/
//-- screen sizing variables --//
static const int top_bar_height = 15;
static const int left_bar_width = 32;  // cause there are 8 major vertical grid ticks. (320 - 32)/8 = 36
static const int bottom_bar_height = 7;
static const int channel_area_height = 218;  // 240 - 15 - 7

//-- waveform / grid layout variables --//

static const int grid_spacing_x = 36;  // (320 - 32) / 8
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
    0xfd20, 0xfc26, 0xfb0c, 0xdacf,
    0xba92, 0x92b4, 0x3cd9, 0xbefd};

uint32_t current_page = 0;

uint8_t channel_buffers[TOTAL_SIGNALS][BUFFER_SIZE];  // sample buffers for each signal

uint8_t zero_samples[4096];

/********************************
 *  Helper Function Declarations (idk if we need )
 ********************************/
// ----- Basic Drawing Stuff ----- //
static void draw_hline(int x_start, int x_end, int y, uint16_t color);
static void draw_vline(int x, int y_start, int y_end, uint16_t color);
static void fill_rect(int x_cord, int y_cord, int w, int h, uint16_t color);
static uint16_t dim_color(uint16_t color);
static void draw_logic_view(const ZoomState* state, const Channel* channels, int lanes);
static void draw_trigger_marker(const ZoomState* state, uint32_t trigger_position);
static void draw_page_tabs();
static void draw_top_info_bar(const ZoomState* state);
static int text_draw_label_uint(int col, int row, const char* label, uint32_t value);
static void text_plot_char(int col, int row, char c);
static void text_draw_string(int col, int row, const char* text);
static void text_draw_uint(int col, int row, uint32_t value);
static void draw_time_scale(const ZoomState* state);
static void draw_channel_labels(const Channel* channels, int lanes);

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

// draw the tabs to show which pannel user is on
static void draw_page_tabs(void) {
    const int tab_width = 28;
    const int tab_height = bottom_bar_height + 3;  // a bit taller than bottom bar
    const int tab_gap = 4;

    const int y_top = SCREEN_H - tab_height;
    const int tab0_x = left_bar_width;
    const int tab1_x = left_bar_width + tab_width + tab_gap;

    const uint16_t selected_tab_color = separator_color;
    const uint16_t unselected_tab_color = dim_color(separator_color);

    // highlight the tab that is currently selected
    uint16_t tab0_color = (current_page == 0) ? selected_tab_color : unselected_tab_color;
    uint16_t tab1_color = (current_page == 1) ? selected_tab_color : unselected_tab_color;

    // draw filled tab bodies
    fill_rect(tab0_x, y_top, tab_width, tab_height, tab0_color);
    fill_rect(tab1_x, y_top, tab_width, tab_height, tab1_color);

    // draw an outline on the top of the tab just to make things a bit cleaner
    draw_hline(tab0_x, tab0_x + tab_width - 1, y_top, grid_color);
    draw_hline(tab1_x, tab1_x + tab_width - 1, y_top, grid_color);

    // character-buffer placement
    const int text_row = (y_top + (tab_height / 2) - 2) / 4;

    const char* tab0_text = "TAB 0";
    const char* tab1_text = "TAB 1";

    const int tab0_col = (tab0_x / 4) + 1;
    const int tab1_col = (tab1_x / 4) + 1;

    text_draw_string(tab0_col, text_row, tab0_text);
    text_draw_string(tab1_col, text_row, tab1_text);
}

// store a single char in a string buffer
static void text_plot_char(int col, int row, char c) {
    if (col < 0 || col >= CHAR_COLS || row < 0 || row >= CHAR_ROWS)
        return;

    volatile char* char_buf = (volatile char*)FPGA_CHAR_BASE;

    char_buf[(row << 7) + col] = c;  // same as row * 128 + col
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

// draw an unsigned integer into the character buffer without snprintf
static void text_draw_uint(int col, int row, uint32_t value) {
    char digits[10];  // max uint32_t = 4294967295
    int count = 0;

    if (row < 0 || row >= CHAR_ROWS)
        return;

    if (value == 0) {
        text_plot_char(col, row, '0');
        return;
    }

    // extract digits in reverse order
    while (value > 0 && count < 10) {
        digits[count] = '0' + (value % 10);
        value /= 10;
        count++;
    }

    // draw in correct order
    for (int i = count - 1; i >= 0; i--) {
        if (col >= CHAR_COLS)
            return;
        if (col >= 0)
            text_plot_char(col, row, digits[i]);
        col++;
    }
}

// <>
static int text_draw_label_uint(int col, int row, const char* label, uint32_t value) {
    text_draw_string(col, row, label);

    // move col forward past label
    while (*label) {
        col++;
        label++;
    }

    text_draw_uint(col, row, value);

    // return new col position (after number)
    while (value > 0) {
        col++;
        value /= 10;
    }

    return col + 2;  // spacing
}

// <>
static void draw_top_info_bar(const ZoomState* state) {
    if (state == 0)
        return;

    const int row = 0;  // top row
    int col = 3;

    // --- Time/div --- //
    col = text_draw_label_uint(col, row, "T/div:", state->time_div);
    text_draw_string(col, row, "ns ");
    col += 10;

    // --- Sample rate (convert to MHz) --- //
    uint32_t fs_mhz = state->sample_rate / 1000000;
    col = text_draw_label_uint(col, row, "Fs:", fs_mhz);
    text_draw_string(col, row, "MHz ");
    col += 10;

    // --- Window (total visible time) --- //
    uint32_t samples_per_div = state->visible_samples / 8;
    uint32_t window_ns = samples_per_div * state->time_div;

    col = text_draw_label_uint(col, row, "Win:", window_ns);
    text_draw_string(col, row, "ns ");
    col += 10;

    // --- Scroll offset -- //
    uint32_t whole_divs = state->scroll_offset / samples_per_div;
    uint32_t rem = state->scroll_offset % samples_per_div;

    uint32_t offset_ns =
        whole_divs * state->time_div +
        (rem * state->time_div) / samples_per_div;

    col = text_draw_label_uint(col, row, "Off:", offset_ns);
    text_draw_string(col, row, "ns");
}

// draw a time scale (top bar, x axis), updates as scrolls. In units ns
static void draw_time_scale(const ZoomState* state) {
    if (state == 0)
        return;

    const int divisions = 8;
    const int label_row = (top_bar_height - 1) / 4;

    uint32_t samples_per_div = state->visible_samples / divisions;
    if (samples_per_div == 0)
        return;

    uint32_t whole_divs = state->scroll_offset / samples_per_div;
    uint32_t rem_samples = state->scroll_offset % samples_per_div;

    uint32_t left_time_ns =
        whole_divs * state->time_div +
        (rem_samples * state->time_div) / samples_per_div;

    for (int i = 0; i < divisions; i++) {
        int x = left_bar_width + i * grid_spacing_x;
        int col = x / 4;

        uint32_t tick_time_ns = left_time_ns + i * state->time_div;

        text_draw_uint(col - 1, label_row, tick_time_ns);
    }
}

// draw labels and channel strips
static void draw_channel_labels(const Channel* channels, const int lanes) {
    if (channels == 0 || lanes <= 0 || lanes > TOTAL_SIGNALS)
        return;

    int lane_height = 27;

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
                                    ? channels[i].color
                                    : dim_color(channels[i].color);

        uint16_t label_bg = channels[i].enabled
                                ? left_bar_color
                                : dim_color(left_bar_color);

        // fill the label area background per-lane
        fill_rect(0, y_top, left_bar_width, lane_height, label_bg);

        // draw color stripe at far left
        fill_rect(0, y_top, stripe_width, lane_height, stripe_color);

        // draw the channel name
        if (channels[i].label[0] != '\0') {  // make sure string isn't empty (first element would be the null terminator )
            text_draw_string(text_col, label_row, channels[i].label);
        }
    }
}

// handle zooming logic by determining the sample window for each enabled channel and prints it out using draw_digital_waveform(...)
static void draw_logic_view(const ZoomState* state, const Channel* channels, int signals_per_page) {
    uint32_t start = state->scroll_offset;
    uint32_t end = visualizer_get_end_sample(state);

    if (end <= start)
        return;

    uint32_t visible_count = end - start;

    int lane_height = 27;
    int x_start = left_bar_width;
    int waveform_width = SCREEN_W - left_bar_width;

    for (int i = 0; i < signals_per_page; i++) {
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
            channels[i].color);
    }
}

// draw vertical trigger marker line across waveform area
static void draw_trigger_marker(const ZoomState* state, uint32_t trigger_position) {
    if (state == 0 || state->visible_samples == 0)
        return;

    uint32_t start = state->scroll_offset;
    uint32_t end = visualizer_get_end_sample(state);

    // trigger not visible on current screen
    if (trigger_position < start || trigger_position >= end)
        return;

    int waveform_width = SCREEN_W - left_bar_width;
    int x_start = left_bar_width;

    uint32_t samples_from_left = trigger_position - start;
    int x = x_start + (samples_from_left * (waveform_width - 1)) / state->visible_samples;

    if (x < left_bar_width)
        x = left_bar_width;
    if (x >= SCREEN_W)
        x = SCREEN_W - 1;

    draw_vline(x, top_bar_height, SCREEN_H - bottom_bar_height - 1, 0xFFE0);
}

/********************************
 *  Function Implementations
 ********************************/
// clear buffer
void text_clear() {
    volatile char* char_buf = (volatile char*)FPGA_CHAR_BASE;

    for (int row = 0; row < CHAR_ROWS; row++) {
        volatile char* p = char_buf + (row << 7);
        for (int col = 0; col < CHAR_COLS; col++) {
            *p++ = ' ';
        }
    }
}

// draws the main static part of the background
void draw_logic_ui_frame(const Channel* channels, const int lanes) {
    if (lanes <= 0 || (lanes > TOTAL_SIGNALS))
        return;

    // Top bar
    fill_rect(0, 0, SCREEN_W, top_bar_height, top_bar_color);

    // bottom bar
    fill_rect(0, SCREEN_H - bottom_bar_height, SCREEN_W, bottom_bar_height, bottom_bar_color);

    // Left label column
    fill_rect(0, top_bar_height, left_bar_width, channel_area_height, left_bar_color);

    // Vertical grid lines
    for (int x = left_bar_width; x < SCREEN_W; x += grid_spacing_x) {
        draw_vline(x, top_bar_height, SCREEN_H - bottom_bar_height - 1, grid_color);
    }

    // Channel separators
    // int spacing = 27;
    for (int i = 1; i < lanes; i++) {
        int y = top_bar_height + i * 27;
        draw_hline(0, SCREEN_W - 1, y, separator_color);
    }
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
void draw_signals(const ZoomState* state, const Channel* channels, const int signals_per_page) {
    if (state == 0 || channels == 0 || signals_per_page != TOTAL_SIGNALS_ON_SCREEN)
        return;
    draw_logic_view(state, channels, signals_per_page);
}

// draw given page
void draw_ui_page(const Channel* channels, const ZoomState* state, uint32_t trigger_position) {
    int start_index = current_page * TOTAL_SIGNALS_ON_SCREEN;  // either 0 or 8
    draw_logic_ui_frame(&channels[start_index], TOTAL_SIGNALS_ON_SCREEN);
    text_clear();
    draw_top_info_bar(state);
    draw_channel_labels(&channels[start_index], TOTAL_SIGNALS_ON_SCREEN);
    draw_time_scale(state);
    draw_page_tabs();
    draw_signals(state, &channels[start_index], TOTAL_SIGNALS_ON_SCREEN);
    draw_trigger_marker(state, trigger_position);
}

// switch to the other page
void switch_ui_page() {
    current_page ^= 1;  // Toggle page (0 or 1)
}

// to show what location the user's channel select is at, draw a line (helper function for interface.c)
void draw_select_line(const Channel* channels, int selected_channel) {
    if (channels == 0)
        return;

    if (selected_channel < 0 || selected_channel >= TOTAL_SIGNALS_ON_SCREEN)
        return;  // deselected / invalid state

    int start_index = current_page * TOTAL_SIGNALS_ON_SCREEN;  // either 0 or 8

    const int lane_height = 27;
    const int underline_x_start = 10;
    const int underline_x_end = left_bar_width - 14;

    int y_top = top_bar_height + selected_channel * lane_height;

    uint16_t color = channels[selected_channel].enabled ? channels[start_index + selected_channel].color
                                                        : dim_color(channels[start_index + selected_channel].color);

    // channel label is roughly centered vertically in the lane,
    // so place underline a bit below that
    int y_underline = y_top + (lane_height / 2) + 6;

    // keep underline inside lane
    if (y_underline >= y_top + lane_height - 1)
        y_underline = y_top + lane_height - 2;

    draw_hline(underline_x_start, underline_x_end, y_underline,
               color);
}

// show the current state of the logic analyzer (play or pause)
void draw_la_status_icon(bool la_is_running) {
    const int size = 3;
    const int padding = 2;

    const int x_right = SCREEN_W - padding - 1;
    const int y_top = 0;

    const int x_left = x_right - size + 1;

    if (la_is_running) {
        // --- PLAY (triangle) ---
        uint16_t color = 0x8e8f;  // matcha green lol
        draw_vline(x_left, y_top + 0, y_top + 2, color);
        draw_vline(x_left + 1, y_top + 0, y_top + 2, color);
        draw_vline(x_left + 2, y_top + 1, y_top + 1, color);

    } else {
        // --- PAUSE (two vertical bars) ---
        uint16_t color = 0x9a29;  // grey red

        // two vertical bars
        int bar1_x = x_left;
        int bar2_x = x_left + 2;

        draw_vline(bar1_x, y_top, y_top + size - 1, color);
        draw_vline(bar2_x, y_top, y_top + size - 1, color);
    }
}

// initalize channels struct (from draw_screen module)
void channels_init(Channel* channels, const int total_signals) {
    for (int i = 0; i < total_signals; i++) {
        channels[i].samples = channel_buffers[i];  // assign a buffer to each channel
        channels[i].count = 0;
        channels[i].enabled = false;
        channels[i].color = channel_colors[i];  // give each signal it's color

        // give each signal it's label
        channels[i].label[0] = 'C';
        channels[i].label[1] = 'H';
        if (i < 10) {
            channels[i].label[2] = '0' + i;
            channels[i].label[3] = '\0';
        } else {
            channels[i].label[2] = '1';
            channels[i].label[3] = '0' + (i - 10);
            channels[i].label[4] = '\0';
        }
    }
}
