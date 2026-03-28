#include <stdbool.h>
#include <stdint.h>

#include "address_map_niosV.h"
#include "interface.h"

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
