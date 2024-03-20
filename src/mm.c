/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/mm.c
 *
 * Memory management, page mapping, and page fault handling
 * 
 */

#include <stddef.h>
#include <stdint.h>

#include "mm.h"
#include "multiboot.h"
#include "printk.h"

/* Make space to store a static ammount of RAM regions from multiboot2.
 * We should only get 2 or maybe 3, there's space for 5. */
struct MM_unused unused[MM_RAM_REGIONS];

struct MM_frame_list used;
struct MM_frame_list freed;

/**
 * MM_frame_list_contains() - Search frame list for a certain address 
 * @list to search 
 * @addr to search for
 * 
 * @return void ** pointer to page frame address found; NULL if not found
 */
static void ** MM_frame_list_contains(struct MM_frame_list * list, void * addr)
{
        for (struct MM_frame_list * c = list; c && c != MM_FRAME_EMPTY;
                        c = c->next) {
                for (int i = 0; i < MM_FRAME_LIST_CAPACITY; i++) {
                        if (c->addr[i] == addr)
                                return c->addr + i;
                }
        }

        return NULL;
}

/**
 * MM_frame_list_add() - Add a frame address to a list 
 * @list to add to
 * @addr page frame address to add
 * 
 * @return void ** pointer to page frame address in list; NULL on failure
 */
static void ** MM_frame_list_add(struct MM_frame_list * list, void * addr)
{
        void ** ret;

        /* First make sure we're not putting a duplicate */
        if ((ret = MM_frame_list_contains(list, addr))) {
                printk("Address %p already in table\n", addr);
                return ret;
        }

        /* Find the first list with space */
        while (list->next && list->next != MM_FRAME_EMPTY
                        && list->num == MM_FRAME_LIST_CAPACITY )
                list = list->next;



        /* Try find the first avaialable spot */
        for (int i = 0; i < MM_FRAME_LIST_CAPACITY; i++) {
                if (list->addr[i] == MM_FRAME_EMPTY) {
                        list->addr[i] = addr;
                        list->num++;

                        /* If we're the last list and almost full,
                         * allocate a new one */
                        if (list->num + 5 >= MM_FRAME_LIST_CAPACITY
                                        && !list->next
                                        && list->next != MM_FRAME_EMPTY) {
                                list->next = MM_FRAME_EMPTY;
                                list->next = MM_pf_alloc();

                                list->next->next = NULL;
                                list->next->num = 0;

                                /* Mark all entries as empty */
                                for (int j = 0; j < MM_FRAME_LIST_CAPACITY;
                                                j++) {
                                        list->next->addr[j] = MM_FRAME_EMPTY;
                                }
                        }

                        return list->addr + i;
                }
        }

        /* If list has no space but says it has space; fix and return null.
         * Theoretically we'll never get here... */
        printk("List capcaity off by one!! FATAL\n");
        list->num = MM_FRAME_LIST_CAPACITY;

        return NULL;
}

/**
 * MM_frame_list_remove() - Remove a page frame address from a list 
 * @list to remove from
 * @addr page frame address to search and remove
 * 
 * @return void ** address of just removed entry; NULL on failure 
 */
static void ** MM_frame_list_remove(struct MM_frame_list * list, void * addr)
{
        for (struct MM_frame_list * c = list; c && c != MM_FRAME_EMPTY;
                        c = c->next) {
                for (int i = 0; i < MM_FRAME_LIST_CAPACITY; i++) {
                        if (c->addr[i] == addr) {
                                c->addr[i] = MM_FRAME_EMPTY;
                                c->num--;

                                return c->addr + i;
                        }
                }
        }

        return MM_FRAME_EMPTY;
}

/**
 * parse_elf() - Read multiboot2 ELF section headers to find already used memory 
 * @elf_symbols pointer to ELF section headers entry
 * 
 */
static void parse_elf(struct multiboot_elf_symbols * elf_symbols)
{
        for (int i = 0; i < elf_symbols->num; i++) {
                printk("    Section type %d, address %lx, size %ld\n",
                        elf_symbols->headers[i].sh_type,
                        elf_symbols->headers[i].sh_addr,
                        elf_symbols->headers[i].sh_size);

                /* Add any pages this section is in to the used table */
                for (int j = 0; j < elf_symbols->headers[i].sh_size;
                                j += MM_PF_SIZE) {
                        MM_frame_list_add(&used, (void *)((~(MM_PF_SIZE - 1))
                                & (elf_symbols->headers[i].sh_addr + j)));
                }
        }
        return;
}

/**
 * parse_mem_map() - Read multiboot2 memory map and record available mem 
 * @mm pointer to memory map entry
 * 
 */
