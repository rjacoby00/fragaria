/* 
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/ps2.c
 *
 * 8042 PS2 Controller Driver
 * 
 */

#include "port_io.h"
#include "ps2.h"

/* TODO Make timeouts for all polling in this file */

uint8_t lshift = 0;
uint8_t rshift = 0;
uint8_t caps_lock = 0;
uint8_t scroll_lock = 0;
uint8_t num_lock = 1;

const char scanmap[] = {
             /* 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07
              * 0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F */
/* 0x00 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\t', '`',  '\0',
/* 0x10 */      '\0', '\0', '\0', '\0', '\0', 'q',  '1',  '\0',
                '\0', '\0', 'z',  's',  'a',  'w',  '2',  '\0',
/* 0x20 */      '\0', 'c',  'x',  'd',  'e',  '4',  '3',  '\0',
                '\0', ' ',  'v',  'f',  't',  'r',  '5',  '\0',
/* 0x30 */      '\0', 'n',  'b',  'h',  'g',  'y',  '6',  '\0',
                '\0', '\0', 'm',  'j',  'u',  '7',  '8',  '\0',
/* 0x40 */      '\0', ',',  'k',  'i',  'o',  '0',  '9',  '\0',
                '\0', '.',  '/',  'l',  ';',  'p',  '-',  '\0',
/* 0x50 */      '\0', '\0', '\'', '\0', '[',  '=',  '\0', '\0',
                '\0', '\0', '\0', ']',  '\0', '\\', '\0', '\0',
/* 0x60 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '1',  '\0', '4',  '7',  '\0', '\0', '\0',
/* 0x70 */      '0',  '.',  '2',  '5',  '6',  '8',  '\0', '\0',
                '\0', '+',  '3',  '-',  '*',  '9',  '\0', '\0',
/* 0x80 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0x90 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xA0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xB0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xC0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xD0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xE0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xF0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
};

const char scanmap_shift[] = {
             /* 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07
              * 0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F */
/* 0x00 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\t', '~',  '\0',
/* 0x10 */      '\0', '\0', '\0', '\0', '\0', 'Q',  '!',  '\0',
                '\0', '\0', 'Z',  'S',  'A',  'W',  '@',  '\0',
/* 0x20 */      '\0', 'C',  'X',  'D',  'E',  '$',  '#',  '\0',
                '\0', ' ',  'V',  'F',  'T',  'R',  '%',  '\0',
/* 0x30 */      '\0', 'N',  'B',  'H',  'G',  'Y',  '^',  '\0',
                '\0', '\0', 'M',  'J',  'U',  '&',  '*',  '\0',
/* 0x40 */      '\0', '<',  'K',  'I',  'O',  ')',  '(',  '\0',
                '\0', '>',  '/',  'L',  ';',  'P',  '-',  '\0',
/* 0x50 */      '\0', '\0', '"',  '\0', '{',  '+',  '\0', '\0',
                '\0', '\0', '\0', '}',  '\0', '|',  '\0', '\0',
/* 0x60 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '1',  '\0', '4',  '7',  '\0', '\0', '\0',
/* 0x70 */      '0',  '.',  '2',  '5',  '6',  '8',  '\0', '\0',
                '\0', '+',  '3',  '-',  '*',  '9',  '\0', '\0',
/* 0x80 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0x90 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xA0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xB0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xC0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xD0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xE0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
/* 0xF0 */      '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
};

static int write_keyboard(uint8_t command)
{
        while(PS2_STATUS_INPUT_BUFF & inb(PS2_IO_COMMAND_PORT));
        outb(PS2_IO_DATA_PORT, command);

        return 0;
}

int read_keyboard()
{
        while(!(PS2_STATUS_OUTPUT_BUFF & inb(PS2_IO_COMMAND_PORT)));

        return inb(PS2_IO_DATA_PORT);
}

