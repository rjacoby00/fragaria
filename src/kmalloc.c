/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/kmalloc.c
 *
 * Memory management, page mapping, and page fault handling
 * 
 */

#include <stddef.h>
#include <stdint.h>

#include "kmalloc.h"
#include "mm.h"
#include "printk.h"

void * bottom = NULL, * top = NULL;
struct malloc_header * head = NULL;

/**
 * print_header() - prints all header data to stderr for debugging
 * @hdr: Pointer to header to print
 *
 * @return: void
 */
static void print_header(struct malloc_header * hdr)
{
        printk("kmalloc header at %p\n", (void *)hdr);

        /* Print warning message if we're unaligned */
        if ((uintptr_t)hdr % MALLOC_ALIGNMENT)
                printk("    WARNING: HEADER UNALIGNED\n");

        /* Make sure we're not dereferencing NULL ptr */
        if (hdr) {
                printk("    next: %p\n", (void *)hdr->next);
                printk("    previous: %p\n", (void *)hdr->previous);
                printk("    size: %lu\n", hdr->size);
                printk("    status: %s\n", hdr->status?"ALLOCATED":"FREE");
                printk("    data start: %p\n", (void *)hdr->start);
        }

        return;
}

/**
 * kmalloc_init() - runs the first time malloc or calloc is called
 *
 * @return: 0 on success of intial allocation, nonzero on failure
 */
static int kmalloc_init()
{
        /* Try allocating one chunk */
        bottom = MMU_alloc_pages(MALLOC_CHUNK_SIZE / MM_PF_SIZE);
        printk("MALLOC: allocated %d pages\n", MALLOC_CHUNK_SIZE / MM_PF_SIZE);

        /* Check if allocation succeeded */
        if(bottom == MM_FRAME_EMPTY) {
                bottom = NULL;
                printk("KMALLOC: init failed\n");
                return -1;
        }

        /* Calculate the position of the end of our memory */
        top = (uint8_t *)bottom + MALLOC_CHUNK_SIZE;

        /* Start the head on a multiple of MALLOC_ALIGNMENT */
        if ((uintptr_t)bottom % MALLOC_ALIGNMENT) {
                head = (void *)((uintptr_t)bottom
                        + MALLOC_ALIGNMENT
                        - ((uintptr_t)bottom % MALLOC_ALIGNMENT));
        } else {
                head = bottom;
        }

        printk("MALLOC: bottom at %p\n", bottom);
        printk("MALLOC: top at %p\n", top);
        printk("MALLOC: chunk size: %d\n", MALLOC_CHUNK_SIZE);
        printk("MALLOC: hdr size: %lu\n", sizeof(struct malloc_header));
                
        /* Initialize the first block as free */
        head->next = NULL;
        head->previous = NULL;
        head->status = FREE;
        head->start = (void *)((uintptr_t)head + HEADER_ALIGNED_SIZE);
        head->size = (uint8_t *)top - (uint8_t *)head->start;

        printk("MALLOC: base header created:\n");
        print_header(head);

        return 0;
}

/**
 * get_block() - find the first free block that fits 
 * @size: Minmum size of block to find
 *
 * Return: Pointer to the header of found block
 */
static struct malloc_header * get_block(size_t size)
{
        struct malloc_header * current = head;

        printk("MALLOC: finding block\n");

        /* Traverse the list, stop on the last block */
        for(current = head; current->next; current = current->next) {
                printk("MALLOC: checking header\n");
                print_header(current);

                if(current->status == FREE && current->size >= size) {
                        /* We've found a block to put our data in */
                        printk("MALLOC: found block\n");

                        return current;
                }
        }

        if(current->status == FREE && current->size >= size) {
                /* The last block will fit our data */
                printk("MALLOC: found block\n");

                return current;
        }

        /* If we get here, there's no free block big enough, so move break */
        printk("MALLOC: no block large enough, moving break\n");
        print_header(current);

        /* get enough memory + some(64k) */
        if(MMU_alloc_pages((size - current->size + MALLOC_CHUNK_SIZE)
                        / MM_PF_SIZE) == MM_FRAME_EMPTY) {
                printk("MALLOC: failed to get more memory\n");

                return NULL;
        }

