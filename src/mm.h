/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/mm.h
 *
 * Header for basic memory management and page fault handling
 * 
 */

#ifndef MM_H
#define MM_H                                    1

#include <stdint.h>

#include "multiboot.h"

#define MM_RAM_REGIONS                          5
#define MM_PF_SIZE                              4096

/*
 * Virtual memory layout note
 *
 * Base address   Use
 * 0x000000000000 Physical Page Frame Map (limits us to 512 GB physical RAM)
 * 0x008000000000 Kernel heap base - PML4E slot 1
 * Heap and stack growth - PML4E slots 3-31
 * 0x0F0000000000 Base of kernel stack space (bottom of first 512 GB of stacks)
 * 0x100000000000 Base of user space - not used yet - PML4E slot 32
 */

/**
 * struct MM_unused
 * Store RAM regions returned by multiboot2, ready to be allocated
 * 
 * @addr base address
 * @size size of RAM region
 * @current pointer to next physical address attempt allocation
 * 
 */
struct MM_unused {
        void * addr;
        uint64_t size;
        void * current;
};

/* This is an invalid 64-bit address(not 48-bit sign extended), so use to mark
 * entries in the allocated and free lists that got removed */
#define MM_FRAME_EMPTY                          (void *)(0xFF00000000000000)
#define MM_FRAME_LIST_CAPACITY                  510

/**
 * struct MM_frame_list
 * Store a number page frame addresses in one struct; should cleanly fill a page
 * frame to allow for easy dynamic allocation without virtual mem or malloc.
 * 
 */
struct MM_frame_list {
        struct MM_frame_list * next;
        uint64_t num;
        void * addr[MM_FRAME_LIST_CAPACITY];
};

/*
 * Page table structs
 */

#define MM_ADDR_MASK                            (0x000FFFFFFFFFF000)
#define PL0_MASK                                (0x000000000FFF)
#define PL1_MASK                                (0x0000001FF000)
#define PL2_MASK                                (0x00003FE00000)
#define PL3_MASK                                (0x007FC0000000)
#define PL4_MASK                                (0xFF8000000000)

#define PT_TO_ALLOC                             1

/**
 * struct cr3 
 * Format of contents of CR3 register, points to PML4
 * 
 */
struct cr3 {
        union {
                uint64_t address;
                struct {
                        uint8_t reserved0:3;
                        uint8_t pwt:1;
                        uint8_t pcd:1;
                } __attribute__((packed));
        };
} __attribute__((packed));

/**
 * struct pml4
 * Format of Page Map Level 4 Table(PML4); is close enough to PDP and PD tables
 * to be reused; only the behaviour of mbz changes to have certain bits ignored. 
 * 
 */
struct pml4 {
        union {
                uint64_t address;
                struct {
                        uint16_t present:1;
                        uint16_t rw:1;
                        uint16_t us:1;
                        uint16_t pwt:1;
                        uint16_t pcd:1;
                        uint16_t a:1;
                        uint16_t ign:1;
                        uint16_t mbz:2;
                        uint16_t avl:3;
                        uint16_t addr0:4;
                        uint32_t addr1;
                        uint16_t addr2:4;
                        uint16_t available:11;
                        uint16_t nx:1;
                } __attribute__((packed));
        };
} __attribute__((packed));

/**
 * struct pt
 * Format of page table entry; close to PML4 format but slightly different low
 * bits 
 * 
 */
struct pt {
        union {
                uint64_t address;
                struct {
                        uint16_t present:1;
                        uint16_t rw:1;
                        uint16_t us:1;
                        uint16_t pwt:1;
                        uint16_t pcd:1;
                        uint16_t a:1;
                        uint16_t d:1;
                        uint16_t pat:1;
                        uint16_t g:1;
                        uint16_t avl:3;
                        uint16_t addr0:4;
                        uint32_t addr1;
                        uint16_t addr2:4;
                        uint16_t available:11;
                        uint16_t nx:1;
                } __attribute__((packed));
        };
} __attribute__((packed));

/*
 * Page frame allocator functions
 */

void MM_init(struct multiboot_table_header *);
void * MM_pf_alloc(void);
void MM_pf_free(void *);

/* 
 * Virtual page allocator functions
 */

/* Reuse PML4 table already set up in boot.asm (CR3 already points there!)*/
extern struct pml4 p4_table[512];

void * MMU_alloc_page(void);
void * MMU_alloc_pages(int);
void MMU_free_page(void *);

#endif /* #ifndef MM_H */
