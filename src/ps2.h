/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/ps2.h
 *
 * Header for PS2 Driver functions and PS2 definitions
 * 
 */

#ifndef PS2_H
#define PS2_H                                   1

#include <stdint.h>

#define PS2_IO_DATA_PORT                        0x60
#define PS2_IO_COMMAND_PORT                     0x64

#define PS2_STATUS_OUTPUT_BUFF                  1<<0
#define PS2_STATUS_INPUT_BUFF                   1<<1
#define PS2_STATUS_SYSTEM_FLAG                  1<<2
#define PS2_STATUS_COMMAND_DATA                 1<<3
#define PS2_STATUS_TIME_OUT                     1<<6
#define PS2_STATUS_PARITY_ERROR                 1<<7

#define PS2_COMMAND_READ_BYTE_ZERO              0x20
#define PS2_COMMAND_WRITE_BYTE_ZERO             0x60
#define PS2_COMMAND_DISSABLE_PORT_TWO           0xA7
#define PS2_COMMAND_ENABLE_PORT_TWO             0xA8
#define PS2_COMMAND_TEST_PORT_TWO               0xA9
#define PS2_COMMAND_TEST_CONTROLLER             0xAA
#define PS2_COMMAND_TEST_PORT_ONE               0xAB
#define PS2_COMMAND_DISSABLE_PORT_ONE           0xAD
#define PS2_COMMAND_ENABLE_PORT_ONE             0xAE
#define PS2_COMMAND_READ_CONTROLLER_IN          0xC0

#define PS2_RESPONSE_SELF_TEST_OK               0x55

#define PS2_KEYBOARD_SET_LED                    0xED
#define PS2_KEYBOARD_ECHO                       0xEE
#define PS2_KEYBOARD_SET_SCANCODE_SET           0xF0
#define PS2_KEYBOARD_ENABLE_SCANNING            0xF4
#define PS2_KEYBOARD_DISABLE_SCANNING           0xF5
#define PS2_KEYBOARD_RESEND                     0xFE
#define PS2_KEYBOARD_RESET                      0xFF

#define PS2_KEYBOARD_ACK                        0xFA
#define PS2_KEYBOARD_SELF_TEST_PASS             0xAA

#define SCAN_TAB                                0x0D
#define SCAN_LEFT_ALT                           0x11
#define SCAN_LEFT_SHIFT                         0x12
#define SCAN_LEFT_CONTROL                       0x14
#define SCAN_CAPS_LOCK                          0x58
#define SCAN_RIGHT_SHIFT                        0x59
#define SCAN_ENTER                              0x5A
#define SCAN_BACKSPACE                          0x66
#define SCAN_ESCAPE                             0x76
#define SCAN_NUM_LOCK                           0x77
#define SCAN_SCROLL_LOCK                        0x7E

#define SCAN_RELEASE                            0xF0
#define SCAN_MULTI_BYTE                         0xE0

struct ps2_configuration {
        uint8_t port1_interrupt:1;
        uint8_t port2_interrupt:1;
        uint8_t system_flag:1;
        uint8_t sbz:1;
        uint8_t port1_clock:1;
        uint8_t port2_clock:1;
        uint8_t port1_translation:1;
        uint8_t mbz:1;
} __attribute__((packed));

int ps2_init(void);
int read_keyboard(void);
char get_char(void);

#endif /* #ifndef PS2_H */
