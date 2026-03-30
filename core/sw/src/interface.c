#include "interface.h"

#include <stdio.h>
#include <string.h>

#include "address_map_niosV.h"
#include "io.h"
#include "vga_driver.h"

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
    text_clear();
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