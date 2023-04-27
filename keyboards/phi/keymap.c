// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
// #include "quantum.h"
// Debugging
#include "print.h"

// Headers for raw hid communication
#include "raw_hid.h"

// SPI
#include "spi_master.h"

#define PHI_T1_SPI_CS_PIN F5 // First digit chip select
#define PHI_T2_SPI_CS_PIN F6 // Second digit chip select
#define PHI_T3_SPI_CS_PIN F7 // Third digit chip select

// Communication setup for SPI to Exixe
#define PHI_MSBFIRST false // MSB first
#define PHI_SPI_MODE 0     // SPI mode 0
#define PHI_SPI_DIVISOR 2  // 8Mhz clock

#define EXIXE_SPI_BUF_SIZE 16    // 16 Byte messages
#define EXIXE_SPI_HEADER 0xAA    // Byte 0
#define EXIXE_SPI_HEADER_OD 0xAB // Byte 0 for OVERDRIVE

// Tube Refresh
#define EXIXE_REFRESH_TIME 900000 // ms to wait for refresh, 900K ms is 15 min
static uint32_t key_timer;
int requestedVolume = 0;
bool isMute = false;

void refresh_timer(void) {
    key_timer = timer_read32(); // Store time of last refresh
}

// Custom keycodes
enum my_keycodes {
    KC_HIDSEND = SAFE_RANGE // For debugging - send some data over hid to PC
};

// Design for 2x3 macropad + encoder
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* ┌─────────┬─────────┬───────────┐
     * │    -    │   -     │     C     │
     * ├─────────┼─────────┼───────────┤
     * │ KC_MPLY │  COPY   │   MUTE    │
     * ├─────────┼─────────┼───────────┤
     * │  WIN    │ PASTE   │   ATLTAB  │
     * └─────────┴─────────┴───────────┘
     */
    [0] = LAYOUT_ortho_3x3(_______, _______, KC_HIDSEND, KC_MPLY, LCTL(KC_C), KC_MUTE, LWIN(KC_TAB), LCTL(KC_V), LALT(KC_TAB))};

// Unused
void matrix_init_custom(void) {}

// HID input
bool is_hid_connected = false; // Flag indicating if we have a PC connection yet

// // RAW HID Debugging
// void raw_hid_send_debug(void) {
//     // Send some data to connected computer to confirm RAW HID connection
//     uint8_t send_data[32] = {0};
//     send_data[0]          = 2; // Send something
//     raw_hid_send(send_data, sizeof(send_data));
// }

// Switch off selected nixie tube (Exixe)
void exixe_off(pin_t pin) {
    if (spi_start(pin, PHI_MSBFIRST, PHI_SPI_MODE, PHI_SPI_DIVISOR)) {
        // print("Connection Success\n");
        unsigned char spi_buf[EXIXE_SPI_BUF_SIZE];
        memset(spi_buf, 0x80, EXIXE_SPI_BUF_SIZE); // clear SPI buffer
        spi_buf[0] = EXIXE_SPI_HEADER;
        // Send all zero except the first byte
        if (spi_transmit(spi_buf, EXIXE_SPI_BUF_SIZE)) {
            // print("Transmit OK\n");
        };
    };
    spi_stop();
}

// Display requested digit on Exixe
void exixe_send_val(int num, pin_t pin) {
    if (num == 0) { // Zero is the 10th byte
        num = 10;
    } else if (num == -1) { // Left comma is the 11th byte
        num = 11;
    }
    if (spi_start(pin, PHI_MSBFIRST, PHI_SPI_MODE, PHI_SPI_DIVISOR)) {
        // print("Connection Success\n");e
        // uprintf("%d",num);
        uint8_t spi_buf[EXIXE_SPI_BUF_SIZE];
        __builtin_memset(spi_buf, 0x80, EXIXE_SPI_BUF_SIZE); // clear SPI buffer
        spi_buf[0]   = EXIXE_SPI_HEADER;
        spi_buf[num] = 0x80 | 127;
        if (spi_transmit(spi_buf, EXIXE_SPI_BUF_SIZE)) {
            print("Transmit OK\n");
        };
    };
    spi_stop();
}