static void parse_mem_map(struct multiboot_mem_map * mm)
{
        static int n = 0;

        for (int i = 0; i < (mm->header.size - sizeof(struct multiboot_mem_map))
                        / sizeof(struct multiboot_mm_entry); i++) {
                if (mm->entries[i].type == MULTIBOOT_MM_TYPE_RAM) {
                        if (n >= MM_RAM_REGIONS)
                                break;

                        printk("RAM region at 0x%lx, %ld bytes\n",
                                mm->entries[i].base_addr,
                                mm->entries[i].length);

                        unused[n].addr = (void *)mm->entries[i].base_addr;
                        unused[n].current = unused[n].addr;
                        unused[n].size = mm->entries[i].length;

                        n++;
                }
        }

        return;
}

/**
 * MM_init() - Initialized memory mangement structures 
 * 
 */
void MM_init(struct multiboot_table_header * multiboot)
{
        /* Make sure all sizes are zero as that's how we track how many regions
         * actually got found once we read them in */
        for (int i = 0; i < MM_RAM_REGIONS; i++)
                unused[i].size = 0;

        used.next = NULL;
        used.num = 0;

        freed.next = NULL;
        freed.num = 0;

        /* Mark all entries in used and freed list as empty */
        for (int i = 0; i < MM_FRAME_LIST_CAPACITY; i++) {
                used.addr[i] = MM_FRAME_EMPTY;
                freed.addr[i] = MM_FRAME_EMPTY;
        }

        printk("Found multiboot table at: %p\n", multiboot);
        printk("    multiboot table length: %d bytes\n", multiboot->total_size);

        /* Mark multiboot2 table as used */
        for(int i = 0; i < multiboot->total_size; i -= MM_PF_SIZE) {
                MM_frame_list_add(&used, ((uint8_t *)multiboot) + i);
        }

        /* Read the multiboot2 table */
        for (int i = 8; i < multiboot->total_size;) {
                struct multiboot_header * current = 
                        (struct multiboot_header *)((uint8_t *)multiboot + i);

                /* Ignore anything that's not ELF table, mm, or terminator */
                if(current->type == 0) {
                        break;
                } else if (current->type == MULTIBOOT_ELF_SYMBOLS) {
                        /* Once we find ELF table, map all sections as used */
                        parse_elf((struct multiboot_elf_symbols *)current);
                } else if (current->type == MULTIBOOT_MEM_MAP) {
                        /* Once we find mem map, add segments to unused */
                        parse_mem_map((struct multiboot_mem_map *)current);

                }

                i += (current->size + 7) & 0xFFFFFFF8;
        }

        /* Read in RAM regions */
        return;
}

/**
 * MM_pf_alloc() - Allocate a page
 * 
 * Check for previously freed page frames first; if non exist get a new one from
 * the unused chunk.  When we querry the unused chunk make sure page is not
 * already in use by kernel. 
 * 
 * @return void * address of page frame; MM_FRAME_EMPTY on failure
 */
void * MM_pf_alloc()
{
        void * attempt;
        int n = 0;

        for (struct MM_frame_list * c = &freed; c; c = c->next) {
                /* Don't bother looking for free frames in an empty free list */
                if (c->num == 0)
                        continue;

                /* If we find a freed frame, reuse it */
                for (int i = 0; i < MM_FRAME_LIST_CAPACITY; i++) {
                        if (c->addr[i] != MM_FRAME_EMPTY) {
                                attempt = c->addr[i];
                                c->addr[i] = MM_FRAME_EMPTY;
                                c->num--;

                                MM_frame_list_add(&used, attempt);

                                return attempt;
                        }
                }
        }

        /* If there are no previously freed frames, find one from unused */
        for(attempt = unused[n].current; n < MM_RAM_REGIONS;
                        attempt += MM_PF_SIZE) {
                /* If we've reached the end of this unused block, reset */
                if (attempt - unused[n].addr + MM_PF_SIZE > unused[n].size) {
                        n++;
                        attempt = unused[n].current - MM_PF_SIZE;
                        continue;
                }

                /* If we haven't used this block yet, give it and update top */
                if(!MM_frame_list_contains(&used, attempt)) {
                        MM_frame_list_add(&used, attempt);

                        /* Only update top if we're going forward(everything
                         * behind unused[n].current has been allocated) */
                        if (unused[n].current < attempt + MM_PF_SIZE)
                                unused[n].current = attempt + MM_PF_SIZE;

                        return attempt;
                }
        }

        /* If we reach here, all frames have been allocated; we're out of RAM */
        return MM_FRAME_EMPTY;
}

/**
 * MM_pf_free() - Remove a page frame from used list and add to free list 
 * @pf address in page frame to free (gets rounded down to nearest page)
 * 
 */
void MM_pf_free(void * pf)
{
        pf = (void *)((uint64_t)pf & ~(MM_PF_SIZE - 1));

        /* If given page was allocated, add it to free list*/
        if (MM_frame_list_remove(&used, pf) != MM_FRAME_EMPTY) {
                MM_frame_list_add(&freed, pf);
        } else {
                printk("MM_pf_free() called on unallocated address %p!\n", pf);
        }

        return;
}
