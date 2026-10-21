#include QMK_KEYBOARD_H

#define MODE_TEXT 0
#define MODE_RAW  1

static uint8_t current_mode = MODE_TEXT;
static char text_line1[32] = "InstroPad Ready";
static char text_line2[32] = "";
static uint8_t raw_frame_buffer[512] = {0};
static bool oled_dirty = true;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_P7, KC_P8, KC_P9, KC_ENTER,
        KC_P4, KC_P5, KC_P6,
        KC_P1, KC_P2, KC_P3
    )
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) }
};
#endif

void raw_hid_receive(uint8_t *data, uint8_t length) {
    uint8_t cmd = data[0];

    if (cmd == 0x01) { 
        current_mode = MODE_TEXT;
        uint8_t line = data[1];
        if (line == 0) {
            memcpy(text_line1, &data[2], 29);
            text_line1[29] = '\0';
        } else {
            memcpy(text_line2, &data[2], 29);
            text_line2[29] = '\0';
        }
        oled_dirty = true;
    } 
    else if (cmd == 0x02) { 
        current_mode = MODE_RAW;
        uint8_t offset = data[1] * 30;
        if (offset + 30 <= 512) {
            memcpy(&raw_frame_buffer[offset], &data[2], 30);
            oled_dirty = true;
        }
    } 
    else if (cmd == 0x03) { 
        oled_clear();
        memset(text_line1, 0, sizeof(text_line1));
        memset(text_line2, 0, sizeof(text_line2));
        memset(raw_frame_buffer, 0, sizeof(raw_frame_buffer));
        oled_dirty = true;
    }
}

#ifdef OLED_ENABLE
bool oled_task_user(void) {
    if (!oled_dirty) {
        return false;
    }

    if (current_mode == MODE_RAW) {
        oled_write_raw((const char *)raw_frame_buffer, sizeof(raw_frame_buffer));
    } else {
        oled_clear();
        oled_set_cursor(0, 0);
        oled_write(text_line1, false);
        oled_set_cursor(0, 2);
        oled_write(text_line2, false);
    }

    oled_dirty = false;
    return false;
}
#endif