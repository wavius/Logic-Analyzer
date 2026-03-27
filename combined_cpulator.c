/* --- START OF core/sw/inc/vga_driver.h --- */
#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

#include <stdint.h>
#include <string.h>  //for memset

// ----- Core VGA ----- //
void vga_init();
void plot_pixel(int x, int y, uint16_t color);
void wait_for_vsync();
void clear_screen();

// ----- Other ----- //
// int getXres(); //implemented for debugging purposes

#endif
/* --- END OF core/sw/inc/vga_driver.h --- */

/* --- START OF core/sw/src/vga_driver.c --- */
// Removed include: core/sw/src/vga_driver.c

// Removed include: core/sw/src/vga_driver.c

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

/* --- END OF core/sw/src/vga_driver.c --- */

/* --- START OF core/sw/inc/ps2_input.h --- */
#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

// Removed include: core/sw/inc/ps2_input.h

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

    KEY_1, KEY_2, KEY_3, KEY_4, 
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
    bool channel[8];                                     // for selecting channels
} KeyPressed;

/********************************
 *  Functions
 ********************************/
void keyboard_init(Keyboard* kb);
bool keyboard_read_event(Keyboard* kb, KeyEvent* ev);

#endif  // PS2_KEYBOARD_H
/* --- END OF core/sw/inc/ps2_input.h --- */

/* --- START OF core/sw/src/ps2_input.c --- */
// Removed include: core/sw/src/ps2_input.c

#include <stddef.h>

// data register bits
#define PS2_DATA_MASK 0x000000FF
#define PS2_RVALID_MASK 0x00008000

/********************************
 *  Helper Functions
 ********************************/
bool ps2_read_byte(Keyboard* kb, uint8_t* out) {
    if (kb == 0 || kb->regs == 0 || out == 0)
        return false;

    uint32_t value = kb->regs->data;

    if ((value & PS2_RVALID_MASK) == 0)
        return false;

    *out = (uint8_t)(value & PS2_DATA_MASK);
    return true;
}

void keyboard_clear_fifo(Keyboard* kb) {
    uint8_t dummy;
    while (ps2_read_byte(kb, &dummy)) {
    }
}

// PS/2 Set 2 make codes
const KeyCode normal_map[256] = {
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
    [0x79] = KEY_PLUS,  // keypad +
    [0x7B] = KEY_MINUS  // keypad -
};

const KeyCode ext_map[256] = {
    [0x75] = KEY_UP,
    [0x72] = KEY_DOWN,
    [0x6B] = KEY_LEFT,
    [0x74] = KEY_RIGHT};

