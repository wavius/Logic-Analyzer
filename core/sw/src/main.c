#include <stdbool.h>
#include <stdint.h>

#include "address_map_niosV.h"
#include "interface.h"

// NOTE: CURRENTLY HARD CODED FOR 16 CHANNELS. DO NOT ENTER MORE. WILL LEAD TO UNDEFINED BEHAVIOUR
int main(void) {
    put_on_leds(0);
    setup_init();
    put_on_leds(0);

    draw();  // draw inital frame upon start up
    put_on_leds(32);
    while (1) {
        // keyboard_poll_user_input();
        draw();
    }
    return 0;
}
