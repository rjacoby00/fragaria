/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/port_io.h
 *
 * Header for port-based IO inb/outb wrapper functions
 */

#ifndef PORT_IO_H
#define PORT_IO_H                               1

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val)
{
        asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));

        return ret;
}

#endif /* #ifndef PORT_IO_H */
