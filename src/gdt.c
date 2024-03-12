/* 
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/gdt.c
 *
 * A better GDT than the one we built in assembly
 * 
 */

#include <stdint.h>

#include "gdt.h"
#include "irq.h"

/* A small GDT with up to 8 entries (we shouldn't need more ever) */
struct gdt {
        uint64_t table[8];
        uint8_t next_free;
} gdt;

struct tss tss;

void GDT_init()
{
        uint8_t enable_ints = 0;
        static struct {
                uint16_t limit;
                uint64_t base;
        } __attribute__((packed)) gdt_pointer;

        gdt.table[0] = 0;

        /* Set up a new kernel-mode code descriptor */
        gdt.table[1] = 0;
        ((struct segment_descriptor *)(gdt.table + 1))->conforming = 1;
        ((struct segment_descriptor *)(gdt.table + 1))->executable = 1;
        ((struct segment_descriptor *)(gdt.table + 1))->s_type = 1;
        ((struct segment_descriptor *)(gdt.table + 1))->dpl = 0;
        ((struct segment_descriptor *)(gdt.table + 1))->present = 1; 
        ((struct segment_descriptor *)(gdt.table + 1))->long_mode = 1;

        /* Set up a TSS descriptor */
        gdt.table[2] = 0;
        gdt.table[3] = 0;
        ((struct tss_descriptor *)(gdt.table + 2))->segment_limit0 =
                        (0x0000FFFF & (sizeof(tss) - 1)) >> 0;
        ((struct tss_descriptor *)(gdt.table + 2))->segment_limit1 =
                        (0x00FF0000 & (sizeof(tss) - 1)) >> 16;

        ((struct tss_descriptor *)(gdt.table + 2))->base_address0 = 
                        ((uint64_t)&tss & 0x000000000000FFFF) >> 0;
        ((struct tss_descriptor *)(gdt.table + 2))->base_address1 =
                        ((uint64_t)&tss & 0x0000000000FF0000) >> 16;
        ((struct tss_descriptor *)(gdt.table + 2))->base_address2 =
                        ((uint64_t)&tss & 0x00000000FF000000) >> 24;
        ((struct tss_descriptor *)(gdt.table + 2))->base_address3 =
                        ((uint64_t)&tss & 0xFFFFFFFF00000000) >> 32;

        ((struct tss_descriptor *)(gdt.table + 2))->type = TSS_64;
        ((struct tss_descriptor *)(gdt.table + 2))->mbz0 = 0;
        ((struct tss_descriptor *)(gdt.table + 2))->dpl = 0;
        ((struct tss_descriptor *)(gdt.table + 2))->present = 1;
        ((struct tss_descriptor *)(gdt.table + 2))->avl = 0;

        ((struct tss_descriptor *)(gdt.table + 2))->mbz1 = 0;

        /* 0*8 is the null descriptor, 1*8 is kernel code, 2*8 is TSS but it
         * takes two slots, so 4*8 is the next free when we add user modes */
        gdt.next_free = 4;

        /* Now let's fill the TSS itself */
        tss.reserved0 = 0;
        for (int i = 0; i < sizeof(tss.rsp) / sizeof(tss.rsp[0]); i++)
                tss.rsp[i] = 0;
        tss.reserved1 = 0;
        for (int i = 0; i < sizeof(tss.ist) / sizeof(tss.ist[0]); i++)
                tss.ist[i] = 0;
        tss.reserved2 = 0;
        tss.reserved3 = 0;
        tss.io_map_base = sizeof(tss);

        tss.ist[PF_IST_INDEX] = (uint64_t)(&pf_stack_top);
        tss.ist[DF_IST_INDEX] = (uint64_t)(&df_stack_top);
        tss.ist[GP_IST_INDEX] = (uint64_t)(&gp_stack_top);

        /* Set up the GDT pointer */
        gdt_pointer.limit = sizeof(gdt.table) - 1;
        gdt_pointer.base = (uint64_t)(&gdt.table);

        /* Make sure to swap gdt with interrupts disabled */
        if (interrupts_enabled()) {
                enable_ints = 1;
                CLI;
        }

        asm("lgdt %0" :: "m"(gdt_pointer));
        asm("mov $16, %%ax ; ltr %%ax" ::: "rax");

        if(enable_ints)
                STI;

        return;
}
