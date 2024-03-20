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
#include "mm.h"
#include "multiboot.h"
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

void kmain(uint32_t magic, struct multiboot_table_header * multiboot)
{
        VGA_clear();
        printk("fragaria starting\n\n");

        if(magic != MULTIBOOT_MAGIC) {
                printk("Started from non-multiboot bootloader...\nstopping.");
                asm("hlt");
        }

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

        MM_init(multiboot);

        /* Alloc one frame and write some data to it */
        for (int i = 0; i < 100; i++) {
                void * frame = MM_pf_alloc();

                printk("Got frame at %p\n", frame);

                for (int j = 0; j < MM_PF_SIZE; j++) {
                        *((uint8_t *)frame + j)  = j % 255;
                }

                for (int j = 0; j < MM_PF_SIZE; j++) {
                        if(*((uint8_t *)frame + j) != j % 255)
                                printk("RAM ERROR!\n");
                }

                MM_pf_free(frame);
        }

        /* Allocate 1+512 frames */
        {
                void ** frames = MM_pf_alloc();

                printk("\nUsing frame at %p to store 512 more\n", frames);

                for (int i = 0; i < 512; i++) {
                        frames[i] = MM_pf_alloc();
                        printk("Got frame at %p\n", frames[i]);
                }

                for (int i = 0; i < 512; i++) {
                        MM_pf_free(frames[i]);
                        printk("Freed frame at %p\n", frames[i]);
                }

                printk("1+512 frames OK\n\n");

                MM_pf_free(frames);
        }

        /* Allocate 2+1024 frames */
        {
                void ** frames = MM_pf_alloc();
                void ** frames2 = MM_pf_alloc();

                printk("Testing 2+1024 frames (%p, %p)\n", frames, frames2);

                for (int i = 0; i < 512; i++) {
                        frames[i] = MM_pf_alloc();
                        frames2[i] = MM_pf_alloc();
                }

                for (int i = 0; i < 512; i++) {
                        MM_pf_free(frames2[i]);
                        MM_pf_free(frames[i]);
                }

                MM_pf_free(frames);
                MM_pf_free(frames2);

                printk("2+1024 freed OK\n\n");
        }

        /* Allocate every frame; we have 32,639 frames, minus ones in use
         * already; the addresses should fit in 64 frames */
        {
                void ** all_frames[64];
                void * new_frame;
                int count = 0;

                printk("Testing allocate every frame\n");

                for (int i = 0; i < 64; i++) {
                        all_frames[i] = MM_pf_alloc();
                }

                while ((new_frame = MM_pf_alloc()) != MM_FRAME_EMPTY) {
                        all_frames[count / 512][count % 512] = new_frame;
                        count++;
                }

                printk("Allocated %d frames\n", count);

                printk("Writing alloc number to every frame 512 times\n");

                for (int i = 0; i < count; i++) {
                        for (int j = 0; j < 512; j++) {
                                *(uint64_t *)all_frames[i / 512][i % 512] = i;
                        }
                }

                for (int i = 0; i < count; i++) {
                        for (int j = 0; j < 512; j++) {
                                if(*(uint64_t *)all_frames[i / 512][i % 512] != i) {
                                        printk("TEST FAILED\n");
                                        return;
                                }
                        }
                        MM_pf_free(all_frames[i / 512][i % 512]);
                }

                printk("Test OK, all frames passed\n");


        }

        /* Unmask keyboard */
        IRQ_clear_mask(PIC_KEYBOARD);
        printk("Keyboard unmasked: ");

        while(1)
                asm("hlt");

        return;
}
