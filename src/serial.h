/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/serial.h
 *
 * Header for serial interface functions
 * 
 */

#ifndef SERIAL_H
#define SERIAL_H                                1

#define SERIAL_TX_BUFF_LEN                      64

#define SERIAL_IO_COM1                          0x03F8
#define SERIAL_IO_COM2                          0x02F8
#define SERIAL_IO_COM3                          0x03E8
#define SERIAL_IO_COM4                          0x02E8
#define SERIAL_IO_LPT1                          0x0378
#define SERIAL_IO_LPT2                          0x0278

#define SERIAL_IRQ_COM1                         4
#define SERIAL_IRQ_COM2                         3
#define SERIAL_IRQ_COM3                         4
#define SERIAL_IRQ_COM4                         3
#define SERIAL_IRQ_LPT1                         7
#define SERIAL_IRQ_LPT2                         5

#define SERIAL_DATA_REG                         0
#define SERIAL_INT_REG                          1
#define SERIAL_DVD_LSB_REG                      0
#define SERIAL_DVD_MSB_REG                      1
#define SERIAL_INT_ID_REG                       2
#define SERIAL_FIFO_CONTROL_REG                 2
#define SERIAL_LINE_CONTROL_REG                 3
#define SERIAL_MODEM_CONTROL_REG                4
#define SERIAL_LINE_STATUS_REG                  5
#define SERIAL_MODEM_STATUS_REG                 6
#define SERIAL_SCRATCH_REG                      7

/* Interrupt enable bitmasks */
#define SERIAL_INT_DATA_AVAILABLE               (0b1<<0)
#define SERIAL_INT_TX_EMPTY                     (0b1<<1)
#define SERIAL_INT_ERROR                        (0b1<<2)
#define SERIAL_INT_STATUS                       (0b1<<3)

/* Interrupt ID bitmasks */
#define SERIAL_INT_PENDING                      (0b1<<0)
#define SERIAL_MSR_CHANGE                       (0b000<<1)
#define SERIAL_TX_EMPTY                         (0b001<<1)
#define SERIAL_RX_DATA                          (0b010<<1)
#define SERIAL_LSR_CHANGE                       (0b011<<1)
#define SERIAL_CHAR_TIMEOUT                     (0b110<<1)
#define SERIAL_64_BIT_FIFO                      (0b1<<5)
#define SERIAL_NO_FIFO                          (0b00<<6)
#define SERIAL_UNUSABLE_FIFO                    (0b01<<6)
#define SERIAL_FIFO_ENABLED                     (0b11<<6)

/* FIFO control bitmasks*/
#define SERIAL_ENABLE_FIFO                      (0b1<<0)
#define SERIAL_CLEAR_RX_FIFO                    (0b1<<1)
#define SERIAL_CLEAR_TX_FIFO                    (0b1<<2)
#define SERIAL_DMA_MODE                         (0b1<<3)
#define SERIAL_FIFO_1B                          (0b00<<6)
#define SERIAL_FIFO_4B                          (0b01<<6)
#define SERIAL_FIFO_8B                          (0b10<<6)
#define SERIAL_FIFO_14B                         (0b11<<6)

/* Line control bitmasks */
#define SERIAL_5_BIT_CHAR                       (0b000)
#define SERIAL_6_BIT_CHAR                       (0b001)
#define SERIAL_7_BIT_CHAR                       (0b010)
#define SERIAL_8_BIT_CHAR                       (0b011)
#define SERIAL_STOP_BITS                        (0b100)
#define SERIAL_PARITY_NONE                      (0b000<<3)
#define SERIAL_PARITY_ODD                       (0b001<<3)
#define SERIAL_PARITY_EVEN                      (0b011<<3)
#define SERIAL_PARITY_MARK                      (0b101<<3)
#define SERIAL_PARITY_SPACE                     (0b111<<3)
#define SERIAL_DLAB                             (0b1<<7)

/* Modem control bitmasks */
#define SERIAL_DTR                              (0b1<<0)
#define SERIAL_RTS                              (0b1<<1)
#define SERIAL_OUT1                             (0b1<<2)
#define SERIAL_OUT2                             (0b1<<3)
#define SERIAL_LOOP                             (0b1<<4)

/* Line status register bitmasks */
#define SERIAL_DATA_READY                       (0b1<<0)
#define SERIAL_OVERRUN_ERROR                    (0b1<<1)
#define SERIAL_PARITY_ERROR                     (0b1<<2)
#define SERIAL_FRAMING_ERROR                    (0b1<<3)
#define SERIAL_BREAK_INDICATOR                  (0b1<<4)
#define SERIAL_TX_BUFF_EMPTY                    (0b1<<5)
#define SERIAL_TX_IDLE                          (0b1<<6)
#define SERIAL_IMPENDING_ERROR                  (0b1<<7)

/* Modem status register bitmasks */
#define SERIAL_MODEM_DCTS                       (0b1<<0)
#define SERIAL_MODEM_DDSR                       (0b1<<1)
#define SERIAL_MODEM_TERI                       (0b1<<2)
#define SERIAL_MODEM_DDCD                       (0b1<<3)
#define SERIAL_CTS                              (0b1<<4)
#define SERIAL_DSR                              (0b1<<5)
#define SERIAL_RI                               (0b1<<6)
#define SERIAL_DCD                              (0b1<<7)

void serial_pic_handle(void);
void SER_init(void);
int SER_write(const char *, int);

#endif /* #ifndef SERIAL_H */