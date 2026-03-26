#include <stdbool.h>
#include <stdint.h>

#include "address_map_niosV.h"
#include "interface.h"

/********************************
 *  Structs + global variables
 ********************************/

/********************************
 *  Main Program
 ********************************/
// NOTE: CURRENTLY HARD CODED FOR 16 CHANNELS. DO NOT ENTER MORE. WILL LEAD TO UNDEFINED BEHAVIOUR

int main(void) {
    setup_init();
    while (1) {
        la_start();
        trigger_logic_analyzer();
    }
    return 0;
}
