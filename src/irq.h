/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/irq.h
 *
 * Header for interrupt handling setup, functions, and macros
 * 
 */

#ifndef IRQ_H
#define IRQ_H                                   1

#include <stdint.h>

#define CLI asm("cli")
#define STI asm("sti")

#define IFLAGS_IF                               0x0200

static inline uint8_t interrupts_enabled(void)
{
        unsigned long flags;

        asm volatile("pushf; pop %0"
                : "=rm" (flags)
                :
                : "memory");

        return flags & IFLAGS_IF;
}

#define NUM_IRQS                                256

#define PIC_1                                   0x20    /* Main PIC 1-7 */
#define PIC_2                                   0xA0    /* Second PIC 8-15*/
#define PIC_1_COMMAND                           PIC_1
#define PIC_1_DATA                              (PIC_1+1)
#define PIC_2_COMMAND                           PIC_2
#define PIC_2_DATA                              (PIC_2+1)

#define PIC_PROG_TIMER                          0
#define PIC_KEYBOARD                            1
#define PIC_CASCADE                             2       /* Used internally */
#define PIC_COM2                                3
#define PIC_COM1                                4
#define PIC_LPT2                                5
#define PIC_FLOPPY                              6
#define PIC_LP1                                 7
#define PIC_RTC                                 8
#define PIC_PS2_MOUSE                           12
#define PIC_FPU                                 13
#define PIC_ATA_PRIMARY                         14
#define PIC_ATA_SECONDARY                       15


#define PIC_EOI                                 0x20    /* End of interrupt */

#define ICW1_ICW4                               0x01    /* ICW4 present */
#define ICW1_SINGLE                             0x02    /* Single mode */
#define ICW1_INTERVAL4                          0x04    /* Call address int */
#define ICW1_LEVEL                              0x08    /* Level mode */
#define ICW1_INIT                               0x10    /* Start init cmd */

#define ICW4_8086                               0x01    /* 8086 mode */
#define ICW4_AUTO                               0x02    /* Auto EOI */
#define ICW4_BUF_SECOND                         0x08    /* Buff mode->PIC2 */
#define ICW4_BUF_FIRST                          0x0C    /* Buff mode->PIC1 */
#define ICW4_SFNM                               0x10    /* Fully nested */

#define INT_TYPE_INTERRUPT                      0x0E
#define INT_TYPE_TRAP                           0x0F

struct idt_entry {
        uint16_t target_offset0;
        uint16_t target_selector;
        uint16_t ist:3;
        uint16_t reserved0:5;
        uint16_t type:4;
        uint16_t mbz:1;
        uint16_t dpl:2;
        uint16_t present:1;
        uint16_t target_offset1;
        uint32_t target_offset2;
        uint32_t reserved1;
} __attribute__((packed));

struct irq_stack_frame {
        uint32_t error_code;
        uint32_t reserved0;
        uint64_t rip;
        uint16_t cs;
        uint16_t reserved1;
        uint32_t reserved2;
        uint64_t rflags;
        uint64_t rsp;
        uint16_t ss;
        uint16_t reserved3;
        uint32_t reserved4;
} __attribute__((packed));

extern void * irq_entry;                        /* asm irq handler entry */
void irq_c_handler(int, uint32_t, void *, struct irq_stack_frame *);

typedef void (*irq_handler_t)(int irq, uint32_t error, void * cr2, void * arg);

void IRQ_init(void);
void IRQ_set_mask(int);
void IRQ_clear_mask(int);
int IRQ_get_mask(int);
void IRQ_end_of_interrupt(int);
void IRQ_set_handler(int, irq_handler_t, void *);

