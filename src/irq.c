/* 
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/irq.c
 *
 * Interrupt setup and handler C code
 * 
 */

#include <stddef.h>
#include <stdint.h>

#include "gdt.h"
#include "irq.h"
#include "port_io.h"
#include "printk.h"
#include "ps2.h"        /* Used for hooking PIC handler to driver */
#include "serial.h"     /* Used for hooking PIC handler to driver */

/* Main IDT */
struct idt_entry idt[256];

/* Space to put registered IRQ handlers */
static struct {
        void * arg;
        irq_handler_t handler;
} irq_table[NUM_IRQS] = {0};

/**
 * pic_handle() - Small irq_handle_t to handle PIC and dispach to multiple devs
 * @irq numer - subtract 0x20 to get PIC line
 * @error number(should always be zero)
 * @cr2 register contents; should always be zero
 * @arg unused
 * 
 * This provides a basic way to handle multiple deviceas on the same PIC channel
 * until there's a dynamic way to register multiple PIC/IRQ handlers per
 * channel/irq.
 * 
 */
static void pic_handle(int irq, uint32_t error, void * cr2, void * arg)
{
        irq -= PIC_1;

        switch(irq) {
        case PIC_KEYBOARD:
                        ps2_pic_handle();
                        break; 
        case PIC_COM1:
        case PIC_COM2:
                        serial_pic_handle();
                        break;
        default:
                        printk("Unhandled PIC IRQ: %d\n", irq);
        }

        IRQ_end_of_interrupt(irq);

        return;
}

/**
 * irq_c_handler() - C entry to interrupt handling
 * @irq Interrupt number passed by assembly code
 * @error Optional error number; will be set to zero if not used by irq
 * @cr2 Value in CR2 register when interrupt happened
 * @frame pointer to IRQ_stack frame, now modified to always have error code
 * 
 */
void irq_c_handler(int irq, uint32_t error, void * cr2, 
        struct irq_stack_frame * frame)
{

        if (irq_table[irq].handler) {
                irq_table[irq].handler(irq, error, cr2, irq_table[irq].arg);
        } else {
                printk("ERROR: Unhandled interrupt: 0x%x\n", irq);
                printk("    Error: %x\n", error);
                printk("    CR2: %p\n", cr2);
                printk("    Instruction Pointer: 0x%lx\n", frame->rip);
                printk("    Code Segment: %d\n", frame->cs);
                printk("    CPU Flags: %lx\n", frame->rflags);
                printk("    Stack Pointer: 0x%lx\n", frame->rsp);
                printk("    Stack Segment: %d\n", frame->ss);
                printk("Unrecoverable error, stopping...\n");
                asm("hlt");
        }

        return;
}

/**
 * remap_PIC() - small helper function to remap both PICs 
 * @offset1 starting irq offset of PIC1
 * @offset2 starting irq offset of PIC2
 * 
 * IRQs from PICs will be mapped from [offset, offset+8]
 * 
 */
static void remap_PIC(int offset1, int offset2)
{
        uint8_t mask1, mask2;

        /* Save masks */
        mask1 = inb(PIC_1_DATA);
        mask2 = inb(PIC_2_DATA);

        /* Start init */
        outb(PIC_1_COMMAND, ICW1_INIT | ICW1_ICW4);
        io_wait();
        outb(PIC_2_COMMAND, ICW1_INIT | ICW1_ICW4);
        io_wait();
        /* Set starting IRQ num for PIC1 */
        outb(PIC_1_DATA, offset1);
        io_wait();
        /* Set starting IRQ num for PIC2 */
        outb(PIC_2_DATA, offset2);
        io_wait();
        /* Tell PIC1 that PIC2 is at IRQ2 */
        outb(PIC_1_DATA, 4);
        io_wait();
        /* Tell PIC2 that it's identity is 0x02 */
        outb(PIC_2_DATA, 2);
        io_wait();

        /* Tell the PICs to use 8086 mode */
        outb(PIC_1_DATA, ICW4_8086);
        io_wait();
        outb(PIC_2_DATA, ICW4_8086);
        io_wait();

        /* Restore masks */
        outb(PIC_1_DATA, mask1);
        outb(PIC_2_DATA, mask2);

        return;
}