        /* Update top and current->size */
        top = (void *)((uintptr_t)top
                        + size
                        - current->size
                        + MALLOC_CHUNK_SIZE);

        current->size = size + MALLOC_CHUNK_SIZE;

        printk("MALLOC: new top at %p\n", top);
        printk("MALLOC: new top header:\n");
        print_header(current);

        return current;
}

/**
 * find_header() - find the header for the block that contains this pointer
 * @ptr: Pointer to somewhere in the memory block
 *
 * Return: Pointer to the header of that block, NULL on failure
 */
static struct malloc_header * find_header(void * ptr)
{
        struct malloc_header * current;

        printk("MALLOC: finding header for pointer %p\n", ptr);

        for(current = head; current; current = current->next) {
                /* Check if the current header's block contains ptr */
                if((uintptr_t)ptr >= (uintptr_t)current->start && 
                   (uintptr_t)ptr <= (uintptr_t)current->start
                        + (uintptr_t)current->size + 1)
                        return current;
        }

        return current;
}

/**
 * calloc() - allocate a block of memory and initialize to zeroes
 * @nmeb: Number of members to make space for
 * @size: Size of each member
 *
 * Return: void * Pointer to allocated memory, NULL on fail
 */
void * kcalloc(size_t nmeb, size_t size)
{
        void * ptr;
        int i;
       
        ptr = kmalloc(nmeb * size);

        if(!ptr)
                return NULL;

        for(i = 0; i < nmeb * size; i++)
                ((uint8_t *)ptr)[i] = 0;

        printk("MALLOC: calloc(%lu, %lu)\t\t=> (ptr=%p, size=%lu)\n", nmeb,
                size, (void *) ptr, nmeb * size);


        return ptr;
}

/**
 * kmalloc() - allocate a block of memory
 * @size: Number of bytes to allocate
 *
 * Return: void * Pointer to allocated memory, NULL on fail
 */
void * kmalloc(size_t size)
{
        struct malloc_header * current;

        if(!top) kmalloc_init();

        printk("MALLOC: malloc(%lu)\n", size);

        /* Find a canidate header */
        current = get_block(size);

        /* If get_block returns null, break move failed and we're out of mem */
        if(!current)
                return NULL;

        /* Mark block as allocated */
        current->status = ALLOCATED;

        /* Determine if it makes sense to put a new block (if we have space
         * for an aligned header and one block */
        if(current->size - size >= HEADER_ALIGNED_SIZE + MALLOC_ALIGNMENT) {
                /* Split off a free block */
                struct malloc_header * new_header;

                new_header = (void *)((uintptr_t)current
                                + HEADER_ALIGNED_SIZE
                                + size
                                + MALLOC_ALIGNMENT
                                - size % MALLOC_ALIGNMENT);

                /* Link into list of headers */
                new_header->next = current->next;
                current->next = new_header;
                new_header->previous = current;

                /* Set new header values */
                new_header->status = FREE;
                new_header->size = current->size
                                - HEADER_ALIGNED_SIZE
                                - size
                                - MALLOC_ALIGNMENT
                                + (size % MALLOC_ALIGNMENT);
                new_header->start = (void *)((uintptr_t)new_header
                                + HEADER_ALIGNED_SIZE);

                /* Update the size of the allocated block(make sure it's
                 * aligned to a alignment block) */
                current->size = size
                        + MALLOC_ALIGNMENT
                        - (size % MALLOC_ALIGNMENT);

                printk("MALLOC: current header:\n");
                print_header(current);

                printk("MALLOC: new header:\n");
                print_header(new_header);
        }

        printk("MALLOC: malloc(%lu)\t\t=> (ptr=%p, size=%lu)\n", size,
                (void *) current, current->size);

        return current->start;
}

/**
 * kfree() - free allocated memory
 * @ptr: Pointer to anywhere in the allocated block
 *
 * Note: ptr can point to anywhere from the orignal returned pointer from
 *       malloc/realloc, up to one past the end of the requested space(to
 *       account for incrementing the pointer).
 *
 * Return: void
 */