extern void irq0x00(void);
extern void irq0x01(void);
extern void irq0x02(void);
extern void irq0x03(void);
extern void irq0x04(void);
extern void irq0x05(void);
extern void irq0x06(void);
extern void irq0x07(void);
extern void irq0x08(void);
extern void irq0x09(void);
extern void irq0x0A(void);
extern void irq0x0B(void);
extern void irq0x0C(void);
extern void irq0x0D(void);
extern void irq0x0E(void);
extern void irq0x0F(void);
extern void irq0x10(void);
extern void irq0x11(void);
extern void irq0x12(void);
extern void irq0x13(void);
extern void irq0x14(void);
extern void irq0x15(void);
extern void irq0x16(void);
extern void irq0x17(void);
extern void irq0x18(void);
extern void irq0x19(void);
extern void irq0x1A(void);
extern void irq0x1B(void);
extern void irq0x1C(void);
extern void irq0x1D(void);
extern void irq0x1E(void);
extern void irq0x1F(void);
extern void irq0x20(void);
extern void irq0x21(void);
extern void irq0x22(void);
extern void irq0x23(void);
extern void irq0x24(void);
extern void irq0x25(void);
extern void irq0x26(void);
extern void irq0x27(void);
extern void irq0x28(void);
extern void irq0x29(void);
extern void irq0x2A(void);
extern void irq0x2B(void);
extern void irq0x2C(void);
extern void irq0x2D(void);
extern void irq0x2E(void);
extern void irq0x2F(void);
extern void irq0x30(void);
extern void irq0x31(void);
extern void irq0x32(void);
extern void irq0x33(void);
extern void irq0x34(void);
extern void irq0x35(void);
extern void irq0x36(void);
extern void irq0x37(void);
extern void irq0x38(void);
extern void irq0x39(void);
extern void irq0x3A(void);
extern void irq0x3B(void);
extern void irq0x3C(void);
extern void irq0x3D(void);
extern void irq0x3E(void);
extern void irq0x3F(void);
extern void irq0x40(void);
extern void irq0x41(void);
extern void irq0x42(void);
extern void irq0x43(void);
extern void irq0x44(void);
extern void irq0x45(void);
extern void irq0x46(void);
extern void irq0x47(void);
extern void irq0x48(void);
extern void irq0x49(void);
extern void irq0x4A(void);
extern void irq0x4B(void);
extern void irq0x4C(void);
extern void irq0x4D(void);
extern void irq0x4E(void);
extern void irq0x4F(void);
extern void irq0x50(void);
extern void irq0x51(void);
extern void irq0x52(void);
extern void irq0x53(void);
extern void irq0x54(void);
extern void irq0x55(void);
extern void irq0x56(void);
extern void irq0x57(void);
extern void irq0x58(void);
extern void irq0x59(void);
extern void irq0x5A(void);
extern void irq0x5B(void);
extern void irq0x5C(void);
extern void irq0x5D(void);
extern void irq0x5E(void);
extern void irq0x5F(void);
extern void irq0x60(void);
extern void irq0x61(void);
extern void irq0x62(void);
extern void irq0x63(void);
extern void irq0x64(void);
extern void irq0x65(void);
extern void irq0x66(void);
extern void irq0x67(void);
extern void irq0x68(void);
extern void irq0x69(void);
extern void irq0x6A(void);
extern void irq0x6B(void);
extern void irq0x6C(void);
extern void irq0x6D(void);
extern void irq0x6E(void);
extern void irq0x6F(void);
extern void irq0x70(void);
extern void irq0x71(void);
extern void irq0x72(void);
extern void irq0x73(void);
extern void irq0x74(void);
extern void irq0x75(void);
extern void irq0x76(void);
extern void irq0x77(void);
extern void irq0x78(void);
extern void irq0x79(void);
extern void irq0x7A(void);
extern void irq0x7B(void);
extern void irq0x7C(void);
extern void irq0x7D(void);
extern void irq0x7E(void);
extern void irq0x7F(void);
extern void irq0x80(void);
extern void irq0x81(void);
extern void irq0x82(void);
extern void irq0x83(void);
extern void irq0x84(void);
extern void irq0x85(void);
extern void irq0x86(void);
extern void irq0x87(void);
extern void irq0x88(void);
extern void irq0x89(void);
extern void irq0x8A(void);
extern void irq0x8B(void);
extern void irq0x8C(void);
extern void irq0x8D(void);
extern void irq0x8E(void);
extern void irq0x8F(void);
extern void irq0x90(void);
extern void irq0x91(void);
extern void irq0x92(void);
extern void irq0x93(void);
extern void irq0x94(void);
extern void irq0x95(void);
extern void irq0x96(void);
extern void irq0x97(void);
extern void irq0x98(void);
extern void irq0x99(void);
extern void irq0x9A(void);
extern void irq0x9B(void);
extern void irq0x9C(void);
extern void irq0x9D(void);
extern void irq0x9E(void);
extern void irq0x9F(void);
extern void irq0xA0(void);
extern void irq0xA1(void);
extern void irq0xA2(void);
extern void irq0xA3(void);
extern void irq0xA4(void);
extern void irq0xA5(void);
extern void irq0xA6(void);
extern void irq0xA7(void);
extern void irq0xA8(void);
extern void irq0xA9(void);
extern void irq0xAA(void);
extern void irq0xAB(void);
extern void irq0xAC(void);
extern void irq0xAD(void);
extern void irq0xAE(void);
extern void irq0xAF(void);
extern void irq0xB0(void);
extern void irq0xB1(void);
extern void irq0xB2(void);
extern void irq0xB3(void);
extern void irq0xB4(void);
extern void irq0xB5(void);
extern void irq0xB6(void);
extern void irq0xB7(void);
extern void irq0xB8(void);
extern void irq0xB9(void);
extern void irq0xBA(void);
extern void irq0xBB(void);
extern void irq0xBC(void);
extern void irq0xBD(void);
extern void irq0xBE(void);
extern void irq0xBF(void);
extern void irq0xC0(void);
extern void irq0xC1(void);
extern void irq0xC2(void);
extern void irq0xC3(void);
extern void irq0xC4(void);
extern void irq0xC5(void);
extern void irq0xC6(void);
extern void irq0xC7(void);
extern void irq0xC8(void);
extern void irq0xC9(void);
extern void irq0xCA(void);
extern void irq0xCB(void);
extern void irq0xCC(void);
extern void irq0xCD(void);
extern void irq0xCE(void);
extern void irq0xCF(void);
extern void irq0xD0(void);
extern void irq0xD1(void);
extern void irq0xD2(void);
extern void irq0xD3(void);
extern void irq0xD4(void);
extern void irq0xD5(void);
extern void irq0xD6(void);
extern void irq0xD7(void);
extern void irq0xD8(void);
extern void irq0xD9(void);
extern void irq0xDA(void);
extern void irq0xDB(void);
extern void irq0xDC(void);
extern void irq0xDD(void);
extern void irq0xDE(void);
extern void irq0xDF(void);
extern void irq0xE0(void);
extern void irq0xE1(void);
extern void irq0xE2(void);
extern void irq0xE3(void);
extern void irq0xE4(void);
extern void irq0xE5(void);
extern void irq0xE6(void);
extern void irq0xE7(void);
extern void irq0xE8(void);
extern void irq0xE9(void);
extern void irq0xEA(void);
extern void irq0xEB(void);
extern void irq0xEC(void);
extern void irq0xED(void);
extern void irq0xEE(void);
extern void irq0xEF(void);
extern void irq0xF0(void);
extern void irq0xF1(void);
extern void irq0xF2(void);
extern void irq0xF3(void);
extern void irq0xF4(void);
extern void irq0xF5(void);
extern void irq0xF6(void);
extern void irq0xF7(void);
extern void irq0xF8(void);
extern void irq0xF9(void);
extern void irq0xFA(void);
extern void irq0xFB(void);
extern void irq0xFC(void);
extern void irq0xFD(void);
extern void irq0xFE(void);
extern void irq0xFF(void);

#endif /* #ifndef IRQ_H */
