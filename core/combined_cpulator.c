/* --- START OF sw/inc/constants.h --- */
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define TOTAL_SIGNALS 16 // hard coded to be 16
#define TOTAL_SIGNALS_ON_SCREEN 8 // hard coded to be 8
#define BUFFER_SIZE 4096 // hard coded to be 4096

#endif
/* --- END OF sw/inc/constants.h --- */

/* --- START OF sw/inc/vga_driver.h --- */
#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include <stdint.h>
#include <string.h>  //for memset

// ----- Core VGA ----- //
void vga_init();
void plot_pixel(int x, int y, uint16_t color);
void wait_for_vsync();
void clear_screen();
void text_clear();

// ----- Other ----- //
// int getXres(); //implemented for debugging purposes

#endif
/* --- END OF sw/inc/vga_driver.h --- */

/* --- START OF sw/src/vga_driver.c --- */
// Removed include: sw/src/vga_driver.c

// Removed include: sw/src/vga_driver.c

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
static volatile struct frame_buffer_controller* frameBuf =
    (struct frame_buffer_controller*)0xFF203020;
static volatile int pixel_buffer_start;

// ----- Two frame buffers (double buffering) ----- //
static uint16_t Buffer1[240][512];
static uint16_t Buffer2[240][512];

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

/* --- END OF sw/src/vga_driver.c --- */

/* --- START OF sw/inc/ps2_input.h --- */
#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

// Removed include: sw/inc/ps2_input.h

/********************************
 *  Structs and Data Types
 ********************************/
typedef struct {
    volatile uint32_t data;
    volatile uint32_t control;
} PS2Regs;

// cause I'm picky and the formater is a pain in the ass
// clang-format off
typedef enum {
    KEY_NONE = 0,

    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,

    KEY_TAB, KEY_ESC, KEY_SPACE, KEY_PLUS, KEY_MINUS, 

    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, 
    KEY_5, KEY_6, KEY_7, KEY_8,

    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F,
    KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L,
    KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
    KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X,
    KEY_Y, KEY_Z
} KeyCode;
// clang-format on

typedef struct {
    bool valid;    // true if an event was decoded
    bool pressed;  // true = make code, false = break code
    KeyCode key;   // logical key
} KeyEvent;

typedef struct {
    volatile PS2Regs* regs;
    bool extended_pending;
    bool break_pending;
} Keyboard;

typedef struct {
    bool arrow_up, arrow_down, arrow_left, arrow_right;  // for scrolling
    bool tab, esc, space, plus, minus;                   // for controlling states of UI (i.e. switch pages, zoom in, zoom out)
    bool s, t, c, e;                                     // for controlling states of logic analyzer or UI (i.e. switch pages, start, trigger, clear)
    bool channel[9];                                     // for selecting channels
} KeyPressed;

/********************************
 *  Functions
 ********************************/
void keyboard_init(Keyboard* kb);
bool keyboard_read_event(Keyboard* kb, KeyEvent* ev);

#endif  // PS2_KEYBOARD_H
/* --- END OF sw/inc/ps2_input.h --- */

/* --- START OF sw/src/ps2_input.c --- */
// Removed include: sw/src/ps2_input.c

#include <stddef.h>

// data register bits
#define PS2_DATA_MASK 0x000000FF
#define PS2_RVALID_MASK 0x00008000

/********************************
 *  Helper Functions
 ********************************/
static bool ps2_read_byte(Keyboard* kb, uint8_t* out) {
    if (kb == 0 || kb->regs == 0 || out == 0)
        return false;

    uint32_t value = kb->regs->data;

    if ((value & PS2_RVALID_MASK) == 0)
        return false;

    *out = (uint8_t)(value & PS2_DATA_MASK);
    return true;
}

static void keyboard_clear_fifo(Keyboard* kb) {
    uint8_t dummy;
    while (ps2_read_byte(kb, &dummy)) {
    }
}

// PS/2 Set 2 make codes
static const KeyCode normal_map[256] = {
    // letters
    [0x1C] = KEY_A,
    [0x32] = KEY_B,
    [0x21] = KEY_C,
    [0x23] = KEY_D,
    [0x24] = KEY_E,
    [0x2B] = KEY_F,
    [0x34] = KEY_G,
    [0x33] = KEY_H,
    [0x43] = KEY_I,
    [0x3B] = KEY_J,
    [0x42] = KEY_K,
    [0x4B] = KEY_L,
    [0x3A] = KEY_M,
    [0x31] = KEY_N,
    [0x44] = KEY_O,
    [0x4D] = KEY_P,
    [0x15] = KEY_Q,
    [0x2D] = KEY_R,
    [0x1B] = KEY_S,
    [0x2C] = KEY_T,
    [0x3C] = KEY_U,
    [0x2A] = KEY_V,
    [0x1D] = KEY_W,
    [0x22] = KEY_X,
    [0x35] = KEY_Y,
    [0x1A] = KEY_Z,

    // numbers
    [0x45] = KEY_0,
    [0x16] = KEY_1,
    [0x1E] = KEY_2,
    [0x26] = KEY_3,
    [0x25] = KEY_4,
    [0x2E] = KEY_5,
    [0x36] = KEY_6,
    [0x3D] = KEY_7,
    [0x3E] = KEY_8,

    // controls
    [0x0D] = KEY_TAB,
    [0x76] = KEY_ESC,
    [0x29] = KEY_SPACE,
    [0x55] = KEY_PLUS,  // keyboard +
    [0x4E] = KEY_MINUS  // keyboard -
};

