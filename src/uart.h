#ifndef SRC_UART_H_
#define SRC_UART_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
	union {
		uint8_t       thr;  // W = transmit hold register (offset 0)
		uint8_t const rbr;  // R = receive buffer register (also offset 0)
		uint8_t       dll;  // R/W = divisor latch low (offset 0 when DLAB=1)
	};
	union {
		uint8_t       ier;  // R/W = interrupt enable register (offset 1)
		uint8_t       dlh;  // R/W = divisor latch high (offset 1 when DLAB=1)
	};
	union {
		uint8_t const iir;  // R = interrupt identif. reg. (offset 2)
		uint8_t       fcr;  // W = FIFO control reg. (also offset 2)
	};
	uint8_t       lcr;  // R/W = line control register (offset 3)
	uint8_t       mcr;  // R/W = modem control register (offset 4)
	uint8_t const lsr;  // R   = line status register (offset 5)
} UartRegisters;

#define UART_IER_RDA  (1 << 0)  // Enable Received Data Available Interrupt 
#define UART_IER_THRE (1 << 1)  // Enable Transmitter Holding Register Empty Interrupt
#define UART_IER_RLS  (1 << 2)  // Enable Receiver Line Status Interrupt
#define UART_IER_MS   (1 << 3)  // Enable Modem Status Interrupt 
#define UART_IER_SM   (1 << 4)  // Enable Sleep Mode
#define UART_IER_LPM  (1 << 5)  // Enable Low Power Mode

#define UART_IIR_MASK (7 << 1)
#define UART_IIR_MS   (0 << 1)  // MODEM Status
#define UART_IIR_THRE (1 << 1)  // Transmitter Holding Register Empty
#define UART_IIR_RDA  (2 << 1)  // Received Data Available
#define UART_IIR_RLS  (3 << 1)  // Receiver Line Status
#define UART_IIR_TOI  (6 << 1)  // Character Timeout Indicator

#define UART_LCR_8bpB (3 << 0)  // Values 0..3 indicate 5..8 bits per byte
#define UART_LCR_2STP (1 << 2)  // Use 2 stop bits instead of 1
#define UART_LCR_PAR  (7 << 3)  // Parity mode
#define UART_LCR_BSE  (1 << 6)  // Break Signal Enabled
#define UART_LCR_DLAB (1 << 7)  // Divisor Latch Access Bit

#define UART_LSR_DR   (1 << 0)  // Data Ready
#define UART_LSR_OE   (1 << 1)  // Overrun Error
#define UART_LSR_PE   (1 << 2)  // Parity Error
#define UART_LSR_FE   (1 << 3)  // Framing Error
#define UART_LSR_BI   (1 << 4)  // Break Interrupt
#define UART_LSR_THRE (1 << 5)  // Transmitter Holding Register Empty
#define UART_LSR_TEMT (1 << 6)  // Transmitter Empty
#define UART_LSR_FIFE (1 << 7)  // FIFO Error

__attribute__((unused)) inline static void uart_putc_blocking(volatile UartRegisters *uart, char c)
{
	while ((uart->lsr & UART_LSR_THRE) == 0)
		;  // Polling!
	uart->thr = c;
}

__attribute__((unused)) inline static char uart_getc_blocking(volatile UartRegisters *uart) {
	while ((uart->lsr & UART_LSR_DR) == 0)
		;  // Polling!
	return uart->rbr;
}

__attribute__((unused)) inline static void uart_write_blocking(volatile UartRegisters *uart, size_t len, const char buf[len])
{
	for (size_t i = 0; i < len; ++i) {
		uart_putc_blocking(uart, buf[i]);
	}
}

#endif /* end of include guard: SRC_UART_H_ */