void kfree(void * ptr)
{
        struct malloc_header * current;

        /* Check NULL ptr */
        if(!ptr) {
                printk("MALLOC: free(NULL)\n");
                return;
        }

        /* Init library if not already done */
        if(!top)
                kmalloc_init();

        printk("MALLOC: free(%p)", ptr);

        /* Find the header that corresponds to the block requested */
        current = find_header(ptr);

        /* Just return if the provided pointer is invalid(and we haven't 
         * already segfault'd */
        if(!current)
                return;

        /* Mark block as free */
        current->status = FREE;

        /* Combine forwards */
        if(current->next && current->next->status == FREE) {
                current->size += current->next->size + HEADER_ALIGNED_SIZE;
                current->next = current->next->next;
        }

        /* Combine backwards */
        if(current->previous && current->previous->status == FREE) {
                current->previous->size += current->size + HEADER_ALIGNED_SIZE;
                current->previous->next = current->next;
                current = current->previous;
        }

        /* Check if we're on the last block and need to give mem back to
         * the operating system */
        /* TODO: fix page alignment */
        if(!current->next && current->size > MALLOC_CHUNK_SIZE) {
                printk("MALLOC: trying to return mem\n");

                /* sbrk(MALLOC_CHUNK_SIZE - current->size) - calculate how much
                 * to free and leave at least a chunk allocated */

                /* We should have the chunk size left over */
                current->size = MALLOC_CHUNK_SIZE;

                /* Update the top */
                top = (void *)((uintptr_t)top
                                + MALLOC_CHUNK_SIZE
                                - current->size);

                /* Set new break to top */
                MMU_free_page(top);

                printk("MALLOC: new top at %p\n", top);
                printk("MALLOC: new last block:\n");
                print_header(current);
        }

        return;
}

/**
 * krealloc() - resize malloc'd memory block
 * @ptr: Pointer to anywhere in the allocated block
 * @size: New size of block
 *
 * Return: void * Pointer to new location of allocated block
 */