static const KeyCode ext_map[256] = {
    [0x75] = KEY_UP,
    [0x72] = KEY_DOWN,
    [0x6B] = KEY_LEFT,
    [0x74] = KEY_RIGHT};

KeyCode decode_key(uint8_t scancode, bool extended) {
    return extended ? ext_map[scancode] : normal_map[scancode];
}

/********************************
 *  Core Functions
 ********************************/
void keyboard_init(Keyboard* kb) {
    if (kb == 0)
        return;

    kb->regs = (volatile PS2Regs*)0xFF200100;
    kb->extended_pending = false;
    kb->break_pending = false;

    keyboard_clear_fifo(kb);
}

bool keyboard_read_event(Keyboard* kb, KeyEvent* ev) {
    uint8_t byte;
    bool extended;
    KeyCode key;

    if (kb == 0 || ev == 0)
        return false;

    ev->valid = false;
    ev->pressed = false;
    ev->key = KEY_NONE;

    // keep consuming bytes until:
    // 1) successfully decode a full key event
    // 2) the PS/2 FIFO is empty
    while (ps2_read_byte(kb, &byte)) {
        // prefix: extended code
        if (byte == 0xE0) {
            kb->extended_pending = true;
            continue;
        }

        // prefix: break code
        if (byte == 0xF0) {
            kb->break_pending = true;
            continue;
        }

        extended = kb->extended_pending;
        key = decode_key(byte, extended);

        kb->extended_pending = false;

        if (key == KEY_NONE) {
            kb->break_pending = false;
            continue;
        }

        ev->valid = true;
        ev->pressed = !kb->break_pending;
        ev->key = key;

        kb->break_pending = false;
        return true;
    }

    return false;
}
/* --- END OF sw/src/ps2_input.c --- */

/* --- START OF sw/inc/io.h --- */
#ifndef IO_H
#define IO_H

#include <stdint.h>

// -- Functions -- //
void hex_write_char(int hex_index, char c);
void hex_clear_digit(int hex_index);
void hex_clear_all(void);


#endif
/* --- END OF sw/inc/io.h --- */

/* --- START OF sw/src/io.c --- */
// Removed include: sw/src/io.c

// Removed include: sw/src/io.c

/********************************
 *  Helper Functions and stuff
 ********************************/
volatile uint32_t* led_ptr = (volatile uint32_t*)0xFF200000;

// returns active-low 7-seg encoding for the fpga
static uint8_t hex_encode_char(char c) {
    uint8_t seg;
    switch (c) {
        // digits
        case '0':
            seg = 0x3F;
            break;  // abcdef
        case '1':
            seg = 0x06;
            break;  // bc
        case '2':
            seg = 0x5B;
            break;  // abdeg
        case '3':
            seg = 0x4F;
            break;  // abcdg
        case '4':
            seg = 0x66;
            break;  // bcfg
        case '5':
            seg = 0x6D;
            break;  // acdfg
        case '6':
            seg = 0x7D;
            break;  // acdefg
        case '7':
            seg = 0x07;
            break;  // abc
        case '8':
            seg = 0x7F;
            break;  // abcdefg
        case '9':
            seg = 0x6F;
            break;  // abcdfg

        // hex letters
        case 'A':
        case 'a':
            seg = 0x77;
            break;  // abcefg

        case 'B':
        case 'b':
            seg = 0x7C;
            break;  // cdefg

        case 'C':
        case 'c':
            seg = 0x39;
            break;  // adef

        case 'D':
        case 'd':
            seg = 0x5E;
            break;  // bcdeg

        case 'E':
        case 'e':
            seg = 0x79;
            break;  // adefg

        case 'F':
        case 'f':
            seg = 0x71;
            break;  // aefg

        // extra letters that look decent on 7-seg
        case 'H':
        case 'h':
            seg = 0x76;
            break;  // bcefg

        case 'L':
        case 'l':
            seg = 0x38;
            break;  // def

        case 'P':
        case 'p':
            seg = 0x73;
            break;  // abefg

        case 'U':
        case 'u':
            seg = 0x3E;
            break;  // bcdef

        case 'Y':
        case 'y':
            seg = 0x6E;
            break;  // bcdfg

        case '-':
            seg = 0x40;
            break;  // g
        case '_':
            seg = 0x08;
            break;  // d
        case ' ':
            seg = 0x00;
            break;  // blank

        default:
            seg = 0x00;
            break;  // unsupported -> blank
    }
    return seg;
}

/********************************
 * Visible Functions
 ********************************/
// put values on leds
void put_on_leds(uint32_t led_val) {
    *led_ptr = led_val;
}

// write one character to one HEX display
void hex_write_char(int hex_index, char c) {
    if (hex_index < 0 || hex_index > 5)  // options of hex displays to put stuff on is restricted
        return;

    uint8_t seg = hex_encode_char(c);

    if (hex_index <= 3) {
        volatile uint32_t* hex30 = (volatile uint32_t*)0xFF200020;
        uint32_t shift = hex_index * 8;
        uint32_t mask = 0xFFu << shift;

        uint32_t value = *hex30;
        value &= ~mask;
        value |= ((uint32_t)seg << shift);
        *hex30 = value;
    } else {
        volatile uint32_t* hex54 = (volatile uint32_t*)0xFF200030;
        uint32_t shift = (hex_index - 4) * 8;
        uint32_t mask = 0xFFu << shift;

        uint32_t value = *hex54;
        value &= ~mask;
        value |= ((uint32_t)seg << shift);
        *hex54 = value;
    }
}

