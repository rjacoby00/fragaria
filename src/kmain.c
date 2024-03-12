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
#include "gdt.h"
#include "printk.h"
#include "ps2.h"
#include "vga.h"

/**
 * pic_handle() - Small irq_handle_t to adapt PS2 driver to irq based
 * @irq numer - subtract 0x20 to get PIC line
 * @error number(should always be zero)
 * @cr2 register contents; should always be zero
 * @arg unused
 * 
 */
static void pic_handle(int irq, uint32_t error, void * cr2, void * arg)
{
        irq -= PIC_1;

        switch(irq) {
        case PIC_KEYBOARD:
                        char character;
                        if((character = get_char()))
                                VGA_display_char(character);

                        break;
        default:
                        printk("Unhandled PIC IRQ: %d\n", irq);
        }

        IRQ_end_of_interrupt(irq);

        return;
}

static void fault_handle_sp(int irq, uint32_t error, void * cr2, void * arg)
{
        void * sp;

        asm("mov %%rsp, %0" : "=rm"(sp));

        printk("Fault at %p, new sp: %p\n", cr2, sp);

        return;
}

void kmain()
{
        VGA_clear();
        printk("fragaria starting\n\n");

        /* Mask all PIC interrupts */
        for(int i = 0; i < 16; i++)
                IRQ_set_mask(i);

        /* Register common PIC hanlder */
        for(int i = 0x20; i < 0x30; i++)
                IRQ_set_handler(i, pic_handle, NULL);

        IRQ_set_handler(0x08, fault_handle_sp, NULL);   /* DF */
        IRQ_set_handler(0x0D, fault_handle_sp, NULL);   /* GP */
        IRQ_set_handler(0x0E, fault_handle_sp, NULL);   /* PF */

        GDT_init();

        IRQ_init();

        ps2_init();

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

        /* Unmask keyboard */
        IRQ_clear_mask(1);

        while(1)
                asm("hlt");

        return;
}
