#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

#include "address_map_niosV.h"

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