// clear one HEX display
void hex_clear_digit(int hex_index) {
    hex_write_char(hex_index, ' ');
}

// clear all HEX displays
void hex_clear_all(void) {
    volatile uint32_t* hex30 = (volatile uint32_t*)0xFF200020;
    volatile uint32_t* hex54 = (volatile uint32_t*)0xFF200030;

    *hex30 = 0x7F7F7F7F;
    *hex54 = 0x00007F7F;
}
/* --- END OF sw/src/io.c --- */

/* --- START OF sw/inc/test_la_c.h --- */
#ifndef TEST_LA_C_H
#define TEST_LA_C_H

#include <stdbool.h>
#include <stdint.h>

// Removed include: sw/inc/test_la_c.h

// ---------------------------------------------------------------------------
//  Drop-in replacements for the custom LA hardware module
//  Include this instead of real LA header for CPUlator
//  All signals are synthetically generated in software
// ---------------------------------------------------------------------------

// Call once before la_start() to choose which channel edge triggers capture.
void la_set_trigger_channel(int channel);

// "Arms" the logic analyzer — in test mode this pre-fills the internal buffer
// with generated waveforms immediately.
void la_start(void);

// Always returns true in test mode (capture is instant).
bool la_is_done(void);

// Copies the generated buffer into dst (up to `count` samples).
void la_download_buffer(uint16_t* dst, int count);

// No-ops in test mode — kept so interface.c compiles unchanged.
void la_stop(void);
void la_reset_read_pointer(void);

// Returns the sample index of the first rising edge on the trigger channel.
int la_get_trigger_index(void);

#endif  // TEST_LA_C_H
/* --- END OF sw/inc/test_la_c.h --- */

/* --- START OF sw/src/test_la_c.c --- */
// Removed include: sw/src/test_la_c.c

#include <string.h>

// ---------------------------------------------------------------------------
//  Internal state
// ---------------------------------------------------------------------------
static uint16_t test_buffer[BUFFER_SIZE];  // packed: bit i of sample[s] = channel i
static bool buffer_ready = false;
static int trigger_channel = 0;
static int trigger_index = 0;

// ---------------------------------------------------------------------------
//  Waveform generators
//  Each returns 0 or 1 for a given sample index s.
// ---------------------------------------------------------------------------

// Square wave: period in samples
static int square(int s, int period) {
    return (s % period) < (period / 2) ? 1 : 0;
}

// Single pulse: high for `width` samples starting at `offset`
static int pulse(int s, int offset, int width) {
    return (s >= offset && s < offset + width) ? 1 : 0;
}

// Clock-like burst: active only during [burst_start, burst_end)
static int burst(int s, int period, int burst_start, int burst_end) {
    if (s < burst_start || s >= burst_end) return 0;
    return square(s, period);
}

// Simple PWM: 30 % duty cycle
static int pwm(int s, int period) {
    return (s % period) < (period * 3 / 10) ? 1 : 0;
}

// Alternating 1-0 (fastest possible toggle each sample)
static int toggle(int s) {
    return s & 1;
}

// Always high / always low
static int constant(int val) { return val; }

// ---------------------------------------------------------------------------
//  Build the packed buffer
// ---------------------------------------------------------------------------
static void generate_signals(void) {
    for (int s = 0; s < BUFFER_SIZE; s++) {
        uint16_t packed = 0;

        // CH0  — slow square wave (period 512)
        packed |= (uint16_t)(square(s, 512)) << 0;
        // CH1  — medium square wave (period 128)
        packed |= (uint16_t)(square(s, 128)) << 1;
        // CH2  — fast square wave (period 32)
        packed |= (uint16_t)(square(s, 32)) << 2;
        // CH3  — very fast toggle (period 2)
        packed |= (uint16_t)(toggle(s)) << 3;
        // CH4  — single wide pulse in the middle of the buffer
        packed |= (uint16_t)(pulse(s, 1800, 400)) << 4;
        // CH5  — three narrow pulses
        packed |= (uint16_t)(pulse(s, 200, 20) | pulse(s, 600, 20) | pulse(s, 1000, 20)) << 5;
        // CH6  — PWM 30 % duty, period 64
        packed |= (uint16_t)(pwm(s, 64)) << 6;
        // CH7  — burst: fast clock only in first quarter of buffer
        packed |= (uint16_t)(burst(s, 16, 0, BUFFER_SIZE / 4)) << 7;
        // CH8  — slow square offset by half period (inverted CH0)
        packed |= (uint16_t)(!square(s, 512)) << 8;
        // CH9  — square wave period 256
        packed |= (uint16_t)(square(s, 256)) << 9;
        // CH10 — always high
        packed |= (uint16_t)(constant(1)) << 10;
        // CH11 — always low
        packed |= (uint16_t)(constant(0)) << 11;
        // CH12 — slow PWM 70 % duty
        packed |= (uint16_t)(pwm(s, 256)) << 12;
        // CH13 — medium pulse train (period 200, width 40)
        packed |= (uint16_t)((s % 200) < 40 ? 1 : 0) << 13;
        // CH14 — staircase approximation (steps of 256 samples, cycling 0/1)
        packed |= (uint16_t)((s / 256) & 1) << 14;
        // CH15 — inverted CH2
        packed |= (uint16_t)(!square(s, 32)) << 15;

        test_buffer[s] = packed;
    }
}

