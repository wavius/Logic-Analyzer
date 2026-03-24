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

typedef enum {
    KEY_NONE = 0,

    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,

    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z
} KeyCode;

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
#define PS2_DATA_MASK 0x000000FF    // first 8 bits of 32 bits
#define PS2_RVALID_MASK 0x00008000  // bit 15

/********************************
 *  Helper Functions
 ********************************/
bool ps2_read_byte(Keyboard* kb, uint8_t* out) {
    if (kb == 0 || kb->regs == 0 || out == 0)  // check for null values
        return false;

    uint32_t value = kb->regs->data;

    if ((value & PS2_RVALID_MASK) == 0)  // check the rvalid bit to see if the data is valid
        return false;

    *out = (uint8_t)(value & PS2_DATA_MASK);  // if the data was valid, read the value and return true
    return true;
}

void keyboard_clear_fifo(Keyboard* kb) {
    uint8_t dummy;
    while (ps2_read_byte(kb, &dummy)) {
    }
}

const KeyCode normal_map[256] = {
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
    [0x1A] = KEY_Z};

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

/* --- START OF core/sw/src/main.c --- */
#include <stdbool.h>
#include <stdint.h>

// Removed include: core/sw/src/main.c
// Removed include: core/sw/src/main.c
// Removed include: core/sw/src/main.c
// Removed include: core/sw/src/main.c
// Removed include: core/sw/src/main.c

#define LED_UP_MASK (1 << 0)
#define LED_DOWN_MASK (1 << 1)
#define LED_LEFT_MASK (1 << 2)
#define LED_RIGHT_MASK (1 << 3)

/********************************
 *  Structs + global variables
 ********************************/

/********************************
 *  Main Program
 ********************************/
// NOTE: CURRENTLY HARD CODED FOR 16 CHANNELS. DO NOT ENTER MORE. WILL LEAD TO UNDEFINED BEHAVIOUR
volatile bool up_down = false;
volatile bool down_down = false;
volatile bool left_down = false;
volatile bool right_down = false;
int main(void) {
    Keyboard kb;
    KeyEvent ev;

    keyboard_init(&kb);
    volatile uint32_t* led_ptr = (volatile uint32_t*)0xFF200000;
    uint32_t led_val = 0;

    *led_ptr = led_val;  // set to 0 at first

    while (1) {
        if (keyboard_read_event(&kb, &ev)) {
            if (ev.key == KEY_UP) {
                up_down = ev.pressed;
            } else if (ev.key == KEY_DOWN) {
                down_down = ev.pressed;
            } else if (ev.key == KEY_LEFT) {
                left_down = ev.pressed;
            } else if (ev.key == KEY_RIGHT) {
                right_down = ev.pressed;
            } else {
                break;
            }
        }
        if (up_down)
            led_val = 0b1;
        else if (down_down)
            led_val = 0b11;
        else if (left_down)
            led_val = 0b11;
        else if (right_down)
            led_val = 0b11;
        else
            led_val = 0;

        *led_ptr = led_val;
    }

    return 0;
}

/* --- END OF core/sw/src/main.c --- */
