/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/vga.h
 *
 * Header for BIOS VGA interface functions
 * 
 */

#ifndef VGA_H
#define VGA_H                                   1

#define VGA_WIDTH                               80
#define VGA_HEIGHT                              25

#define VGA_ROW(x)                              (x) / VGA_WIDTH
#define VGA_COL(x)                              (x) % VGA_WIDTH

#define VGA_BG(x)                               (0b0111 & (x))<<12
#define VGA_FG(x)                               (0b1111 & (x))<<8

#define VGA_BLINK                               1<<15

enum vga_color {
        VGA_COLOR_BLACK = 0,
        VGA_COLOR_BLUE = 1,
        VGA_COLOR_GREEN = 2,
        VGA_COLOR_CYAN = 3,
        VGA_COLOR_RED = 4,
        VGA_COLOR_MAGENTA = 5,
        VGA_COLOR_BROWN = 6,
        VGA_COLOR_LIGHT_GREY = 7,
        VGA_COLOR_DARK_GREY = 8,
        VGA_COLOR_LIGHT_BLUE = 9,
        VGA_COLOR_LIGHT_GREEN = 10,
        VGA_COLOR_LIGHT_CYAN = 11,
        VGA_COLOR_LIGHT_RED = 12,
        VGA_COLOR_LIGHT_MAGENTA = 13,
        VGA_COLOR_LIGHT_BROWN = 14,
        VGA_COLOR_WHITE = 15,
};

int VGA_clear(void);
int VGA_display_char(char);
int VGA_display_str(const char *);

#endif /* #ifndef VGA_H */