/**
 * set_idt_entry() - small helper function to setup an idt entry
 * @target Pointer to handler function
 * @irq number
 *
 */
static void set_idt_entry(uint64_t target, int irq, int stack)
{
        /* Set target */
        idt[irq].target_offset0 = (0x000000000000FFFF & target) >> 0;
        idt[irq].target_offset1 = (0x00000000FFFF0000 & target) >> 16;
        idt[irq].target_offset2 = (0xFFFFFFFF00000000 & target) >> 32;

        /* Set selector(GDT entry) for code execution */
        idt[irq].target_selector = 8;
        
        /* Set interrupt stack table to unused */
        idt[irq].ist = stack;

        /* Set type (we're never using traps) */
        idt[irq].type = INT_TYPE_TRAP;

        /* Set privilege (we're staying in ring 0 for now) */
        idt[irq].dpl = 0;

        /* Set present */
        idt[irq].present = 1;

        /* Zero mbz and reserved */
        idt[irq].reserved0 = 0;
        idt[irq].reserved1 = 0;
        idt[irq].mbz = 0;

        return;
}

/**
 * IRQ_init() - Set up interrupt handling and base table
 *
 */
void IRQ_init()
{
        static struct {
                uint16_t limit;
                uint64_t base;
        } __attribute__((packed)) idt_pointer;

        /* Disable interrupts */
        CLI;

        /* Setup PIC handle stuff first */
        /* Mask all PIC interrupts */
        for(int i = 0; i < 16; i++)
                IRQ_set_mask(i);

        /* Register common PIC hanlder */
        for(int i = 0x20; i < 0x30; i++)
                IRQ_set_handler(i, pic_handle, NULL);

        /* Remap PICs to interrupts 0x20-0x2F */
        remap_PIC(0x20, 0x28);

        /* Now setup IDT */
        /* Set up the argument to pass to lidt */
        idt_pointer.limit = sizeof(idt) - 1;
        idt_pointer.base = (uint64_t)idt;

        /* Set up IDT to point all 256 interrupts back to our code */
        set_idt_entry((uint64_t)irq0x00, 0x00, 0);
        set_idt_entry((uint64_t)irq0x01, 0x01, 0);
        set_idt_entry((uint64_t)irq0x02, 0x02, 0);
        set_idt_entry((uint64_t)irq0x03, 0x03, 0);
        set_idt_entry((uint64_t)irq0x04, 0x04, 0);
        set_idt_entry((uint64_t)irq0x05, 0x05, 0);
        set_idt_entry((uint64_t)irq0x06, 0x06, 0);
        set_idt_entry((uint64_t)irq0x07, 0x07, 0);
        set_idt_entry((uint64_t)irq0x08, 0x08, DF_IST_INDEX + 1);
        set_idt_entry((uint64_t)irq0x09, 0x09, 0);
        set_idt_entry((uint64_t)irq0x0A, 0x0A, 0);
        set_idt_entry((uint64_t)irq0x0B, 0x0B, 0);
        set_idt_entry((uint64_t)irq0x0C, 0x0C, 0);
        set_idt_entry((uint64_t)irq0x0D, 0x0D, GP_IST_INDEX + 1);
        set_idt_entry((uint64_t)irq0x0E, 0x0E, PF_IST_INDEX + 1);
        set_idt_entry((uint64_t)irq0x0F, 0x0F, 0);
        set_idt_entry((uint64_t)irq0x10, 0x10, 0);
        set_idt_entry((uint64_t)irq0x11, 0x11, 0);
        set_idt_entry((uint64_t)irq0x12, 0x12, 0);
        set_idt_entry((uint64_t)irq0x13, 0x13, 0);
        set_idt_entry((uint64_t)irq0x14, 0x14, 0);
        set_idt_entry((uint64_t)irq0x15, 0x15, 0);
        set_idt_entry((uint64_t)irq0x16, 0x16, 0);
        set_idt_entry((uint64_t)irq0x17, 0x17, 0);
        set_idt_entry((uint64_t)irq0x18, 0x18, 0);
        set_idt_entry((uint64_t)irq0x19, 0x19, 0);
        set_idt_entry((uint64_t)irq0x1A, 0x1A, 0);
        set_idt_entry((uint64_t)irq0x1B, 0x1B, 0);
        set_idt_entry((uint64_t)irq0x1C, 0x1C, 0);
        set_idt_entry((uint64_t)irq0x1D, 0x1D, 0);
        set_idt_entry((uint64_t)irq0x1E, 0x1E, 0);
        set_idt_entry((uint64_t)irq0x1F, 0x1F, 0);
        set_idt_entry((uint64_t)irq0x20, 0x20, 0);
        set_idt_entry((uint64_t)irq0x21, 0x21, 0);
        set_idt_entry((uint64_t)irq0x22, 0x22, 0);
        set_idt_entry((uint64_t)irq0x23, 0x23, 0);
        set_idt_entry((uint64_t)irq0x24, 0x24, 0);
        set_idt_entry((uint64_t)irq0x25, 0x25, 0);
        set_idt_entry((uint64_t)irq0x26, 0x26, 0);
        set_idt_entry((uint64_t)irq0x27, 0x27, 0);
        set_idt_entry((uint64_t)irq0x28, 0x28, 0);
        set_idt_entry((uint64_t)irq0x29, 0x29, 0);
        set_idt_entry((uint64_t)irq0x2A, 0x2A, 0);
        set_idt_entry((uint64_t)irq0x2B, 0x2B, 0);
        set_idt_entry((uint64_t)irq0x2C, 0x2C, 0);
        set_idt_entry((uint64_t)irq0x2D, 0x2D, 0);
        set_idt_entry((uint64_t)irq0x2E, 0x2E, 0);
        set_idt_entry((uint64_t)irq0x2F, 0x2F, 0);
        set_idt_entry((uint64_t)irq0x30, 0x30, 0);
        set_idt_entry((uint64_t)irq0x31, 0x31, 0);
        set_idt_entry((uint64_t)irq0x32, 0x32, 0);
        set_idt_entry((uint64_t)irq0x33, 0x33, 0);
        set_idt_entry((uint64_t)irq0x34, 0x34, 0);
        set_idt_entry((uint64_t)irq0x35, 0x35, 0);
        set_idt_entry((uint64_t)irq0x36, 0x36, 0);
        set_idt_entry((uint64_t)irq0x37, 0x37, 0);
        set_idt_entry((uint64_t)irq0x38, 0x38, 0);
        set_idt_entry((uint64_t)irq0x39, 0x39, 0);
        set_idt_entry((uint64_t)irq0x3A, 0x3A, 0);
        set_idt_entry((uint64_t)irq0x3B, 0x3B, 0);
        set_idt_entry((uint64_t)irq0x3C, 0x3C, 0);
        set_idt_entry((uint64_t)irq0x3D, 0x3D, 0);
        set_idt_entry((uint64_t)irq0x3E, 0x3E, 0);
        set_idt_entry((uint64_t)irq0x3F, 0x3F, 0);
        set_idt_entry((uint64_t)irq0x40, 0x40, 0);
        set_idt_entry((uint64_t)irq0x41, 0x41, 0);
        set_idt_entry((uint64_t)irq0x42, 0x42, 0);
        set_idt_entry((uint64_t)irq0x43, 0x43, 0);
        set_idt_entry((uint64_t)irq0x44, 0x44, 0);
        set_idt_entry((uint64_t)irq0x45, 0x45, 0);
        set_idt_entry((uint64_t)irq0x46, 0x46, 0);
        set_idt_entry((uint64_t)irq0x47, 0x47, 0);
        set_idt_entry((uint64_t)irq0x48, 0x48, 0);
        set_idt_entry((uint64_t)irq0x49, 0x49, 0);
        set_idt_entry((uint64_t)irq0x4A, 0x4A, 0);
        set_idt_entry((uint64_t)irq0x4B, 0x4B, 0);
        set_idt_entry((uint64_t)irq0x4C, 0x4C, 0);
        set_idt_entry((uint64_t)irq0x4D, 0x4D, 0);
        set_idt_entry((uint64_t)irq0x4E, 0x4E, 0);
        set_idt_entry((uint64_t)irq0x4F, 0x4F, 0);
        set_idt_entry((uint64_t)irq0x50, 0x50, 0);
        set_idt_entry((uint64_t)irq0x51, 0x51, 0);
        set_idt_entry((uint64_t)irq0x52, 0x52, 0);
        set_idt_entry((uint64_t)irq0x53, 0x53, 0);
        set_idt_entry((uint64_t)irq0x54, 0x54, 0);
        set_idt_entry((uint64_t)irq0x55, 0x55, 0);
        set_idt_entry((uint64_t)irq0x56, 0x56, 0);
        set_idt_entry((uint64_t)irq0x57, 0x57, 0);
        set_idt_entry((uint64_t)irq0x58, 0x58, 0);
        set_idt_entry((uint64_t)irq0x59, 0x59, 0);
        set_idt_entry((uint64_t)irq0x5A, 0x5A, 0);
        set_idt_entry((uint64_t)irq0x5B, 0x5B, 0);
        set_idt_entry((uint64_t)irq0x5C, 0x5C, 0);
        set_idt_entry((uint64_t)irq0x5D, 0x5D, 0);
        set_idt_entry((uint64_t)irq0x5E, 0x5E, 0);
        set_idt_entry((uint64_t)irq0x5F, 0x5F, 0);
        set_idt_entry((uint64_t)irq0x60, 0x60, 0);
        set_idt_entry((uint64_t)irq0x61, 0x61, 0);
        set_idt_entry((uint64_t)irq0x62, 0x62, 0);
        set_idt_entry((uint64_t)irq0x63, 0x63, 0);
        set_idt_entry((uint64_t)irq0x64, 0x64, 0);
        set_idt_entry((uint64_t)irq0x65, 0x65, 0);
        set_idt_entry((uint64_t)irq0x66, 0x66, 0);
        set_idt_entry((uint64_t)irq0x67, 0x67, 0);
        set_idt_entry((uint64_t)irq0x68, 0x68, 0);
        set_idt_entry((uint64_t)irq0x69, 0x69, 0);
        set_idt_entry((uint64_t)irq0x6A, 0x6A, 0);
        set_idt_entry((uint64_t)irq0x6B, 0x6B, 0);
        set_idt_entry((uint64_t)irq0x6C, 0x6C, 0);
        set_idt_entry((uint64_t)irq0x6D, 0x6D, 0);
        set_idt_entry((uint64_t)irq0x6E, 0x6E, 0);
        set_idt_entry((uint64_t)irq0x6F, 0x6F, 0);
        set_idt_entry((uint64_t)irq0x70, 0x70, 0);
        set_idt_entry((uint64_t)irq0x71, 0x71, 0);
        set_idt_entry((uint64_t)irq0x72, 0x72, 0);
        set_idt_entry((uint64_t)irq0x73, 0x73, 0);
        set_idt_entry((uint64_t)irq0x74, 0x74, 0);
        set_idt_entry((uint64_t)irq0x75, 0x75, 0);
        set_idt_entry((uint64_t)irq0x76, 0x76, 0);
        set_idt_entry((uint64_t)irq0x77, 0x77, 0);
        set_idt_entry((uint64_t)irq0x78, 0x78, 0);
        set_idt_entry((uint64_t)irq0x79, 0x79, 0);
        set_idt_entry((uint64_t)irq0x7A, 0x7A, 0);
        set_idt_entry((uint64_t)irq0x7B, 0x7B, 0);
        set_idt_entry((uint64_t)irq0x7C, 0x7C, 0);
        set_idt_entry((uint64_t)irq0x7D, 0x7D, 0);
        set_idt_entry((uint64_t)irq0x7E, 0x7E, 0);
        set_idt_entry((uint64_t)irq0x7F, 0x7F, 0);
        set_idt_entry((uint64_t)irq0x80, 0x80, 0);
        set_idt_entry((uint64_t)irq0x81, 0x81, 0);
        set_idt_entry((uint64_t)irq0x82, 0x82, 0);
        set_idt_entry((uint64_t)irq0x83, 0x83, 0);
        set_idt_entry((uint64_t)irq0x84, 0x84, 0);
        set_idt_entry((uint64_t)irq0x85, 0x85, 0);
        set_idt_entry((uint64_t)irq0x86, 0x86, 0);
        set_idt_entry((uint64_t)irq0x87, 0x87, 0);
        set_idt_entry((uint64_t)irq0x88, 0x88, 0);
        set_idt_entry((uint64_t)irq0x89, 0x89, 0);
        set_idt_entry((uint64_t)irq0x8A, 0x8A, 0);
        set_idt_entry((uint64_t)irq0x8B, 0x8B, 0);
        set_idt_entry((uint64_t)irq0x8C, 0x8C, 0);
        set_idt_entry((uint64_t)irq0x8D, 0x8D, 0);
        set_idt_entry((uint64_t)irq0x8E, 0x8E, 0);
        set_idt_entry((uint64_t)irq0x8F, 0x8F, 0);
        set_idt_entry((uint64_t)irq0x90, 0x90, 0);
        set_idt_entry((uint64_t)irq0x91, 0x91, 0);
        set_idt_entry((uint64_t)irq0x92, 0x92, 0);
        set_idt_entry((uint64_t)irq0x93, 0x93, 0);
        set_idt_entry((uint64_t)irq0x94, 0x94, 0);
        set_idt_entry((uint64_t)irq0x95, 0x95, 0);
        set_idt_entry((uint64_t)irq0x96, 0x96, 0);
        set_idt_entry((uint64_t)irq0x97, 0x97, 0);
        set_idt_entry((uint64_t)irq0x98, 0x98, 0);
        set_idt_entry((uint64_t)irq0x99, 0x99, 0);
        set_idt_entry((uint64_t)irq0x9A, 0x9A, 0);
        set_idt_entry((uint64_t)irq0x9B, 0x9B, 0);
        set_idt_entry((uint64_t)irq0x9C, 0x9C, 0);
        set_idt_entry((uint64_t)irq0x9D, 0x9D, 0);
        set_idt_entry((uint64_t)irq0x9E, 0x9E, 0);
        set_idt_entry((uint64_t)irq0x9F, 0x9F, 0);
        set_idt_entry((uint64_t)irq0xA0, 0xA0, 0);
        set_idt_entry((uint64_t)irq0xA1, 0xA1, 0);
        set_idt_entry((uint64_t)irq0xA2, 0xA2, 0);
        set_idt_entry((uint64_t)irq0xA3, 0xA3, 0);
        set_idt_entry((uint64_t)irq0xA4, 0xA4, 0);
        set_idt_entry((uint64_t)irq0xA5, 0xA5, 0);
        set_idt_entry((uint64_t)irq0xA6, 0xA6, 0);
        set_idt_entry((uint64_t)irq0xA7, 0xA7, 0);
        set_idt_entry((uint64_t)irq0xA8, 0xA8, 0);
        set_idt_entry((uint64_t)irq0xA9, 0xA9, 0);
        set_idt_entry((uint64_t)irq0xAA, 0xAA, 0);
        set_idt_entry((uint64_t)irq0xAB, 0xAB, 0);
        set_idt_entry((uint64_t)irq0xAC, 0xAC, 0);
        set_idt_entry((uint64_t)irq0xAD, 0xAD, 0);
        set_idt_entry((uint64_t)irq0xAE, 0xAE, 0);
        set_idt_entry((uint64_t)irq0xAF, 0xAF, 0);
        set_idt_entry((uint64_t)irq0xB0, 0xB0, 0);
        set_idt_entry((uint64_t)irq0xB1, 0xB1, 0);
        set_idt_entry((uint64_t)irq0xB2, 0xB2, 0);
        set_idt_entry((uint64_t)irq0xB3, 0xB3, 0);
        set_idt_entry((uint64_t)irq0xB4, 0xB4, 0);
        set_idt_entry((uint64_t)irq0xB5, 0xB5, 0);
        set_idt_entry((uint64_t)irq0xB6, 0xB6, 0);
        set_idt_entry((uint64_t)irq0xB7, 0xB7, 0);
        set_idt_entry((uint64_t)irq0xB8, 0xB8, 0);
        set_idt_entry((uint64_t)irq0xB9, 0xB9, 0);
        set_idt_entry((uint64_t)irq0xBA, 0xBA, 0);
        set_idt_entry((uint64_t)irq0xBB, 0xBB, 0);
        set_idt_entry((uint64_t)irq0xBC, 0xBC, 0);
        set_idt_entry((uint64_t)irq0xBD, 0xBD, 0);
        set_idt_entry((uint64_t)irq0xBE, 0xBE, 0);
        set_idt_entry((uint64_t)irq0xBF, 0xBF, 0);
        set_idt_entry((uint64_t)irq0xC0, 0xC0, 0);
        set_idt_entry((uint64_t)irq0xC1, 0xC1, 0);
        set_idt_entry((uint64_t)irq0xC2, 0xC2, 0);
        set_idt_entry((uint64_t)irq0xC3, 0xC3, 0);
        set_idt_entry((uint64_t)irq0xC4, 0xC4, 0);
        set_idt_entry((uint64_t)irq0xC5, 0xC5, 0);
        set_idt_entry((uint64_t)irq0xC6, 0xC6, 0);
        set_idt_entry((uint64_t)irq0xC7, 0xC7, 0);
        set_idt_entry((uint64_t)irq0xC8, 0xC8, 0);
        set_idt_entry((uint64_t)irq0xC9, 0xC9, 0);
        set_idt_entry((uint64_t)irq0xCA, 0xCA, 0);
        set_idt_entry((uint64_t)irq0xCB, 0xCB, 0);
        set_idt_entry((uint64_t)irq0xCC, 0xCC, 0);
        set_idt_entry((uint64_t)irq0xCD, 0xCD, 0);
        set_idt_entry((uint64_t)irq0xCE, 0xCE, 0);
        set_idt_entry((uint64_t)irq0xCF, 0xCF, 0);
        set_idt_entry((uint64_t)irq0xD0, 0xD0, 0);
        set_idt_entry((uint64_t)irq0xD1, 0xD1, 0);
        set_idt_entry((uint64_t)irq0xD2, 0xD2, 0);
        set_idt_entry((uint64_t)irq0xD3, 0xD3, 0);
        set_idt_entry((uint64_t)irq0xD4, 0xD4, 0);
        set_idt_entry((uint64_t)irq0xD5, 0xD5, 0);
        set_idt_entry((uint64_t)irq0xD6, 0xD6, 0);
        set_idt_entry((uint64_t)irq0xD7, 0xD7, 0);
        set_idt_entry((uint64_t)irq0xD8, 0xD8, 0);
        set_idt_entry((uint64_t)irq0xD9, 0xD9, 0);
        set_idt_entry((uint64_t)irq0xDA, 0xDA, 0);
        set_idt_entry((uint64_t)irq0xDB, 0xDB, 0);
        set_idt_entry((uint64_t)irq0xDC, 0xDC, 0);
        set_idt_entry((uint64_t)irq0xDD, 0xDD, 0);
        set_idt_entry((uint64_t)irq0xDE, 0xDE, 0);
        set_idt_entry((uint64_t)irq0xDF, 0xDF, 0);
        set_idt_entry((uint64_t)irq0xE0, 0xE0, 0);
        set_idt_entry((uint64_t)irq0xE1, 0xE1, 0);
        set_idt_entry((uint64_t)irq0xE2, 0xE2, 0);
        set_idt_entry((uint64_t)irq0xE3, 0xE3, 0);
        set_idt_entry((uint64_t)irq0xE4, 0xE4, 0);
        set_idt_entry((uint64_t)irq0xE5, 0xE5, 0);
        set_idt_entry((uint64_t)irq0xE6, 0xE6, 0);
        set_idt_entry((uint64_t)irq0xE7, 0xE7, 0);
        set_idt_entry((uint64_t)irq0xE8, 0xE8, 0);
        set_idt_entry((uint64_t)irq0xE9, 0xE9, 0);
        set_idt_entry((uint64_t)irq0xEA, 0xEA, 0);
        set_idt_entry((uint64_t)irq0xEB, 0xEB, 0);
        set_idt_entry((uint64_t)irq0xEC, 0xEC, 0);
        set_idt_entry((uint64_t)irq0xED, 0xED, 0);
        set_idt_entry((uint64_t)irq0xEE, 0xEE, 0);
        set_idt_entry((uint64_t)irq0xEF, 0xEF, 0);
        set_idt_entry((uint64_t)irq0xF0, 0xF0, 0);
        set_idt_entry((uint64_t)irq0xF1, 0xF1, 0);
        set_idt_entry((uint64_t)irq0xF2, 0xF2, 0);
        set_idt_entry((uint64_t)irq0xF3, 0xF3, 0);
        set_idt_entry((uint64_t)irq0xF4, 0xF4, 0);
        set_idt_entry((uint64_t)irq0xF5, 0xF5, 0);
        set_idt_entry((uint64_t)irq0xF6, 0xF6, 0);
        set_idt_entry((uint64_t)irq0xF7, 0xF7, 0);
        set_idt_entry((uint64_t)irq0xF8, 0xF8, 0);
        set_idt_entry((uint64_t)irq0xF9, 0xF9, 0);
        set_idt_entry((uint64_t)irq0xFA, 0xFA, 0);
        set_idt_entry((uint64_t)irq0xFB, 0xFB, 0);
        set_idt_entry((uint64_t)irq0xFC, 0xFC, 0);
        set_idt_entry((uint64_t)irq0xFD, 0xFD, 0);
        set_idt_entry((uint64_t)irq0xFE, 0xFE, 0);
        set_idt_entry((uint64_t)irq0xFF, 0xFF, 0);

        /* Load the IDT */
        asm("lidt %0" :: "m"(idt_pointer));

        /* Enable interrupts */
        STI;

        return;
}