// Find the first rising edge on `channel` and store the sample index.
static void compute_trigger_index(void) {
    trigger_index = 0;  // fallback: start of buffer

    for (int s = 1; s < BUFFER_SIZE; s++) {
        int prev = (test_buffer[s - 1] >> trigger_channel) & 1;
        int curr = (test_buffer[s] >> trigger_channel) & 1;
        if (!prev && curr) {  // rising edge
            trigger_index = s;
            return;
        }
    }
}

// ---------------------------------------------------------------------------
//  Visible Functions
// ---------------------------------------------------------------------------
void la_set_trigger_channel(int channel) {
    if (channel >= 0 && channel < 16)
        trigger_channel = channel;
}

void la_start(void) {
    generate_signals();
    compute_trigger_index();
    buffer_ready = true;
}

bool la_is_done(void) {
    return buffer_ready;  // always ready instantly in test mode
}

void la_download_buffer(uint16_t* dst, int count) {
    if (!dst || count <= 0) return;
    if (count > BUFFER_SIZE) count = BUFFER_SIZE;
    memcpy(dst, test_buffer, count * sizeof(uint16_t));
}

void la_stop(void) {
    buffer_ready = false;  // reset so la_is_done() returns false until next la_start()
}

void la_reset_read_pointer(void) {
    // no-op: we use memcpy so there is no read pointer to reset
}

int la_get_trigger_index(void) {
    return trigger_index;
}
/* --- END OF sw/src/test_la_c.c --- */

/* --- START OF sw/inc/visualizer_logic.h --- */
#ifndef VISUALIZER_LOGIC_H
#define VISUALIZER_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

// Removed include: sw/inc/visualizer_logic.h

/********************************
 *  Structs
 ********************************/
typedef struct {
    //-- typically hardcoded values --//
    uint32_t sample_rate;         // sampling frequency
    uint32_t buffer_size;         // buffer size (set to be 4096)
    uint32_t vertical_divisions;  // vertical divisions (set to be 8)

    //-- zoom values --//
    uint32_t time_div;         // time div in ns/div
    uint32_t visible_samples;  // current zoom window
    uint32_t scroll_offset;    // where the signal will start in the sample array (determined by user)
} ZoomState;

/********************************
 *  Functions
 ********************************/
void zoom_state_init(ZoomState* g_state, uint32_t default_visible_samples);
void visualizer_set_zoom(ZoomState* g_state, uint32_t time_div, uint32_t trigger_position);
bool visualizer_zoom_in(ZoomState* g_state, uint32_t trigger_position);
bool visualizer_zoom_out(ZoomState* g_state, uint32_t trigger_position);
void visualizer_scroll_left(ZoomState* g_state);
void visualizer_scroll_right(ZoomState* g_state);
uint32_t visualizer_get_end_sample(const ZoomState* g_state);
void center_view_on_trigger(ZoomState* g_state, uint32_t trigger_position);

#endif
/* --- END OF sw/inc/visualizer_logic.h --- */

/* --- START OF sw/src/visualizer_logic.c --- */
// Removed include: sw/src/visualizer_logic.c

#include <stddef.h>

#define ZOOM_LVL_COUNT 6
#define SAMPLE_RATE 100000000  // hard coded to be 100 MHz
#define VERTICAL_DIVISIONS 8   // hard coded to hold 8

/********************************
 *  Structs + global variables
 ********************************/
static const uint32_t zoom_levels_samples[ZOOM_LVL_COUNT] = {64, 96, 128, 256, 512, 1024};
static uint32_t zoom_levels_time_div[ZOOM_LVL_COUNT];

/********************************
 *  Helper Function Declarations
 ********************************/
static int find_best_zoom_index(uint32_t visible_samples);
static void clamp_scroll_offset(ZoomState* g_state);
static uint32_t clamp_visible_samples(uint32_t visible_samples, uint32_t buffer_size);
uint32_t visualizer_get_scroll_step(const ZoomState* g_state);
static void calc_time_div_zoom_lvls();
uint32_t time_div_to_samples(uint32_t time_div);

/********************************
 *  Helper Functions
 ********************************/
// returns zoom index if found exactly, otherwise nearest valid index
static int find_best_zoom_index(uint32_t visible_samples) {
    int best_index = 0;
    uint32_t best_diff = (zoom_levels_samples[0] > visible_samples) ? (zoom_levels_samples[0] - visible_samples) : (visible_samples - zoom_levels_samples[0]);

    for (int i = 1; i < ZOOM_LVL_COUNT; i++) {
        uint32_t diff = (zoom_levels_samples[i] > visible_samples) ? (zoom_levels_samples[i] - visible_samples) : (visible_samples - zoom_levels_samples[i]);

        if (diff < best_diff) {
            best_diff = diff;
            best_index = i;
        }
    }

    return best_index;  // best index from the zoom levels struct
}

