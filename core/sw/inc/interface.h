#ifndef INTERFACE_H
#define INTERFACE_H

#include "draw_screen.h"
#include "logic_analyzer.h"
#include "ps2_input.h"
#include "visualizer_logic.h"

#define TOTAL_SIGNALS 16      // hard coded to be 16
#define SIGNALS_PER_SCREEN 8  // hard coded to be 8
#define BUFFER_SIZE 4096      // hard coded to be 4096

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