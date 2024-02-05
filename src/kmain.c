/* 
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/kmain.c
 *
 * Fragaria C entry point
 * 
 */

#include "vga.h"

void kmain()
{
        VGA_clear();

        VGA_display_str("fragaria starting\n\n");
        VGA_display_str("test\rr");

        for(int i = 0; i < 24; i++)
                VGA_display_str("a\n");

        while(1)
                asm("hlt");

        return;
}