// ensures visible_samples is valid and does not exceed capture size
static uint32_t clamp_visible_samples(uint32_t visible_samples, uint32_t buffer_size) {
    // for a hardcoded 4096 bit buffer, this function does nothing other than call find_best_zoom_index
    if (buffer_size == 0)  // buffer contains no samples (shouldn't ever happen)
        return 0;

    // choose nearest zoom level first
    int idx = find_best_zoom_index(visible_samples);
    uint32_t chosen = zoom_levels_samples[idx];

    // if chosen zoom is larger than capture, shrink to largest valid zoom
    if (chosen > buffer_size) {
        for (int i = ZOOM_LVL_COUNT - 1; i >= 0; i--) {  // short for loop since zoom level count is just 6
            if (zoom_levels_samples[i] <= buffer_size)
                return zoom_levels_samples[i];
        }

        // if capture is smaller than all zoom levels, just show entire capture (but this shouldn't ever happen since our buffer will be huge)
        return buffer_size;
    }

    return chosen;  // if buffer size is larger than all the chosen zoom levels, just return with chosen zoom level as visible samples.
}

// keeps start_sample within valid range
static void clamp_scroll_offset(ZoomState* g_state) {
    if (g_state == 0)  // uninitalized state->return
        return;

    if (g_state->buffer_size == 0) {  // nothing in the buffer
        g_state->scroll_offset = 0;
        return;
    }

    // if zoom level is greater than the buffer size, show the entire waveform by starting at 0th sample (shouldn't happpen
    // with current hard coded values)
    if (g_state->visible_samples >= g_state->buffer_size) {
        g_state->scroll_offset = 0;
        return;
    }

    uint32_t max_start = g_state->buffer_size - g_state->visible_samples;
    if (g_state->scroll_offset > max_start) {  // prevent how far into the buffer we can show the signal
        g_state->scroll_offset = max_start;
    }
}

// calculate time div in nanoseconds/div
static void calc_time_div_zoom_lvls() {
    for (int i = 0; i < ZOOM_LVL_COUNT; i++) {
        // (samples * 1e9) / (8 divisions * 100MHz)
        // = samples * 1000000000 / 800000000
        // = samples * 10 / 8
        // = samples * 5 / 4
        zoom_levels_time_div[i] = zoom_levels_samples[i] * 5 / 4;
    }
}

// go from time/div (in nanoseconds/div) to samples
uint32_t time_div_to_samples(uint32_t time_div) {
    return time_div * 4 / 5;
}

// get the amount each attempt to scroll will shift the scroll offset of the buffer
uint32_t visualizer_get_scroll_step(const ZoomState* g_state) {
    if (g_state == 0 || g_state->vertical_divisions == 0)
        return 1;
    return g_state->visible_samples / (g_state->vertical_divisions * 2);  // integer division is fine here for display / scroll behavior
}

// center the waveform on the trigger helper
void center_view_on_trigger(ZoomState* g_state, uint32_t trigger_position) {
    if (g_state == 0 || g_state->buffer_size == 0)
        return;

    if (g_state->visible_samples >= g_state->buffer_size) {
        g_state->scroll_offset = 0;
        return;
    }

    uint32_t half_window = g_state->visible_samples / 2;

    if (trigger_position <= half_window) {
        g_state->scroll_offset = 0;
        return;
    }

    uint32_t max_start = g_state->buffer_size - g_state->visible_samples;
    uint32_t desired_start = trigger_position - half_window;

    if (desired_start > max_start)
        g_state->scroll_offset = max_start;
    else
        g_state->scroll_offset = desired_start;

    clamp_scroll_offset(g_state);
}

// center the waveform on the trigger
void visualizer_set_zoom(ZoomState* g_state, uint32_t time_div, uint32_t trigger_position) {
    if (g_state == 0)
        return;

    g_state->time_div = time_div;

    uint32_t visible_samples = time_div_to_samples(time_div);
    g_state->visible_samples = clamp_visible_samples(visible_samples, g_state->buffer_size);

    center_view_on_trigger(g_state, trigger_position);
}
/********************************
 *  Function Implementations
 ********************************/
// initazlize the zoom state
void zoom_state_init(ZoomState* g_state, uint32_t default_visible_samples) {
    if (g_state == 0)  // uninitalized state->return
        return;

    calc_time_div_zoom_lvls();  // populate time div array

    g_state->sample_rate = SAMPLE_RATE;
    g_state->buffer_size = BUFFER_SIZE;
    g_state->vertical_divisions = VERTICAL_DIVISIONS;

    // default zoom if possible, otherwise clamp to what fits
    g_state->visible_samples = clamp_visible_samples(default_visible_samples, BUFFER_SIZE);
    g_state->scroll_offset = 0;  // to start, show from the 0th position

    int idx = find_best_zoom_index(g_state->visible_samples);
    g_state->time_div = zoom_levels_time_div[idx];
}

// Zoom in given the user presses a button
bool visualizer_zoom_in(ZoomState* g_state, uint32_t trigger_position) {
    if (g_state == 0 || g_state->buffer_size == 0)
        return false;

    int inf = 1000000;
    int idx = inf;

    for (int i = 0; i < ZOOM_LVL_COUNT; i++) {
        if (zoom_levels_time_div[i] == g_state->time_div)
            idx = i;
    }
    if (idx == inf)  // somehow, the wrong time_div was set, try again
        idx = find_best_zoom_index(g_state->visible_samples);

    // move to smaller window if possible
    if (idx > 0) {
        visualizer_set_zoom(g_state, zoom_levels_time_div[idx - 1], trigger_position);
        return true;
    }
    return false;
}

