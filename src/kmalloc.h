/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/kmalloc.h
 *
 * Header for kernel heap allocator
 * 
 */

#ifndef KMALLOC_H
#define KMALLOC_H                               1

#include <stddef.h>
#include <stdint.h>

/* Debug string should be standard line width + newline + NULL */
#define MALLOC_CHUNK_SIZE (1<<16)
#define MALLOC_ALIGNMENT 16

#define HEADER_ALIGNED_SIZE \
(sizeof(struct malloc_header) + MALLOC_ALIGNMENT -\
(sizeof(struct malloc_header) % MALLOC_ALIGNMENT))

#define FREE 0
#define ALLOCATED 1

void * kcalloc(size_t nmeb, size_t size);
void * kmalloc(size_t size);
void kfree(void * ptr);
void * krealloc(void * ptr, size_t size);

/* struct malloc_header requiremnts:
   - Next header: NULL on end
   - Previous header: NULL on start
   - Size(in case we don't have another header and to simplify calculations)
   - Status(uint8)
           - 0: free
           - 1: allocated
   - Pointer to start of data
*/

/**
 * struct malloc_header - header for malloc blocks
 * @next: Pointer to the next header, NULL if does not exist
 * @previous: Point to the previous header, NULL if does not exist
 * @size: Size of allocated data block(as requested by user)
 * @status: 0 if free, 1 if allocated
 * @start: Pointer to start of data
 */
struct malloc_header {
        struct malloc_header * next;
        struct malloc_header * previous;
        size_t size;
        uint8_t status;
        void * start;
};

#endif /* #ifndef KMALLOC_H */
