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
#include "serial.h"
#include "vga.h"

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

        IRQ_set_handler(0x08, fault_handle_sp, NULL);   /* DF */
        IRQ_set_handler(0x0D, fault_handle_sp, NULL);   /* GP */
        IRQ_set_handler(0x0E, fault_handle_sp, NULL);   /* PF */
        printk("TSS test handlers intialized\n");

        GDT_init();
        printk("New GDT initialized\n");

        IRQ_init();
        printk("Interrupts setup\n");

        ps2_init();
        printk("PS2 initialized\n");

        SER_init();
        SER_write("serial test\n", 13);
        printk("Serial initialized\n");

        /* Unmask keyboard */
        IRQ_clear_mask(PIC_KEYBOARD);
        printk("Keyboard unmasked: ");

        while(1)
                asm("hlt");

        return;
}