// Display requested colour on Exixe
void exixe_send_col(int red, int green, int blue, pin_t pin) {
    if (spi_start(pin, PHI_MSBFIRST, PHI_SPI_MODE, PHI_SPI_DIVISOR)) {
        // print("Connection Success\n");e
        // uprintf("%d",num);
        uint8_t spi_buf[EXIXE_SPI_BUF_SIZE];
        __builtin_memset(spi_buf, 0, EXIXE_SPI_BUF_SIZE); // clear SPI buffer
        spi_buf[0]  = EXIXE_SPI_HEADER;
        spi_buf[13] = 0x80 | red;   // red LED full brightness
        spi_buf[14] = 0x80 | green; // green LED off
        spi_buf[15] = 0x80 | blue;  // blue LED full brightness

        if (spi_transmit(spi_buf, EXIXE_SPI_BUF_SIZE)) {
            print("Transmit OK\n");
        };
    };
    spi_stop();
}

// // Exixe Debugging
// void exixe_send_debug(void) {
//     if (spi_start(PHI_T1_SPI_CS_PIN, PHI_MSBFIRST, PHI_SPI_MODE, PHI_SPI_DIVISOR)) {
//         // print("Connection Success\n");e
//         // uprintf("%d",num);
//         uint8_t spi_buf[EXIXE_SPI_BUF_SIZE];
//         __builtin_memset(spi_buf, 0, EXIXE_SPI_BUF_SIZE - 3); // clear SPI buffer
//         spi_buf[0]  = EXIXE_SPI_HEADER;
//         spi_buf[1]  = 0x80 | 127;
//         spi_buf[13] = 0x80 | 127; // red LED full brightness
//         spi_buf[14] = 0x80 | 0;   // green LED off
//         spi_buf[15] = 0x80 | 127; // blue LED full brightness

//         if (spi_transmit(spi_buf, EXIXE_SPI_BUF_SIZE)) {
//             print("Transmit OK\n");
//         };
//     };
//     spi_stop();
//     exixe_send_val(2, PHI_T2_SPI_CS_PIN);
//     exixe_send_val(2, PHI_T3_SPI_CS_PIN);
// }

// Set all colours based on volume
void exixe_set_volume_colour(int volume){
    //colours
    int r = 0;
    int g = 0;
    int b = 0;
    //Set colour - cycle low vol to high vol > blue - green - red
    double ratio = (double) volume / 100.0;
    int sol = ratio * 254;
    if (volume < 51)
    {
        r = 0;
        g = sol;
        b = 127-sol;
    }
    else if (volume < 0)
    {
        r = 0;
        g = 0;
        b = 0;
    }
    else if (volume >= 51)
    {
        r = sol - 127;
        g = 254 - sol;
        b = 0;
    }
    else if (volume > 100)
    {
        r = 0;
        g = 0;
        b = 0;
    }
    //Send Colours
    exixe_send_col(r, g, b, PHI_T1_SPI_CS_PIN);
    exixe_send_col(r, g, b, PHI_T2_SPI_CS_PIN);
    exixe_send_col(r, g, b, PHI_T3_SPI_CS_PIN);
}

// Set the volume
void exixe_set_volume(int volume) {
    //Split the recieved volume into 3 digits
    int i0 = volume;
    int i1 = i0 % 10;
    int i2 = (i0 - i1) / 10;

    //Send the digits
    exixe_send_val(i1, PHI_T3_SPI_CS_PIN);
    exixe_send_val(i2, PHI_T2_SPI_CS_PIN);

    //Send the 100s digit (distinguish between a 100 or a 0)
    if (i0 == 100) {
        exixe_send_val(1, PHI_T1_SPI_CS_PIN);
    } else {
        exixe_send_val(-1, PHI_T1_SPI_CS_PIN);
    }

    exixe_set_volume_colour(i0);

}