// Zoom in given the user presses a button
bool visualizer_zoom_out(ZoomState* g_state, uint32_t trigger_position) {
    if (g_state == 0 || g_state->buffer_size == 0)
        return false;

    int inf = -1000000;
    int idx = inf;

    for (int i = 0; i < ZOOM_LVL_COUNT; i++) {
        if (zoom_levels_time_div[i] == g_state->time_div)
            idx = i;
    }
    if (idx == inf)  // somehow, the wrong time_div was set, try again
        idx = find_best_zoom_index(g_state->visible_samples);

    // move to smaller window if possible
    if (idx < ZOOM_LVL_COUNT - 1) {
        visualizer_set_zoom(g_state, zoom_levels_time_div[idx + 1], trigger_position);
        return true;
    }

    return false;
}

// scroll left given the user tries to
void visualizer_scroll_left(ZoomState* g_state) {
    if (g_state == 0)
        return;

    uint32_t step = visualizer_get_scroll_step(g_state);
    if (step > g_state->scroll_offset)  // step size is too large, user scrolled off screen, stop them
        g_state->scroll_offset = 0;
    else
        g_state->scroll_offset -= step;
}

// scroll right given the user tries to
void visualizer_scroll_right(ZoomState* g_state) {
    if (g_state == 0)
        return;

    uint32_t step = visualizer_get_scroll_step(g_state);
    uint32_t max_start = g_state->buffer_size - g_state->visible_samples;

    if (g_state->scroll_offset + step > max_start)  // if the user scrolls too far, clamp them
        g_state->scroll_offset = max_start;
    else
        g_state->scroll_offset += step;
}

// get the last sample to help with plotting
uint32_t visualizer_get_end_sample(const ZoomState* g_state) {
    if (g_state == 0 || g_state->buffer_size == 0)
        return 0;

    uint32_t end = g_state->scroll_offset + g_state->visible_samples;
    return (end < g_state->buffer_size) ? end : g_state->buffer_size;
}
/* --- END OF sw/src/visualizer_logic.c --- */

/* --- START OF sw/inc/draw_screen.h --- */
#ifndef DRAW_SCREEN_H
#define DRAW_SCREEN_H

#include <stdbool.h>
#include <stdint.h>

// Removed include: sw/inc/draw_screen.h
// Removed include: sw/inc/draw_screen.h

// ----- Structs ----- //
// all info needed to draw a singal
typedef struct {
    uint8_t* samples;
    int count;
    // uint16_t color;  // bring back this feature later if I think of a better way to include it
    bool enabled;
    char label[8];
    uint16_t color;
} Channel;

extern uint32_t current_page;  // declaration only

extern uint8_t channel_buffers[TOTAL_SIGNALS][BUFFER_SIZE];

// ----- Logic analyzer UI ----- //
void draw_logic_ui_frame(const Channel* channels, const int lanes);
void draw_signals(const ZoomState* state, const Channel* channels, const int signals_per_page);
void switch_ui_page();
void draw_ui_page(const Channel* channels, const ZoomState* state, uint32_t trigger_position);
void draw_digital_waveform(const uint8_t* samples, const int count, int x0, int y0, int w, int h, uint16_t color);
void channels_init(Channel* channels, const int size);

#endif
/* --- END OF sw/inc/draw_screen.h --- */

/* --- START OF sw/src/draw_screen.c --- */
// Removed include: sw/src/draw_screen.c

// Removed include: sw/src/draw_screen.c
// Removed include: sw/src/draw_screen.c

////////////// temporary include for debugging reasons
// Removed include: sw/src/draw_screen.c
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
void put_on_leds(uint32_t led_val);
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

    volatile char* char_buf = (volatile char*)0x09000000;

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

/* --- END OF sw/src/draw_screen.c --- */

/* --- START OF sw/inc/interface.h --- */
#ifndef INTERFACE_H
#define INTERFACE_H

// Removed include: sw/inc/interface.h
// Removed include: sw/inc/interface.h
// Removed include: sw/inc/interface.h
// Removed include: sw/inc/interface.h
// #include "test_la_c.h"
// Removed include: sw/inc/interface.h

/********************************
 *  Global variables
 ********************************/
extern Keyboard kb;            // keyboard for user input
extern ZoomState g_state;      // handling zooming logic
extern uint32_t current_page;  // defines what the current page is (defined in draw_screen module)

extern Channel channels[TOTAL_SIGNALS];  // all information to draw the signals
extern uint8_t channel_buffers[TOTAL_SIGNALS][BUFFER_SIZE];

/********************************
 *  Functions
 ********************************/
void draw();
void setup_init();
void clear_everything();
void trigger_logic_analyzer();
void keyboard_poll_user_input();

#endif
/* --- END OF sw/inc/interface.h --- */

/* --- START OF sw/src/interface.c --- */
// Removed include: sw/src/interface.c

#include <stdio.h>
#include <string.h>

// Removed include: sw/src/interface.c
// Removed include: sw/src/interface.c
// Removed include: sw/src/interface.c

#define DEFAULT_ZOOM 96

/********************************
 *  Internal global variables
 ********************************/
static uint16_t LA_output[BUFFER_SIZE];
Keyboard kb;        // keyboard for user input
ZoomState g_state;  // handling zooming logic

Channel channels[TOTAL_SIGNALS];  // all information to draw the signals
int key_channel = 0;

bool la_is_running = false;

/********************************
 *  Helper function declarations
 ********************************/
void clear_signals();
int get_current_selected_channel_value();
bool get_signals(bool trigger_running);
void get_signals_test();
void enable_signal(int current_channel);
char int_to_char(int val);

/********************************
 *  Helper Functions
 ********************************/
