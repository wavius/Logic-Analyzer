
/********************************
 *  Not needed functions?
 *******************************
 #define PS2_RAVAIL_MASK 0xFFFF0000
#define PS2_RAVAIL_SHIFT 16

// control register bits
#define PS2_CTRL_RE_MASK 0x00000001
#define PS2_CTRL_RI_MASK 0x00000100
#define PS2_CTRL_CE_MASK 0x00000400

// common PS/2 protocol bytes
#define PS2_ACK 0xFA
#define PS2_RESET_CMD 0xFF
#define PS2_BAT_PASS 0xAA

 static char* key_table[SCAN_CODE_NUM] = {"A", "B", "C", "D", "E", "F", "G", "H", "I",
                                         "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W",
                                         "X", "Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "`",
                                         "-", "=", "\\", "BKSP", "SPACE", "TAB", "CAPS", "L SHFT", "L CTRL",
                                         "L GUI", "L ALT", "R SHFT", "R CTRL", "R GUI", "R ALT", "APPS",
                                         "ENTER", "ESC", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
                                         "F10", "F11", "F12", "SCROLL", "[", "INSERT", "HOME", "PG UP",
                                         "DELETE", "END", "PG DN", "U ARROW", "L ARROW", "D ARROW", "R ARROW",
                                         "NUM", "KP /", "KP *", "KP -", "KP +", "KP ENTER", "KP .", "KP 0",
                                         "KP 1", "KP 2", "KP 3", "KP 4", "KP 5", "KP 6", "KP 7", "KP 8", "KP 9",
                                         "]", ";", "'", ",", ".", "/"};

bool ps2_keyboard_has_data(const PS2Keyboard* kb) {
    uint32_t data_reg;

    if (kb == 0 || kb->base == 0)
        return false;

    data_reg = kb->base->dataReg;
    return read_data_valid(data_reg) != 0;
}

bool ps2_keyboard_has_data(const PS2Keyboard* kb) {
    uint32_t data_reg = ps2_keyboard_read_data_reg(kb);
    return read_data_valid(data_reg) != 0;
}

bool ps2_keyboard_write_byte(PS2Keyboard* kb, uint8_t byte) {
    uint32_t ctrl_reg;

    if (kb == 0)
        return false;

    // note: data are only located at the lower 8 bits
    // note: the software send command to the PS2 peripheral through the data
    //		register rather than the control register
    IOWR_32DIRECT((uintptr_t)kb->base, PS2_DATA_REG_OFFSET * 4, byte);

    ctrl_reg = ps2_keyboard_read_ctrl_reg(kb);
    if (read_CE_bit(ctrl_reg)) {
        // CE bit is set --> error occurs on sending commands
        return false;
    }

    return true;
}

bool ps2_keyboard_wait_for_ack(PS2Keyboard* kb, uint32_t max_tries) {
    uint32_t tries = 0;
    uint8_t data = 0;

    if (kb == 0)
        return false;

    while (1) {
        if (ps2_keyboard_read_byte(kb, &data)) {
            if (data == PS2_ACK)
                return true;
        }

        if (max_tries != 0) {
            tries++;
            if (tries >= max_tries)
                return false;
        }
    }
}


bool ps2_keyboard_write_byte_with_ack(PS2Keyboard* kb,
                                      uint8_t byte,
                                      uint32_t max_tries) {
    if (!ps2_keyboard_write_byte(kb, byte))
        return false;

    return ps2_keyboard_wait_for_ack(kb, max_tries);
}


bool ps2_keyboard_reset(PS2Keyboard* kb, uint32_t max_tries) {
    uint32_t tries = 0;
    uint8_t data = 0;

    if (kb == NULL)
        return false;

    if (!ps2_keyboard_write_byte_with_ack(kb, PS2_RESET_CMD, max_tries))
        return false;

    while (1) {
        if (ps2_keyboard_read_byte(kb, &data)) {
            if (data == PS2_BAT_PASS)
                return true;
        }

        if (max_tries != 0) {
            tries++;
            if (tries >= max_tries)
                return false; //didn't rest sucessfully
        }
    }
}

void ps2_keyboard_enable_read_interrupt(PS2Keyboard* kb) {
    uint32_t ctrl_reg;

    if (kb == 0)
        return;

    ctrl_reg = ps2_keyboard_read_ctrl_reg(kb);
    ctrl_reg |= PS2_CTRL_RE_MASK;
    IOWR_32DIRECT((uintptr_t)kb->base, PS2_CTRL_REG_OFFSET * 4, ctrl_reg);
}



void ps2_keyboard_disable_read_interrupt(PS2Keyboard* kb) {
    uint32_t ctrl_reg;

    if (kb == 0)
        return;

    ctrl_reg = ps2_keyboard_read_ctrl_reg(kb);
    ctrl_reg &= ~PS2_CTRL_RE_MASK;
    IOWR_32DIRECT((uintptr_t)kb->base, PS2_CTRL_REG_OFFSET * 4, ctrl_reg);
}

bool ps2_keyboard_read_interrupt_pending(const PS2Keyboard* kb) {
    uint32_t ctrl_reg = ps2_keyboard_read_ctrl_reg(kb);
    return (ctrl_reg & PS2_CTRL_RI_MASK) != 0;
}

// extract CE bit from the control register of the ps2 keyboard
bool read_command_error(uint32_t ctrl_reg) {
    return (ctrl_reg & PS2_CTRL_CE_MASK) != 0;
}

*/