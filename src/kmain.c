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
#include "kmalloc.h"
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

        /* Test heap allocator and demand paging */
        {
                void * heap = MMU_alloc_pages(16);

                printk("heap at %p\n", heap);

                for (int i = 0; i < 4096; i++) {
                        ((uint64_t *)heap)[i] = i;
                }

                for (int i = 0; i < 4096; i++) {
                        if (((uint64_t *)heap)[i] != i)
                                printk("Virtual memory error!\n");
                }

                MMU_free_page(heap);
        }

        /* Test realloc the same pages (they should already be demand paged)*/
        {
                void * heap = MMU_alloc_pages(16);

                printk("heap at %p\n", heap);

                for (int i = 0; i < 4096; i++) {
                        ((uint64_t *)heap)[i] = i;
                }

                for (int i = 0; i < 4096; i++) {
                        if (((uint64_t *)heap)[i] != i)
                                printk("Virtual memory error!\n");
                }

                MMU_free_page(heap);
        }
        
        /* Test simple malloc */
        {
                void * ptr;

                if (!(ptr = kmalloc(100))) {
                        printk("malloc failed\n");
                } else {
                        printk("malloc returned %p\n", ptr);

                        for (int i = 0; i < 100; i++) {
                                ((uint8_t *)ptr)[i] = i;
                        }

                        for (int i = 0; i < 100; i++) {
                                if(((uint8_t *)ptr)[i] != i)
                                        printk("kmalloc error\n");
                        }
                }

                kfree(ptr);
        }

        /* Test less simple malloc */
        {
                uint8_t ** arrays = kcalloc(256, sizeof(uint8_t *));

                for (int i = 0; i < 256; i++) {
                        arrays[i] = kmalloc(100);

                        for (int j = 0; j < 100; j++) {
                                arrays[i][j] = i;
                        }
                }

                /* Test and free every-other array */
                for (int i = 0; i < 256; i++) {
                        for (int j = 0; j < 100; j++) {
                                if (arrays[i][j] != i)
                                        printk("kmalloc error\n");
                        }

                        if (i % 2 == 0)
                                kfree(arrays[i]);
                }

                /* Make some bigger and smaller */
                for (int i = 0; i < 256; i++) {
                        if (i % 2 == 0) {
                                if (i % 4 == 0) {
                                        arrays[i] = kmalloc(10);
                                } else {
                                        arrays[i] = kmalloc(200);
                                }
                        }

                        /* Write a new value */
                        for (int j = 0; j < 10; j++) {
                                arrays[i][j] = (i * 10) % 255;
                        }
                }

                for (int i = 0; i < 256; i++) {
                        for (int j = 0; j < 10; j++) {
                                if (arrays[i][j] != (i * 10) % 255)
                                        printk("kmalloc error!\n");
                        }

                        kfree(arrays[i]);
                }

                kfree(arrays);
        }

        /* Huge allocation (1 MB) */
        {
                uint64_t * ptr = kcalloc(131072, sizeof(uint64_t));

                for (int i = 0; i < 131072; i++) {
                        ptr[i] = i;
                }

                for (int i = 0; i < 131072; i++) {
                        if (ptr[i] != i)
                                printk("kmalloc error!\n");
                }

                kfree(ptr);
        }

        /* Unmask keyboard */
        IRQ_clear_mask(PIC_KEYBOARD);
        printk("Keyboard unmasked: ");

        while(1)
                asm("hlt");

        return;
}