// turn an int from 0-9 into a char to pass into hex function
char int_to_char(int x) {
    if (x < 0)  // should never trigger but perform the check anyway
        return '0';
    return '0' + x;
}

// select the given channel
void select_channel(int selected) {
    // note: selected channels support [-1, 7] because -1 reflects user scrolling off screen and nothing selected
    if (selected <= -1 || selected >= TOTAL_SIGNALS_ON_SCREEN) {
        key_channel = -1;
        hex_clear_digit(0);  // remove any channel selection indications
        return;              // out of bounds selection
    }
    key_channel = selected;
    hex_write_char(0, int_to_char(key_channel));
}

// increment or decrement selected channel
void increment_channel_selected(int dir) {
    if (dir != -1 && dir != 1)
        return;

    int new_channel = key_channel + dir;

    if (new_channel < -1) {
        key_channel = -1;
        hex_clear_digit(0);
    } else if (new_channel >= TOTAL_SIGNALS_ON_SCREEN) {
        key_channel = TOTAL_SIGNALS_ON_SCREEN;
        hex_clear_digit(0);
    } else {
        key_channel = new_channel;
        hex_write_char(0, int_to_char(key_channel));
    }
}

// figure out which channel the user currently has selected
int get_current_selected_channel_value() {
    // check the user is not in the deselected mode for selecting channels
    // (deselected when current_channel_num == -1 or 8)
    if (key_channel == -1)
        return -1;                                // show deselected state
    uint16_t page_offset = current_page ? 8 : 0;  // current_page is from draw_screen lofic
    return key_channel + page_offset;             // ig 16 signals max, will always be in range [0, 15]
}

// helper function for keyboard_poll_user_input. helps call functions only on rising edge of make code
bool is_new_press(bool previous_state, bool current_pressed) {
    return current_pressed && !previous_state;
}

// a test for plotting signals (DELETE LATER)
void get_signals_test() {
    for (int i = 0; i < TOTAL_SIGNALS; i++) {
        channels[i].count = 4096;
        for (int j = 0; j < BUFFER_SIZE; j++) {
            channels[0].samples[j] = channels[0].samples[j] ^ 1;
        }
        for (int j = 0; j < TOTAL_SIGNALS_ON_SCREEN; j++) {
            channels[j].enabled = true;
            strcpy(channels[i].label, "CH0");
        }
    }
}

// clear signal buffer array
void clear_signals() {
    memset(channel_buffers, 0, sizeof(channel_buffers));  // clear the signal values
    memset(LA_output, 0, sizeof(LA_output));              // clear all other assoicated data
    if (la_is_running) {
        la_stop();  // stop the logic analyzer
        la_is_running = false;
    }
}

// enable signal based on current selected channel
void enable_signal(int current_channel) {
    // check the user is not in the deselected mode for selecting channels
    // (deselected when current_channel_num == -1 or 8)
    if (current_channel == -1 || current_channel >= TOTAL_SIGNALS_ON_SCREEN)
        return;
    if (!(channels[current_channel].enabled))
        channels[current_channel].enabled = true;  // enable if off
    else if (channels[current_channel].enabled)
        channels[current_channel].enabled = false;  // disable if on
}

// once downloaded from LA buffer populate channels samples buffer
bool populate_channels(bool successful_read) {
    if (!successful_read)
        return false;

    // -- polulate the channels to draw on the screen --//
    for (int i = 0; i < TOTAL_SIGNALS; i++) {
        for (int p = 0; p < BUFFER_SIZE; p++) {
            channels[i].samples[p] = (LA_output[p] >> i) & 1;
        }
    }
    return true;
}

/********************************
 *  Functions
 ********************************/
// intitalize peripheral devices and other fundamental logic
void setup_init() {
    // -- intitalize peripheral devices -- //
    vga_init();
    keyboard_init(&kb);
    // add mouse stuff here too if added

    // -- Other initalizations -- //
    zoom_state_init(&g_state, DEFAULT_ZOOM);
    channels_init(channels, TOTAL_SIGNALS);  // all information to DRAW the signals
    hex_write_char(0, int_to_char(key_channel));
}

// recieve all 16 buffers from the logic analyzer
bool get_signals(bool trigger_running) {
    if (!trigger_running || !la_is_running)
        return false;

    bool successful_read = false;

    while (!la_is_done()) {
    }  // get all the signals from the buffer

    la_download_buffer(LA_output, BUFFER_SIZE);

    successful_read = true;

    if (!populate_channels(successful_read)) {
        clear_everything();  // clear and reset signals and LA
        return false;        // something went wrong, don't try to draw
    }

    return true;
}

// wipes everything off the screen, clears the buffer, and stops the logic analyzer
void clear_everything() {
    // -- clear out the buffer -- //
    if (la_is_running) {
        uint16_t throwaway[BUFFER_SIZE];
        la_reset_read_pointer();
        while (!la_is_done()) {
        }  // clear out the buffer
        la_download_buffer(throwaway, BUFFER_SIZE);
        la_reset_read_pointer();  // reset it anyway

        // -- stop logic analyzer -- //
        la_stop();
        la_is_running = false;
    }

    // -- clear out all the signals -- //
    clear_signals();
    channels_init(channels, TOTAL_SIGNALS);  // reset
}