//Set Mute
void exixe_set_mute(void) {
    //Cycle the tubes
    exixe_off(PHI_T3_SPI_CS_PIN);
    exixe_off(PHI_T2_SPI_CS_PIN);
    exixe_off(PHI_T1_SPI_CS_PIN);
    //Muted
    exixe_send_val(-1, PHI_T3_SPI_CS_PIN);
    exixe_send_val(-1, PHI_T2_SPI_CS_PIN);
    exixe_send_val(-1, PHI_T1_SPI_CS_PIN);
    //Send Colours
    exixe_send_col(10, 10, 10, PHI_T1_SPI_CS_PIN);
    exixe_send_col(10, 10, 10, PHI_T2_SPI_CS_PIN);
    exixe_send_col(10, 10, 10, PHI_T3_SPI_CS_PIN);
}

// Tube Refreshing
void exixe_refresh_tube(void) {
    // Cycle all digits on each tube individually
    int waitTime = 50;
    for (int i = 1; i < 12;i++)
    {
        exixe_send_val(i, PHI_T3_SPI_CS_PIN);
        wait_ms(waitTime);
    }
    if (is_hid_connected){
        if (!isMute)
            exixe_set_volume(requestedVolume);
        else
            exixe_set_mute();
    }
    for (int i = 1; i < 12;i++)
    {
        exixe_send_val(12-i, PHI_T2_SPI_CS_PIN);
        wait_ms(waitTime);
    }    
    exixe_send_val(11, PHI_T2_SPI_CS_PIN);
    if (is_hid_connected){
        if (!isMute)
            exixe_set_volume(requestedVolume);
        else
            exixe_set_mute();
    }
    for (int i = 1; i < 12;i++)
    {
        exixe_send_val(i, PHI_T1_SPI_CS_PIN);
        wait_ms(waitTime);
    }    
    if (is_hid_connected){
        if (!isMute)
            exixe_set_volume(requestedVolume);
        else
            exixe_set_mute();
    }
}

// Raw HID Receive always listens (i think)
void raw_hid_receive(uint8_t *data, uint8_t length) {
    // PC connected, so set the flag to show a message on the master display
    is_hid_connected = true;
    print("Receiving...\n");
    // Echo the data received
    raw_hid_send(data, length);

    requestedVolume = data[2];
    if (data[3] == 1)
        isMute = true;
    else
        isMute = false;

    if (!isMute) { // Recieved a volume
        exixe_set_volume(requestedVolume);
    } else {
        exixe_set_mute();
    }
}

// DEBUG
void keyboard_post_init_user(void) {
    //debug_enable = true;
    //debug_matrix = true;

    refresh_timer();

    spi_init();             // Initialise SPI

    // Zero out all tubes
    wait_ms(50);
    exixe_off(PHI_T3_SPI_CS_PIN);
    wait_ms(50);
    exixe_off(PHI_T2_SPI_CS_PIN);
    wait_ms(50);
    exixe_off(PHI_T1_SPI_CS_PIN);
    wait_ms(50);
    exixe_refresh_tube();
    wait_ms(50);
    exixe_send_val(-1, PHI_T3_SPI_CS_PIN);
    wait_ms(50);
    exixe_send_val(-1, PHI_T2_SPI_CS_PIN);
    wait_ms(50);
    exixe_send_val(-1, PHI_T1_SPI_CS_PIN);
    wait_ms(50);
    exixe_send_col(50, 50, 50, PHI_T1_SPI_CS_PIN);
    wait_ms(50);
    exixe_send_col(50, 50, 50, PHI_T2_SPI_CS_PIN);
    wait_ms(50);
    exixe_send_col(50, 50, 50, PHI_T3_SPI_CS_PIN);
}

// Runs every tick
void housekeeping_task_user(void) {
    // Check timer to see if tubes need to be refreshed or not
    if (timer_elapsed32(key_timer) > EXIXE_REFRESH_TIME)
    {
        exixe_refresh_tube();
        refresh_timer();
    }
}

// Unused
void matrix_scan_user(void) {}

// Encoder
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        if (clockwise) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
    }
    return true;
}

// Custom Keycode operation
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        // Refresh the tubes
        case KC_HIDSEND:
            if (record->event.pressed) {
                exixe_refresh_tube();
            }
            return false;
        default:
            return true;
    }
}
