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

void MM_init(struct multiboot_table_header *);
void * MM_pf_alloc(void);
void MM_pf_free(void *);

#endif /* #ifndef MM_H */