char get_char() 
{
        int character = read_keyboard();

        switch (character) { 
        case SCAN_TAB:
                        return '\0';
        case SCAN_LEFT_SHIFT:
                        lshift = 1;

                        return '\0';
        case SCAN_CAPS_LOCK:
                        if (caps_lock)
                                caps_lock = 0;
                        else
                                caps_lock = 1;

                        return '\0';
        case SCAN_RIGHT_SHIFT:
                        rshift = 1;

                        return '\0';
        case SCAN_ENTER:
                        return '\n';
        case SCAN_BACKSPACE:
                        return '\0';
        case SCAN_ESCAPE:
                        return '\0';
        case SCAN_NUM_LOCK:
                        return '\0';
        case SCAN_SCROLL_LOCK:
                        return '\0';
        case SCAN_RELEASE:
                        character = read_keyboard();
                        switch (character) {
                        case SCAN_LEFT_SHIFT:
                                lshift = 0;

                                return '\0';
                        case SCAN_RIGHT_SHIFT:
                                rshift = 0;

                                return '\0';
                        }

                        return '\0';
        case SCAN_MULTI_BYTE:
                        character = read_keyboard();

                        if (character == SCAN_RELEASE)
                                character = read_keyboard();

                        return '\0';
        default:
                        if ((lshift | rshift) ^ caps_lock)
                                return scanmap_shift[character];
                        else
                                return scanmap[character];
        }

        return '\0';
}

int ps2_init()
{
        union ps2_config {
                struct ps2_configuration running_config;
                uint8_t config;
        } ps2_config;

        /* Dissable both PS2 ports */
        outb(PS2_IO_COMMAND_PORT, PS2_COMMAND_DISSABLE_PORT_ONE);
        outb(PS2_IO_COMMAND_PORT, PS2_COMMAND_DISSABLE_PORT_TWO);

        /* Flush controller output buffer */
        inb(PS2_IO_DATA_PORT);

        /* Get the running config */
        outb(PS2_IO_COMMAND_PORT, PS2_COMMAND_READ_BYTE_ZERO);
        while (!(PS2_STATUS_OUTPUT_BUFF & inb(PS2_IO_COMMAND_PORT)));
        ps2_config.config = inb(PS2_IO_DATA_PORT);

        /* Dissable interrupts and translation */
        ps2_config.running_config.port1_interrupt = 0;
        ps2_config.running_config.port2_interrupt = 0;
        ps2_config.running_config.port1_translation = 0;

        /* Do controller self test */
        outb(PS2_IO_COMMAND_PORT, PS2_COMMAND_TEST_CONTROLLER);
        while(!(PS2_STATUS_OUTPUT_BUFF & inb(PS2_IO_COMMAND_PORT)));
        if (inb(PS2_IO_DATA_PORT) != PS2_RESPONSE_SELF_TEST_OK) 
                return 1;

        /* Write new config */
        outb(PS2_IO_COMMAND_PORT, PS2_COMMAND_WRITE_BYTE_ZERO);
        while (PS2_STATUS_INPUT_BUFF & inb(PS2_IO_COMMAND_PORT));
        outb(PS2_IO_DATA_PORT, ps2_config.config);

        /* Enable port one */
        outb(PS2_IO_COMMAND_PORT, PS2_COMMAND_ENABLE_PORT_ONE);

        /* Reset keyboard and do self test */
        do {
                write_keyboard(PS2_KEYBOARD_RESET);
        } while (read_keyboard() != PS2_KEYBOARD_ACK);  /* Keep resending */
        /* Once we get an ACK, we should get self-test passed */
        if(read_keyboard() != PS2_KEYBOARD_SELF_TEST_PASS)
                return 1;

        /* Set scancode 2 */
        do {
                write_keyboard(PS2_KEYBOARD_SET_SCANCODE_SET);
                write_keyboard(2);
        } while (read_keyboard() != PS2_KEYBOARD_ACK);

        /* Enable scanning */
        do {
                write_keyboard(PS2_KEYBOARD_ENABLE_SCANNING);
        } while (read_keyboard() != PS2_KEYBOARD_ACK);

        return 0;
}
