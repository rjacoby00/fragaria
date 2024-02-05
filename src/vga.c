/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/vga.c
 *
 * BIOS VGA interface functions
 */

#include <stdint.h>

#include "string.h"
#include "vga.h"

#define VGA_CONSOLE                             0x0B8000

static uint16_t * vgaBuff = (uint16_t *)VGA_CONSOLE;
static int cursor = 0;

/**
 * scroll() - small VGA utility; scrolls the BIOS VGA console one line
 *
 * Return: zero on success
 */
static int scroll()
{
        int i;

        /* Move the bottom 24 rows up one */
        /* TODO: memcpy is unsafe here(overlapping memory regions), swap for
         * memmove when kmalloc exists or rewrite memcpy here to guarantee
         * safety */
        memcpy(vgaBuff, vgaBuff + VGA_WIDTH, 2 * VGA_WIDTH * (VGA_HEIGHT - 1));
        
        /* Wipe the bottom row */
        for (i = 0; i < VGA_WIDTH; i++) {
                vgaBuff[VGA_WIDTH * (VGA_HEIGHT - 1) + i] = ' '
                        | VGA_FG(VGA_COLOR_LIGHT_GREY)
                        | VGA_BG(VGA_COLOR_BLACK);
        }

        /* Move the cursor up a row */
        cursor = cursor - VGA_WIDTH;

        return 0;
}

/**
 * VGA_clear() - clears BIOS VGA console
 *
 * Return: zero on success
 */
int VGA_clear()
{
        int i;

        /* Write a space in light grey on black for every character */
        for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
                vgaBuff[i] = ' ' | VGA_FG(VGA_COLOR_LIGHT_GREY)
                        | VGA_BG(VGA_COLOR_BLACK);
        }

        return 0;
}

/**
 * VGA_display_char() - write a character to the BIOS VGA console
 * @c: Character to print
 *
 * Return: zero on success
 */
int VGA_display_char(char c)
{
        if (c == '\n') {
                cursor = (VGA_ROW(cursor) + 1) * VGA_WIDTH;
                if(VGA_ROW(cursor) >= VGA_HEIGHT)
                        scroll();
        } else if (c == '\r') {
                cursor = VGA_ROW(cursor) * VGA_WIDTH;
        } else {
                vgaBuff[cursor] = c | VGA_FG(VGA_COLOR_LIGHT_GREY)
                        | VGA_BG(VGA_COLOR_BLACK);
                cursor++;
                if(VGA_ROW(cursor) >= VGA_HEIGHT)
                        scroll();
        }

        return 0;
}

/**
 * VGA_display_str() - print a string to BIOS VGA console
 * @str: String to print
 *
 * Return: zero on success
 */
int VGA_display_str(const char * str)
{
        while (*str) {
                VGA_display_char(*str);
                str++;
        }

        return 0;
}
