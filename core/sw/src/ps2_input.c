#include "ps2_input.h"

#include <stddef.h>

// data register bits
#define PS2_DATA_MASK 0x000000FF    // first 8 bits of 32 bits
#define PS2_RVALID_MASK 0x00008000  // bit 15

/********************************
 *  Helper Functions
 ********************************/
static bool ps2_read_byte(Keyboard* kb, uint8_t* out) {
    if (kb == 0 || kb->regs == 0 || out == 0)  // check for null values
        return false;

    uint32_t value = kb->regs->data;

    if ((value & PS2_RVALID_MASK) == 0)  // check the rvalid bit to see if the data is valid
        return false;

    *out = (uint8_t)(value & PS2_DATA_MASK);  // if the data was valid, read the value and return true
    return true;
}

static void keyboard_clear_fifo(Keyboard* kb) {
    uint8_t dummy;
    while (ps2_read_byte(kb, &dummy)) {
    }
}

static const KeyCode normal_map[256] = {
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

static const KeyCode ext_map[256] = {
    [0x75] = KEY_UP,
    [0x72] = KEY_DOWN,
    [0x6B] = KEY_LEFT,
    [0x74] = KEY_RIGHT};

static inline KeyCode decode_key(uint8_t scancode, bool extended) {
    return extended ? ext_map[scancode] : normal_map[scancode];
}

/********************************
 *  Core Functions
 ********************************/
void keyboard_init(Keyboard* kb) {
    if (kb == 0)
        return;

    kb->regs = (volatile PS2Regs*)PS2_BASE;
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