/**
 * IRQ_set_mask() - set mask on any PIC line 
 * @irq mask to set
 * 
 */
void IRQ_set_mask(int irq)
{
        uint16_t port;
        uint8_t value;

        if (irq < 8) {
                port = PIC_1_DATA;
        } else {
                port = PIC_2_DATA;
                irq -= 8;
        }

        /* Get the mask and set the appropriate IRQ bit */
        value = inb(port) | (1 << irq);

        /* Write it */
        outb(port, value);

        return;
}

/**
 * IRQ_clear_mask() - Clear mask for any PIC line 
 * @irq to clear
 * 
 */
void IRQ_clear_mask(int irq)
{
        uint16_t port;
        uint8_t value;

        if (irq < 8) {
                port = PIC_1_DATA;
        } else {
                port = PIC_2_DATA;
                irq -= 8;
        }

        /* Get the mask and clear appropriate IRQ bit */
        value = inb(port) & (~(1 << irq));

        /* Write it */
        outb(port, value);

        return;
}

/**
 * IRQ_get_mask() - get mask state for any IRQ line 
 * @irq number to querry
 * 
 * @return zero on masked, appropriate bit on unmasked
 */
int IRQ_get_mask(int irq)
{
        uint16_t port;

        if (irq < 8) {
                port = PIC_1_DATA;
        } else {
                port = PIC_2_DATA;
                irq -= 8;
        }

        return inb(port) & (1 << irq);
}

/**
 * IRQ_end_of_interrupt() - clear interrupt from PIC 
 * @irq number to clear
 * 
 */
void IRQ_end_of_interrupt(int irq)
{
        if(irq >= 8)
                outb(PIC_2_COMMAND, PIC_EOI);

        outb(PIC_1_COMMAND, PIC_EOI);

        return;
}

/**
 * IRQ_set_handler() - Setup irq handler
 * @irq number to handle
 * @handler function pointer to handler code
 * @arg void pointer argument to pass to handler
 * 
 * IRQ handler table is initialized to zeroes, it should be safe to call before
 * setting up interrupts
 */
void IRQ_set_handler(int irq, irq_handler_t handler, void * arg)
{
        if(irq < NUM_IRQS) {
                irq_table[irq].handler = handler;
                irq_table[irq].arg = arg;
        }

        return;
}
