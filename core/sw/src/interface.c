#include "interface.h"

#include <stdio.h>
#include <string.h>

#include "address_map_niosV.h"
#include "vga_driver.h"

#define BUFFER_SIZE 4096
#define TOTAL_SIGNALS 16
#define DEFAULT_ZOOM 96

/********************************
 *  Internal global variables
 ********************************/
static uint16_t LA_output[BUFFER_SIZE];

uint8_t key_channel = 0;

/********************************
 *  Helper function declarations
 ********************************/
void clear_signals();
uint16_t get_current_selected_channel_value();
bool get_signals(bool trigger_running);
void draw();
void get_signals_test();

/********************************
 *  Helper Functions
 ********************************/
// select the given channel
void select_channel(int selected) {
    // note: selected channels support [-1, 8] because -1 and 8 states reflect user scrolling off screen and nothing selected
    if (selected < -1 || selected > SIGNALS_PER_SCREEN)
        return;  // out of bounds selection
    key_channel = selected;
}

// increment or decrement selected channel
void increment_channel_selected(int increment_direction) {
    if (increment_direction != -1 && increment_direction != 1)
        return;  // invalid choice, this function can only add by one or subtract by one

    if (key_channel <= -1 && increment_direction == -1)
        return;  // cannot decrement anymore

    if (key_channel >= SIGNALS_PER_SCREEN && increment_direction == 1)
        return;  // cannot increment anymore

    key_channel += increment_direction;
}

// figure out which channel the user currently has selected
uint16_t get_current_selected_channel_value() {
    uint16_t page_offset = current_page ? 8 : 0;  // current_page is from draw_screen lofic
    return key_channel + page_offset;             // ig 16 signals max, will always be in range [0, 15]
}

// helper function for keyboard_poll_user_input. helps call functions only on rising edge of make code
static inline bool is_new_press(bool previous_state, bool current_pressed) {
    return current_pressed && !previous_state;
}

// a test for plotting signals (DELETE LATER)
void get_signals_test() {
    for (int i = 0; i < TOTAL_SIGNALS; i++) {
        channels[i].count = 4096;
        for (int j = 0; j < BUFFER_SIZE; j++) {
            channels[0].samples[j] = channels[0].samples[j] ^ 1;
        }
        for (int j = 0; j < SIGNALS_PER_SCREEN; j++) {
            channels[j].enabled = true;
            strcpy(channels[i].label, "CH0");
        }
    }
}

// clear signal buffer array
void clear_signals() {
    memset(channel_buffers, 0, sizeof(channel_buffers));  // clear the signal values
    memset(signals, 0, sizeof(signals));                  // clear all other assoicated data
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
    zoom_state_init(&g_state);
    channels_init(channels, TOTAL_SIGNALS);  // all information to DRAW the signals
}

// recieve all 16 buffers from the logic analyzer
bool get_signals(bool trigger_running) {
    if (!trigger_running)
        return false;

    clear_everything();

    while (!la_is_done()) {
    }  // get all the signals from the buffer

    la_download_buffer(signals, BUFFER_SIZE);

    // -- polulate the channels to draw on the screen --//
    for (int i = 0; i < TOTAL_SIGNALS; i++) {
        channels[i].count = BUFFER_SIZE;
        for (int p = 0; p < BUFFER_SIZE; p++) {
            channels[i].samples[p] = (signals[p] >> i) & 1;
        }
        for (int j = 0; j < SIGNALS_PER_SCREEN; j++) {
            channels[j].enabled = true;
            snprintf(channels[i].label, sizeof(channels[i].label), "CH%d", i);
        }
    }
    return true;
}

// wipes everything off the screen, clears the buffer, and stops the logic analyzer
void clear_everything() {
    // -- clear out the buffer -- //
    uint16_t throwaway[BUFFER_SIZE];
    la_reset_read_pointer();
    while (!la_is_done()) {
    }  // clear out the buffer
    la_download_buffer(throwaway, BUFFER_SIZE);
    la_reset_read_pointer();  // reset it anyway

    // -- stop logic analyzer -- //
    la_stop();

    // -- clear out all the signals -- //
    clear_signals();
    channels_init(channels, TOTAL_SIGNALS);  // reset
}

// trigger the logic analyzer and draw it to the screen
void trigger_logic_analyzer() {
    bool trigger_running = false;

    uint16_t current_channel_num = get_current_selected_channel_value();

    la_set_trigger_channel(current_channel_num);
    trigger_running = true;

    get_signals(trigger_running);  // populates channel array to be passed in to draw_screen
    // get_signals_test();

    draw();
}

// draw to the screen every frame
void draw() {
    clear_screen();
    draw_logic_ui_frame(channels, SIGNALS_PER_SCREEN);
    draw_signals(channels, SIGNALS_PER_SCREEN);  // currently, we don't deal with zoom or any of the other complicated logic
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
                    increment_channel_selected(1);
                }
                key.arrow_up = ev.pressed;
                break;

            case KEY_DOWN:
                if (is_new_press(key.arrow_down, ev.pressed)) {
                    increment_channel_selected(-1);
                }
                key.arrow_down = ev.pressed;
                break;

            case KEY_TAB:
                if (is_new_press(key.tab, ev.pressed)) {
                    switch_ui_page();
                    draw_ui_page(channels, &g_state, la_get_trigger_index());
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
                    enable_signal();
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
                if (is_new_press(key.s, ev.pressed))
                    la_start();  // start the logic analyzer
                key.s = ev.pressed;
                break;

            case KEY_T:
                if (is_new_press(key.t, ev.pressed))
                    trigger_logic_analyzer();
                key.t = ev.pressed;
                break;

            case KEY_C:
                if (is_new_press(key.c, ev.pressed)) {
                    clear_everything();
                    draw();
                }
                key.c = ev.pressed;
                break;

            case KEY_E:
                if (is_new_press(key.e, ev.pressed))
                    enable_signal();
                key.e = ev.pressed;
                break;

            case KEY_1:
                if (is_new_press(key.channel[0], ev.pressed))
                    select_channel(0);
                key.channel[0] = ev.pressed;
                break;

            case KEY_2:
                if (is_new_press(key.channel[1], ev.pressed))
                    select_channel(1);
                key.channel[1] = ev.pressed;
                break;

            case KEY_3:
                if (is_new_press(key.channel[2], ev.pressed))
                    select_channel(2);
                key.channel[2] = ev.pressed;
                break;

            case KEY_4:
                if (is_new_press(key.channel[3], ev.pressed))
                    select_channel(3);
                key.channel[3] = ev.pressed;
                break;

            case KEY_5:
                if (is_new_press(key.channel[4], ev.pressed))
                    select_channel(4);
                key.channel[4] = ev.pressed;
                break;

            case KEY_6:
                if (is_new_press(key.channel[5], ev.pressed))
                    select_channel(5);
                key.channel[5] = ev.pressed;
                break;

            case KEY_7:
                if (is_new_press(key.channel[6], ev.pressed))
                    select_channel(6);
                key.channel[6] = ev.pressed;
                break;

            case KEY_8:
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