void * krealloc(void * ptr, size_t size)
{
        struct malloc_header * current;

        /* Check size */
        if(size == 0) {
                kfree(ptr);
                return NULL;
        }

        /* Check NULL ptr */
        if(!ptr) {
                printk("MALLOC: realloc(NULL, %lu)\t=> (ptr=NULL, size=0)\n", 
                        size);
                
                return kmalloc(size);
        }

        /* If library hasn't been init, we wont find anything to realloc */
        if(!top)
                return NULL;

        /* Find the header that corresponds to the block requested */
        current = find_header(ptr);

        /* Return error if the provided pointer is invalid(and we haven't 
         * already segfault'd */
        if(!current)
                return NULL;

        /* If they asked for the same size, we don't have to do anything */
        if(size == current->size) {
                printk("MALLOC: realloc(%p, %lu)\t=> (ptr=%p, size=%lu)", ptr,
                        size, (void *)current, current->size);

                return current->start;
        }

        /* If the size is smaller, maybe put a new free block and try merge */
        if(size < current->size) {
                size_t newsize;

                /* Calculate our new smaller size */
                newsize = size
                        + MALLOC_ALIGNMENT
                        - (size % MALLOC_ALIGNMENT);

                /* If we have enough space, */
                if(current->size - newsize >
                                HEADER_ALIGNED_SIZE + MALLOC_ALIGNMENT) {
                        /* Make a new free block */


                        /* Split off a free block */
                        struct malloc_header * new_header;

                        new_header = (void *)((uintptr_t)current
                                        + HEADER_ALIGNED_SIZE
                                        + size
                                        + MALLOC_ALIGNMENT
                                        - size % MALLOC_ALIGNMENT);

                        /* Link into list of headers */
                        new_header->next = current->next;
                        current->next = new_header;
                        new_header->previous = current;

                        /* Set new header values */
                        new_header->status = FREE;
                        new_header->size = current->size
                                        - HEADER_ALIGNED_SIZE
                                        - size
                                        - MALLOC_ALIGNMENT
                                        + (size % MALLOC_ALIGNMENT);
                        new_header->start = (void *)((uintptr_t)new_header
                                        + HEADER_ALIGNED_SIZE);

                        /* Update the size of the allocated block(make sure it's
                         * aligned to a alignment block) */
                        current->size = size
                                + MALLOC_ALIGNMENT
                                - (size % MALLOC_ALIGNMENT);

                        /* Combine forwards */
                        if(new_header->next &&
                                        new_header->next->status == FREE) {
                                new_header->size += new_header->next->size
                                        + HEADER_ALIGNED_SIZE;

                                new_header->next = new_header->next->next;
                        }

                        /* Check if we're on the last block and need to give
                         * mem back to the operating system */
                        /* TODO fix header size page alignment */
                        if(!new_header->next &&
                                        new_header->size > MALLOC_CHUNK_SIZE) {
                                printk("MALLOC: trying to return mem\n");

                                /* We should have the chunk size left over */
                                new_header->size = MALLOC_CHUNK_SIZE;

                                /* Update the top */
                                top = (void *)((uintptr_t)top
                                                + MALLOC_CHUNK_SIZE
                                                - new_header->size);

                                /* Move the break to the new top */
                                MMU_free_page(top);

                                printk("MALLOC: new top at %p\n", top);
                                printk("MALLOC: new last block:\n");
                                print_header(new_header);
                        }

                        printk("MALLOC: shrunk header:\n");
                        print_header(current);

                        printk("MALLOC: new header:\n");
                        print_header(new_header);
                }

                printk("MALLOC: realloc(%p, %lu)\t=> (ptr=%p, size=%lu)", ptr,
                        size, (void *)current, current->size);

                
                /* Return the begnning of the shrunk data block */
                return current->start;
                
        }

        /* If the size is larger, find a new place */
        if(size > current->size) {
                /* Check if the next block is free and will fit */
                if(current->next &&
                   current->next->status == FREE &&
                   current->size + current->next->size + HEADER_ALIGNED_SIZE >=
                                size) {
                        /* We can just expand */
                        if(current->size + HEADER_ALIGNED_SIZE
                           + current->next->size - size >=
                           HEADER_ALIGNED_SIZE + MALLOC_ALIGNMENT) {
                                /* We can fit a free block */
                                struct malloc_header * new_header;

                                new_header = (void *)((uintptr_t)current
                                        + HEADER_ALIGNED_SIZE
                                        + size
                                        + MALLOC_ALIGNMENT
                                        - size % MALLOC_ALIGNMENT);

                                /* Link into list of headers */
                                new_header->next = current->next;
                                current->next = new_header;
                                new_header->previous = current;

                                /* Set new header values */
                                new_header->status = FREE;
                                new_header->size = current->size
                                                - HEADER_ALIGNED_SIZE
                                                - size
                                                - MALLOC_ALIGNMENT
                                                + (size % MALLOC_ALIGNMENT);
                                new_header->start = (void *)((uintptr_t)
                                                new_header
                                                + HEADER_ALIGNED_SIZE);

                                 /* Update the size of the allocated block.
                                  * Make sure it's aligned to a block */
                                current->size = size
                                        + MALLOC_ALIGNMENT
                                        - (size % MALLOC_ALIGNMENT);

                                printk("MALLOC: expanded header:\n");
                                print_header(current);

                                printk("MALLOC: new header:\n");
                                print_header(new_header);
                        } else {
                                /* We cannot fit a free block */

                                current->size += current->next->size
                                        + HEADER_ALIGNED_SIZE;
                                current->next = current->next->next;
                        }

                        printk("MALLOC: realloc(%p, %lu)\t=> "\
                                "(ptr=%p, size=%lu)", ptr, size,
                                (void *)current, current->size);

                        return current->start;

                } else {
                        /* If we can't expand, look for a new place */

                        /* TODO: this is not *quite* the most efficient way to
                         * do this, but it's close and its what I have time
                         * for.  It would be better to mark the current block 
                         * as free so it's considered for copy if the previous 
                         * block is free.
                         */

                        void * new_mem;
                        int i;

                        printk("MALLOC: copying to bigger block\n");

                        /* Get a block big enough */
                        new_mem = kmalloc(size);

                        if(!new_mem)
                                return NULL;

                        /* Copy over all of the data */
                        for(i = 0; i < current->size; i++)
                                ((uint8_t *)new_mem)[i] = 
                                        ((uint8_t *)current->start)[i];

                        kfree(current);

                        printk("MALLOC: realloc(%p, %lu)\t=> "\
                                "(ptr=%p, size=%lu)", ptr, size,
                                (void *)new_mem, size);

                        return new_mem;
                }
        }

        return NULL;
}
