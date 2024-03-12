/* 
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/gdt.h
 *
 * GDT C structues
 * 
 */

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define TSS_64                                  0b1001

#define DF_IST_INDEX                            0
#define PF_IST_INDEX                            1
#define GP_IST_INDEX                            2

extern uint64_t pf_stack_top;
extern uint64_t pf_stack_bottom;
extern uint64_t df_stack_top;
extern uint64_t df_stack_bottom;
extern uint64_t gp_stack_top;
extern uint64_t gp_stack_bottom;

struct segment_descriptor {
        uint16_t limit0;
        uint16_t base0;
        uint8_t base1;
        uint8_t accessed:1;
        uint8_t rw:1;
        uint8_t conforming:1;
        uint8_t executable:1;
        uint8_t s_type:1;
        uint8_t dpl:2;
        uint8_t present:1;
        uint8_t limit1:4;
        uint8_t reserved:1;
        uint8_t long_mode:1;
        uint8_t size:1;
        uint8_t granularity:1;
        uint8_t base2;
} __attribute__((packed));

struct tss_descriptor {
        uint16_t segment_limit0;
        uint16_t base_address0;
        uint8_t base_address1;
        uint8_t type:4;
        uint8_t mbz0:1;
        uint8_t dpl:2;
        uint8_t present:1;
        uint8_t segment_limit1:4;
        uint8_t avl:1;
        uint8_t reserved0:2;
        uint8_t g:1;
        uint8_t base_address2;
        uint32_t base_address3;
        uint8_t reserved1;
        uint8_t mbz1:5;
        uint8_t reserved2:3;
        uint16_t reserved3;
} __attribute__((packed));

struct tss {
        uint32_t reserved0;
        uint64_t rsp[3];
        uint64_t reserved1;
        uint64_t ist[7];
        uint64_t reserved2;
        uint16_t reserved3;
        uint16_t io_map_base;
} __attribute__((packed));

void GDT_init(void);

#endif /* #ifndef GDT_H */