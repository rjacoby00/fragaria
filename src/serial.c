/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/serial.c
 *
 * Serial interface functions
 * 
 */

#include <stddef.h>
#include <stdint.h>

#include "irq.h"
#include "port_io.h"
#include "serial.h"

static uint8_t tx_fifo_size;
static char tx_buff[SERIAL_TX_BUFF_LEN];
static char * produce = NULL, * consume = NULL;

static int tx_reg_empty()
{
        return inb(SERIAL_IO_COM1 + SERIAL_LINE_STATUS_REG)
                & SERIAL_TX_BUFF_EMPTY;
}

/**
 * serial_pic_handle() - Consume from circular TX buffer
 * 
 * Context: usually in ISR but is called from outside ISR to restart TX;
 *      interrupts will always be dissabled when this function is called.
 * 
 * Algorithm:
 * check if we have data to write
 *      no->exit
 * check if tx reg is empty 
 *      yes->write up to 1 byte to transmitter
 * 
 * we don't need to check IIR?
 * 
 */
void serial_pic_handle()
{
        /* Check if we have data to write */
        if (produce == consume)
                return;
        
        /* If we do, check if transmitter is ready */
        if (tx_reg_empty()) {
                outb(SERIAL_IO_COM1, *consume);

                /* Increment consume pointer */
                consume++;
                if(consume >= tx_buff + SERIAL_TX_BUFF_LEN)
                        consume = tx_buff;
        }
        
        return;
}

/**
 * SER_init() - Init serial COM1
 * 
 * Context: not ISR; must be called after IRQ/PIC setup
 * 
 */
void SER_init()
{
        /* Disable interrupts */
        outb(SERIAL_IO_COM1 + SERIAL_INT_REG, 0x00);
        
        /* Swap to DLAB */
        outb(SERIAL_IO_COM1 + SERIAL_LINE_CONTROL_REG, SERIAL_DLAB);
        /* Set DLAB(lets us set clock divisor) 115200 / 9600 = 12 = 0x000C */
        outb(SERIAL_IO_COM1 + SERIAL_DVD_LSB_REG, 0x0C);
        outb(SERIAL_IO_COM1 + SERIAL_DVD_MSB_REG, 0x00);

        /* Set line control (unsets DLAB control) */
        outb(SERIAL_IO_COM1 + SERIAL_LINE_CONTROL_REG,
                (SERIAL_8_BIT_CHAR | SERIAL_PARITY_NONE) & ~SERIAL_STOP_BITS);

        /* Enable FIFO, clear, set 14 byte threshold */
        outb(SERIAL_IO_COM1 + SERIAL_FIFO_CONTROL_REG,
                SERIAL_ENABLE_FIFO | SERIAL_CLEAR_RX_FIFO |
                SERIAL_CLEAR_TX_FIFO | SERIAL_FIFO_14B);
        tx_fifo_size = 14;

        /* Enable IRQs, set RTS/DSR */
        outb(SERIAL_IO_COM1 + SERIAL_MODEM_CONTROL_REG,
                SERIAL_DTR | SERIAL_RTS | SERIAL_OUT2);
        
        /* Test serial chip in loopback mode */
        outb(SERIAL_IO_COM1 + SERIAL_MODEM_CONTROL_REG,
                SERIAL_LOOP | SERIAL_RTS | SERIAL_OUT1);
        outb(SERIAL_IO_COM1, 0xAE);     /* Write a test byte */
        if(inb(SERIAL_IO_COM1) != 0xAE) {
                while(1)
                        asm("hlt");
        }

        /* Set to normal operation */
        outb(SERIAL_IO_COM1 + SERIAL_MODEM_CONTROL_REG,
                SERIAL_DTR | SERIAL_RTS | SERIAL_OUT1 | SERIAL_OUT2);

        /* Enable interrupts */
        produce = tx_buff;
        consume = tx_buff;
        outb(SERIAL_IO_COM1 + SERIAL_INT_REG, SERIAL_INT_TX_EMPTY);
        IRQ_clear_mask(PIC_COM1);

        return;
}

/**
 * SER_write() - write to serial circular buffer 
 * @buff pointer to bytes to write
 * @len number of bytes to write
 * 
 * Context: ISR safe
 * 
 * TODO: define different error codes for all failures
 * 
 * @return number of bytes written on success, -1 on failure
 */
int SER_write(const char * buff, int len)
{
        uint8_t enable_ints = 0;
        int ret = 0;
        uint8_t restart = 0;

        if (interrupts_enabled()) {
                CLI;
                enable_ints = 1;
        }

        /* Check if serial is initialized (we want VGA prink before serial) */
        if (!produce || !consume) {
                ret = -1;
                goto exit;
        }

        /* If producer and consumer start together, try to restart TX before
         * exiting */
        if(produce == consume)
                restart = 1;

        for (int i = 0; i < len; i++) {
                *produce = buff[i];

                /* Increment producer; if we pass the consumer we will loose 
                 * the whole buffer but should't break anything */
                produce++;
                if(produce >= tx_buff + SERIAL_TX_BUFF_LEN)
                        produce = tx_buff;
        }
        
exit:
        if(restart)
                serial_pic_handle();    /* Interrupts MUST be off to call */
        if(enable_ints)
                STI;
        return ret;
}