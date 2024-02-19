/* 
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/kmain.c
 *
 * Fragaria C entry point
 * 
 */

#include "printk.h"
#include "ps2.h"
#include "vga.h"

static void keyboard_loop()
{
        char character;

        while(1)
                if((character = get_char())) VGA_display_char(character);
} 

void kmain()
{
        VGA_clear();
        ps2_init();

        printk("fragaria starting\n\n");
        printk("test\n%d\n%d\n%d%%\rr\n", 1, 10, 100);
        printk("signed decimal:   %d\n", -24);
        printk("unsigned decimal: %u\n", -24);
        printk("unsigned hex:     %x\n", -24);
        printk("signed decimal:   %d\n", 0x03FE);
        printk("unsigned decimal: %u\n", 0x03FE);
        printk("unsigned hex:     %x\n", 0x03FE);
        printk("some characters:%c%c%c%c\n", ' ', 'e', 'a', 't');
        printk("%s\n", "%x%x%x%x%x%x - should be escaped");
        printk("short dec:   %hd\n", 0x03FE);
        printk("short unsig: %hu\n", 0x03FE);
        printk("short hex:   %hx\n", 0x03FE);
        printk("long dec:   %ld\n", (long)-24);
        printk("long unsig: %lu\n", (long)-24);
        printk("long hex:   %lx\n", (long)-24);
        printk("pointer:    %p\n", kmain);


        for(int i = 0; i < 5; i++)
                VGA_display_str("a\n");

        keyboard_loop();


        return;
}