inline KeyCode decode_key(uint8_t scancode, bool extended) {
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
/* --- END OF core/sw/src/ps2_input.c --- */

/* --- START OF core/sw/inc/test_la_c.h --- */
#ifndef TEST_LA_C_H
#define TEST_LA_C_H

#include <stdbool.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
//  Drop-in replacements for the custom LA hardware module
//  Include this instead of real LA header for CPUlator
//  All signals are synthetically generated in software
// ---------------------------------------------------------------------------

#define BUFFER_SIZE 4096

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
/* --- END OF core/sw/inc/test_la_c.h --- */

/* --- START OF core/sw/src/test_la_c.c --- */
// Removed include: core/sw/src/test_la_c.c

#include <string.h>

// ---------------------------------------------------------------------------
//  Internal state
// ---------------------------------------------------------------------------
uint16_t test_buffer[BUFFER_SIZE];  // packed: bit i of sample[s] = channel i
bool buffer_ready = false;
int trigger_channel = 0;
int trigger_index = 0;

// ---------------------------------------------------------------------------
//  Waveform generators
//  Each returns 0 or 1 for a given sample index s.
// ---------------------------------------------------------------------------

// Square wave: period in samples
int square(int s, int period) {
    return (s % period) < (period / 2) ? 1 : 0;
}

// Single pulse: high for `width` samples starting at `offset`
int pulse(int s, int offset, int width) {
    return (s >= offset && s < offset + width) ? 1 : 0;
}

// Clock-like burst: active only during [burst_start, burst_end)
int burst(int s, int period, int burst_start, int burst_end) {
    if (s < burst_start || s >= burst_end) return 0;
    return square(s, period);
}

// Simple PWM: 30 % duty cycle
int pwm(int s, int period) {
    return (s % period) < (period * 3 / 10) ? 1 : 0;
}

// Alternating 1-0 (fastest possible toggle each sample)
int toggle(int s) {
    return s & 1;
}

// Always high / always low
int constant(int val) { return val; }

// ---------------------------------------------------------------------------
//  Build the packed buffer
// ---------------------------------------------------------------------------
void generate_signals(void) {
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
void compute_trigger_index(void) {
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
/* --- END OF core/sw/src/test_la_c.c --- */

/* --- START OF core/sw/inc/visualizer_logic.h --- */
#ifndef VISUALIZER_LOGIC_H
#define VISUALIZER_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

#define BUFFER_SIZE 4096       // hard coded to be 4096
#define SAMPLE_RATE 100000000  // hard coded to be 100 MHz
#define VERTICAL_DIVISIONS 8   // hard coded to hold 8

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
/* --- END OF core/sw/inc/visualizer_logic.h --- */

/* --- START OF core/sw/src/visualizer_logic.c --- */
// Removed include: core/sw/src/visualizer_logic.c

#include <stddef.h>

#define ZOOM_LVL_COUNT 6

/********************************
 *  Structs + global variables
 ********************************/
const uint32_t zoom_levels_samples[ZOOM_LVL_COUNT] = {64, 96, 128, 256, 512, 1024};
uint32_t zoom_levels_time_div[ZOOM_LVL_COUNT];

/********************************
 *  Helper Function Declarations
 ********************************/
int find_best_zoom_index(uint32_t visible_samples);
void clamp_scroll_offset(ZoomState* g_state);
uint32_t clamp_visible_samples(uint32_t visible_samples, uint32_t buffer_size);
uint32_t visualizer_get_scroll_step(const ZoomState* g_state);
void calc_time_div_zoom_lvls();
uint32_t time_div_to_samples(uint32_t time_div);

/********************************
 *  Helper Functions
 ********************************/
// returns zoom index if found exactly, otherwise nearest valid index
int find_best_zoom_index(uint32_t visible_samples) {
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
uint32_t clamp_visible_samples(uint32_t visible_samples, uint32_t buffer_size) {
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
void clamp_scroll_offset(ZoomState* g_state) {
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
void calc_time_div_zoom_lvls() {
    for (int i = 0; i < ZOOM_LVL_COUNT; i++) {
        zoom_levels_time_div[i] =
            ((uint64_t)zoom_levels_samples[i] * 1000000000ULL) /
            ((uint64_t)VERTICAL_DIVISIONS * SAMPLE_RATE);
    }
}

// go from time/div (in nanoseconds/div) to samples
uint32_t time_div_to_samples(uint32_t time_div) {
    return (uint32_t)(((uint64_t)time_div * SAMPLE_RATE * VERTICAL_DIVISIONS) / 1000000000ULL);
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
/* --- END OF core/sw/src/visualizer_logic.c --- */

/* --- START OF core/sw/inc/draw_screen.h --- */
#ifndef DRAW_SCREEN_H
#define DRAW_SCREEN_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>  // for snprintf

// Removed include: core/sw/inc/draw_screen.h

#define TOTAL_SIGNALS 16
#define BUFFER_SIZE 4096

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
/* --- END OF core/sw/inc/draw_screen.h --- */

/* --- START OF core/sw/src/draw_screen.c --- */
// Removed include: core/sw/src/draw_screen.c

// Removed include: core/sw/src/draw_screen.c
// Removed include: core/sw/src/draw_screen.c

// ----- Screen constants ----- //
#define SCREEN_W 320
#define SCREEN_H 240
#define TOTAL_SIGNALS_ON_SCREEN 8

// ----- Char buffer ----- //
#define CHAR_COLS 80
#define CHAR_ROWS 60

/********************************
 *  Structs + global variables
 ********************************/
//-- screen sizing variables --//
const int top_bar_height = 15;
const int left_bar_width = 32;  // cause there are 8 major vertical grid ticks. (320 - 32)/8 = 36
const int bottom_bar_height = 7;
const int channel_area_height = SCREEN_H - (top_bar_height + bottom_bar_height);

//-- waveform / grid layout variables --//
const int grid_spacing_x = (SCREEN_W - left_bar_width) / 8;
const int waveform_margin_divisor = 4;
const int waveform_min_margin = 1;

//-- UI color variables --//
const uint16_t top_bar_color = 0x18e3;
const uint16_t bottom_bar_color = 0x18e3;
const uint16_t left_bar_color = 0x18e3;
const uint16_t separator_color = 0x39c7;
const uint16_t grid_color = 0x18e3;
const uint16_t text_color = 0xd69a;

// color options for the signals drawn to screen
const uint16_t channel_colors[16] = {
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
void draw_hline(int x_start, int x_end, int y, uint16_t color);
void draw_vline(int x, int y_start, int y_end, uint16_t color);
void fill_rect(int x_cord, int y_cord, int w, int h, uint16_t color);
int calculate_channel_height(const int lanes, const int available_height);
void text_plot_char(int col, int row, char c);
void text_draw_string(int col, int row, const char* text);
void text_clear(void);
void draw_channel_labels(const Channel* channels, int lanes);
uint16_t dim_color(uint16_t color);
void draw_logic_view(const ZoomState* state, const Channel* channels, int lanes);
void draw_trigger_marker(const ZoomState* state, uint32_t trigger_position);

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

// store a single char in a string buffer
void text_plot_char(int col, int row, char c) {
    if (col < 0 || col >= CHAR_COLS || row < 0 || row >= CHAR_ROWS)
        return;

    // pointer to controller
    volatile int* ctrl = (int*)0xFF203030;

    // first register = character buffer address
    volatile char* char_buf = (volatile char*)(ctrl[0]);

    char_buf[row * CHAR_COLS + col] = c;
}

// draw text to screen
void text_draw_string(int col, int row, const char* text) {
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
void text_clear(void) {
    volatile int* ctrl = (int*)0xFF203030;
    volatile char* char_buf = (volatile char*)(ctrl[0]);

    for (int row = 0; row < CHAR_ROWS; row++) {
        for (int col = 0; col < CHAR_COLS; col++) {
            char_buf[row * CHAR_COLS + col] = ' ';
        }
    }
}

// for dimming the color of text drawn to screen (used for channel labels for example)
uint16_t dim_color(uint16_t color) {
    uint16_t r = (color >> 11) & 0x1F;
    uint16_t g = (color >> 5) & 0x3F;
    uint16_t b = color & 0x1F;

    r >>= 1;
    g >>= 1;
    b >>= 1;

    return (r << 11) | (g << 5) | b;
}

// draws labels
void draw_channel_labels(const Channel* channels, const int lanes) {
    if (channels == 0 || lanes <= 0 || lanes > TOTAL_SIGNALS)
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
                                    ? channels[i].color
                                    : dim_color(channels[i].color);

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

// handle zooming logic by determining the sample window for each enabled channel and prints it out using draw_digital_waveform(...)
void draw_logic_view(const ZoomState* state, const Channel* channels, int signals_per_page) {
    uint32_t start = state->scroll_offset;
    uint32_t end = visualizer_get_end_sample(state);

    if (end <= start)
        return;

    uint32_t visible_count = end - start;

    int lane_height = calculate_channel_height(signals_per_page, channel_area_height);
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
void draw_trigger_marker(const ZoomState* state, uint32_t trigger_position) {
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
void draw_signals(const ZoomState* state, const Channel* channels, const int signals_per_page) {
    if (state == 0 || channels == 0 || signals_per_page != TOTAL_SIGNALS_ON_SCREEN)
        return;
    draw_logic_view(state, channels, signals_per_page);
}

// draw given page
void draw_ui_page(const Channel* channels, const ZoomState* state, uint32_t trigger_position) {
    int start_index = current_page * TOTAL_SIGNALS_ON_SCREEN;  // either 0 or 8
    draw_logic_ui_frame(&channels[start_index], TOTAL_SIGNALS_ON_SCREEN);
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
        snprintf(channels[i].label, sizeof(channels[i].label), "CH%d", i);  // give each channel it's appropriate label
        channels[i].color = channel_colors[i];                              // give each signal it's color
    }
}

/* --- END OF core/sw/src/draw_screen.c --- */

/* --- START OF core/sw/src/main.c --- */
#include <stdbool.h>
#include <stdint.h>

// Removed include: core/sw/src/main.c

// NOTE: CURRENTLY HARD CODED FOR 16 CHANNELS. DO NOT ENTER MORE. WILL LEAD TO UNDEFINED BEHAVIOUR
int main(void) {
    setup_init();
    clear_everything();
    draw();  // draw inital frame upon start up
    while (1) {
        keyboard_poll_user_input();
        draw();
    }
    return 0;
}

/* --- END OF core/sw/src/main.c --- */