// trigger the logic analyzer and draw it to the screen
void trigger_logic_analyzer() {
    if (!la_is_running)
        return;

    bool trigger_running = false;

    int current_channel_num = get_current_selected_channel_value();

    // check the user is not in the deselected mode for selecting channels
    // (deselected when current_channel_num == -1)
    if (current_channel_num == -1)
        return;
    if (!channels[current_channel_num].enabled)  // not enabled, return
        return;

    la_set_trigger_channel(current_channel_num);
    trigger_running = true;

    get_signals(trigger_running);  // populates channel array to be passed in to draw_screen
    // get_signals_test();
}

// draw to the screen every frame
void draw() {
    clear_screen();
    draw_ui_page(channels, &g_state, la_get_trigger_index());
    wait_for_vsync();
}

// poll for the user's input with keyboard
void keyboard_poll_user_input() {
    KeyEvent ev;
    static KeyPressed key = {0};

    while (keyboard_read_event(&kb, &ev)) {  // drain the entire FIFO (important to prevent lagging! do not change this! )
        switch (ev.key) {                    // figure out what key is currently being pressed/released and call approritate function
            // continuous keys (i.e. holding them will keep triggering their corresponding logic)
            case KEY_LEFT:
                key.arrow_left = ev.pressed;
                break;

            case KEY_RIGHT:
                key.arrow_right = ev.pressed;
                break;

            // one-shot keys (holding won't keep triggering logic. Logic will only be triggered once)
            case KEY_UP:
                if (is_new_press(key.arrow_up, ev.pressed)) {
                    increment_channel_selected(-1);
                }
                key.arrow_up = ev.pressed;
                break;

            case KEY_DOWN:
                if (is_new_press(key.arrow_down, ev.pressed)) {
                    increment_channel_selected(1);
                }
                key.arrow_down = ev.pressed;
                break;

            case KEY_TAB:
                if (is_new_press(key.tab, ev.pressed)) {
                    switch_ui_page();
                }

                key.tab = ev.pressed;
                break;

            case KEY_ESC:
                if (is_new_press(key.esc, ev.pressed))
                    select_channel(-1);
                key.esc = ev.pressed;
                break;

            case KEY_SPACE:
                if (is_new_press(key.space, ev.pressed))
                    enable_signal(get_current_selected_channel_value());
                key.space = ev.pressed;
                break;

            case KEY_PLUS:
                if (is_new_press(key.plus, ev.pressed))
                    visualizer_zoom_in(&g_state, la_get_trigger_index());
                key.plus = ev.pressed;
                break;

            case KEY_MINUS:
                if (is_new_press(key.minus, ev.pressed))
                    visualizer_zoom_out(&g_state, la_get_trigger_index());
                key.minus = ev.pressed;
                break;

            case KEY_S:
                if (is_new_press(key.s, ev.pressed)) {
                    la_start();  // start the logic analyzer
                    la_is_running = true;
                }

                key.s = ev.pressed;
                break;

            case KEY_T:
                if (is_new_press(key.t, ev.pressed)) {
                    trigger_logic_analyzer();
                }
                key.t = ev.pressed;
                break;

            case KEY_C:
                if (is_new_press(key.c, ev.pressed)) {
                    clear_everything();
                }
                key.c = ev.pressed;
                break;

            case KEY_E:
                if (is_new_press(key.e, ev.pressed)) {
                    enable_signal(get_current_selected_channel_value());
                }
                key.e = ev.pressed;
                break;
            case KEY_0:
                if (is_new_press(key.channel[0], ev.pressed)) {
                    select_channel(0);
                }
                key.channel[0] = ev.pressed;
                break;
            case KEY_1:
                if (is_new_press(key.channel[1], ev.pressed)) {
                    select_channel(1);
                }

                key.channel[1] = ev.pressed;
                break;

            case KEY_2:
                if (is_new_press(key.channel[2], ev.pressed))
                    select_channel(2);
                key.channel[2] = ev.pressed;
                break;

            case KEY_3:
                if (is_new_press(key.channel[3], ev.pressed))
                    select_channel(3);
                key.channel[3] = ev.pressed;
                break;
            case KEY_4:
                if (is_new_press(key.channel[4], ev.pressed))
                    select_channel(4);
                key.channel[4] = ev.pressed;
                break;

            case KEY_5:
                if (is_new_press(key.channel[5], ev.pressed))
                    select_channel(5);
                key.channel[5] = ev.pressed;
                break;

            case KEY_6:
                if (is_new_press(key.channel[6], ev.pressed))
                    select_channel(6);
                key.channel[6] = ev.pressed;
                break;

            case KEY_7:
                if (is_new_press(key.channel[7], ev.pressed))
                    select_channel(7);
                key.channel[7] = ev.pressed;
                break;

            default:
                break;  // ignore keys we do not care about
        }
    }

    // continuous actions: run once per poll/frame while held
    if (key.arrow_right && !key.arrow_left) {
        visualizer_scroll_right(&g_state);
    } else if (key.arrow_left && !key.arrow_right) {
        visualizer_scroll_left(&g_state);
    }
}
/* --- END OF sw/src/interface.c --- */

/* --- START OF sw/src/main.c --- */
#include <stdbool.h>
#include <stdint.h>

// Removed include: sw/src/main.c
// Removed include: sw/src/main.c

// NOTE: CURRENTLY HARD CODED FOR 16 CHANNELS. DO NOT ENTER MORE. WILL LEAD TO UNDEFINED BEHAVIOUR
int main(void) {
    setup_init();
    draw();  // draw inital frame upon start up

    while (1) {
        keyboard_poll_user_input();
        draw();
    }
    return 0;
}

/* --- END OF sw/src/main.c --- */
