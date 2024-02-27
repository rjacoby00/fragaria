/* 
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/kmain.c
 *
 * Fragaria C entry point
 * 
 */

#include <stddef.h>
#include <stdint.h>

#include "irq.h"
#include "printk.h"
#include "ps2.h"
#include "vga.h"

static void keyboard_loop()
{
        char character;

        while(1)
                if((character = get_char())) VGA_display_char(character);
} 

static void pic_handle(int irq, uint32_t error, void * cr2, void * arg)
{
        printk("PIC got IRQ %d\n", irq - 0x20);
}

void kmain()
{
        VGA_clear();
        ps2_init();

        for(int i = 0x20; i < 0x30; i++)
                IRQ_set_handler(i, pic_handle, NULL);

        IRQ_init();

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

        keyboard_loop();


        return;
}
