#include "draw_screen.h"

#include "address_map_niosV.h"
#include "vga_driver.h"

////////////// temporary include for debugging reasons
#include "io.h"
/////////////////////////////////////////////////////

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
    0x72A9, 0x5249, 0x3A89, 0x44CB,
    0x5CFE, 0x65FF, 0x65D7, 0x61ED};

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
static void text_plot_char(int col, int row, char c);
static void text_draw_string(int col, int row, const char* text);
static void draw_channel_labels(const Channel* channels, int lanes);
static uint16_t dim_color(uint16_t color);
static void draw_logic_view(const ZoomState* state, const Channel* channels, int lanes);
static void draw_trigger_marker(const ZoomState* state, uint32_t trigger_position);
static void draw_time_scale(const ZoomState* state);
static const uint8_t* get_glyph_8x8(char ch);
static void draw_char_bitmap(int x, int y, char ch, uint16_t fg, uint16_t bg, bool transparent_bg, int scale);
static void draw_text_bitmap(int x, int y, const char* text, uint16_t fg, uint16_t bg, bool transparent_bg, int scale);
static void draw_uint_bitmap(int x, int y, uint32_t value, uint16_t fg, uint16_t bg, bool transparent_bg, int scale);

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

// draw labels
static void draw_channel_labels(const Channel* channels, const int lanes) {
    if (channels == 0 || lanes <= 0 || lanes > TOTAL_SIGNALS)
        return;

    const int lane_height = 27;
    const int stripe_width = 2;

    const int text_scale = 1;  // smallest readable size
    const int glyph_h = 8 * text_scale;
    const int text_x = stripe_width + 2;

    for (int i = 0; i < lanes; i++) {
        int y_top = top_bar_height + i * lane_height;

        uint16_t stripe_color = channels[i].enabled
                                    ? channels[i].color
                                    : dim_color(channels[i].color);

        uint16_t label_bg = channels[i].enabled
                                ? left_bar_color
                                : dim_color(left_bar_color);

        uint16_t label_fg = channels[i].enabled
                                ? 0xFFFF
                                : dim_color(0xFFFF);

        fill_rect(0, y_top, left_bar_width, lane_height, label_bg);
        fill_rect(0, y_top, stripe_width, lane_height, stripe_color);

        int text_y = y_top + (lane_height - glyph_h) / 2;

        if (channels[i].label[0] != '\0') {
            draw_text_bitmap(
                text_x,
                text_y,
                channels[i].label,
                label_fg,
                label_bg,
                true,  // transparent background so lane bg shows through
                text_scale);
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

// draw the time scale (top bar, x - axis) on the screen to update with changes in zoom or scrolling
static void draw_time_scale(const ZoomState* state) {
    if (state == 0 || state->visible_samples == 0)
        return;

    const int divisions = 8;
    const int text_scale = 1;
    const int glyph_h = 8 * text_scale;
    const int y = top_bar_height - glyph_h - 1;  // near bottom of top bar
    const uint16_t fg = text_color;
    const uint16_t bg = top_bar_color;

    uint32_t samples_per_div = state->visible_samples / divisions;
    if (samples_per_div == 0)
        return;

    uint32_t left_div = state->scroll_offset / samples_per_div;
    uint32_t left_time_ns = left_div * state->time_div;

    for (int i = 0; i <= divisions; i++) {
        int x = left_bar_width + i * grid_spacing_x + 1;
        uint32_t tick_time_ns = left_time_ns + ((uint32_t)i * state->time_div);

        draw_uint_bitmap(x, y, tick_time_ns, fg, bg, true, text_scale);
    }
}

/********************************
 *  Function Implementations
 ********************************/
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
    draw_channel_labels(&channels[start_index], TOTAL_SIGNALS_ON_SCREEN);
    draw_time_scale(state);
    draw_signals(state, &channels[start_index], TOTAL_SIGNALS_ON_SCREEN);
    draw_trigger_marker(state, trigger_position);
}

// switch to the other page
void switch_ui_page() {
    current_page ^= 1;  // Toggle page (0 or 1)
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

/********************************
 *  Text draing helpers
 ********************************/

// each byte = one row, bit 7 is leftmost pixel
static const uint8_t GLYPH_SPACE[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t GLYPH_DASH[8] = {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00};
static const uint8_t GLYPH_COLON[8] = {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00};

// digits
static const uint8_t GLYPH_0[8] = {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_1[8] = {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00};
static const uint8_t GLYPH_2[8] = {0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E, 0x00};
static const uint8_t GLYPH_3[8] = {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_4[8] = {0x0C, 0x1C, 0x3C, 0x6C, 0x7E, 0x0C, 0x0C, 0x00};
static const uint8_t GLYPH_5[8] = {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_6[8] = {0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_7[8] = {0x7E, 0x66, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x00};
static const uint8_t GLYPH_8[8] = {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_9[8] = {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00};

// uppercase letters
static const uint8_t GLYPH_A[8] = {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00};
static const uint8_t GLYPH_B[8] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00};
static const uint8_t GLYPH_C[8] = {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_D[8] = {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00};
static const uint8_t GLYPH_E[8] = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00};
static const uint8_t GLYPH_F[8] = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00};
static const uint8_t GLYPH_G[8] = {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_H[8] = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00};
static const uint8_t GLYPH_I[8] = {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00};
static const uint8_t GLYPH_J[8] = {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00};
static const uint8_t GLYPH_K[8] = {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00};
static const uint8_t GLYPH_L[8] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00};
static const uint8_t GLYPH_M[8] = {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00};
static const uint8_t GLYPH_N[8] = {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00};
static const uint8_t GLYPH_O[8] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_P[8] = {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00};
static const uint8_t GLYPH_Q[8] = {0x3C, 0x66, 0x66, 0x66, 0x6E, 0x3C, 0x0E, 0x00};
static const uint8_t GLYPH_R[8] = {0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x00};
static const uint8_t GLYPH_S[8] = {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_T[8] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00};
static const uint8_t GLYPH_U[8] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_V[8] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00};
static const uint8_t GLYPH_W[8] = {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00};
static const uint8_t GLYPH_X[8] = {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00};
static const uint8_t GLYPH_Y[8] = {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00};
static const uint8_t GLYPH_Z[8] = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00};

static const uint8_t* get_glyph_8x8(char ch) {
    switch (ch) {
        case '0':
            return GLYPH_0;
        case '1':
            return GLYPH_1;
        case '2':
            return GLYPH_2;
        case '3':
            return GLYPH_3;
        case '4':
            return GLYPH_4;
        case '5':
            return GLYPH_5;
        case '6':
            return GLYPH_6;
        case '7':
            return GLYPH_7;
        case '8':
            return GLYPH_8;
        case '9':
            return GLYPH_9;

        case 'A':
            return GLYPH_A;
        case 'B':
            return GLYPH_B;
        case 'C':
            return GLYPH_C;
        case 'D':
            return GLYPH_D;
        case 'E':
            return GLYPH_E;
        case 'F':
            return GLYPH_F;
        case 'G':
            return GLYPH_G;
        case 'H':
            return GLYPH_H;
        case 'I':
            return GLYPH_I;
        case 'J':
            return GLYPH_J;
        case 'K':
            return GLYPH_K;
        case 'L':
            return GLYPH_L;
        case 'M':
            return GLYPH_M;
        case 'N':
            return GLYPH_N;
        case 'O':
            return GLYPH_O;
        case 'P':
            return GLYPH_P;
        case 'Q':
            return GLYPH_Q;
        case 'R':
            return GLYPH_R;
        case 'S':
            return GLYPH_S;
        case 'T':
            return GLYPH_T;
        case 'U':
            return GLYPH_U;
        case 'V':
            return GLYPH_V;
        case 'W':
            return GLYPH_W;
        case 'X':
            return GLYPH_X;
        case 'Y':
            return GLYPH_Y;
        case 'Z':
            return GLYPH_Z;

        case '-':
            return GLYPH_DASH;
        case ':':
            return GLYPH_COLON;
        case ' ':
            return GLYPH_SPACE;
        default:
            return GLYPH_SPACE;
    }
}

static void draw_char_bitmap(int x, int y, char ch, uint16_t fg, uint16_t bg, bool transparent_bg, int scale) {
    if (scale <= 0)
        return;

    if (ch >= 'a' && ch <= 'z')
        ch = ch - 'a' + 'A';

    const uint8_t* glyph = get_glyph_8x8(ch);

    for (int row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];

        for (int col = 0; col < 8; col++) {
            bool pixel_on = (bits & (1 << (7 - col))) != 0;
            int px = x + col * scale;
            int py = y + row * scale;

            if (pixel_on) {
                fill_rect(px, py, scale, scale, fg);
            } else if (!transparent_bg) {
                fill_rect(px, py, scale, scale, bg);
            }
        }
    }
}

static void draw_text_bitmap(int x, int y, const char* text, uint16_t fg, uint16_t bg, bool transparent_bg, int scale) {
    if (text == 0 || scale <= 0)
        return;

    while (*text) {
        if (*text == '\n') {
            y += 8 * scale + scale;
            x = 0;  // optional; adjust if you want multiline anchored differently
        } else {
            draw_char_bitmap(x, y, *text, fg, bg, transparent_bg, scale);
            x += 8 * scale;  // no extra spacing, like char buffer
        }
        text++;
    }
}

static void draw_uint_bitmap(int x, int y, uint32_t value, uint16_t fg, uint16_t bg, bool transparent_bg, int scale) {
    char digits[11];
    int count = 0;

    if (scale <= 0)
        return;

    if (value == 0) {
        draw_char_bitmap(x, y, '0', fg, bg, transparent_bg, scale);
        return;
    }

    while (value > 0 && count < 10) {
        digits[count] = '0' + (value % 10);
        value /= 10;
        count++;
    }

    for (int i = count - 1; i >= 0; i--) {
        draw_char_bitmap(x, y, digits[i], fg, bg, transparent_bg, scale);
        x += 8 * scale;
    }
}
