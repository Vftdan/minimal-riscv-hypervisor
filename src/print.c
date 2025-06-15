#include "print.h"

#include "uart.h"

volatile UartRegisters *uart0 = (volatile UartRegisters*) 0x10000000;

void print_string_slice(size_t len, const char slc[len])
{
	uart_write_blocking(uart0, len, slc);
}

void print_string(const char *str)
{
	if (!str) {
		return;
	}

	size_t len = 0;
	while (str[len])
		++len;
	print_string_slice(len, str);
}

void print_addr(uintptr_t num)
{
	char buf[2 + 2 * sizeof(uintptr_t)] = "0x";
	for (size_t i = sizeof(uintptr_t); i > 0; --i) {
		uint8_t byte = num & 0xFF;
		num >>= 8;
		uint8_t nibbles[2] = { byte >> 4, byte & 0xF };
		for (size_t j = 0; j < 2; ++j) {
			uint8_t c = nibbles[j];
			if (c < 10) {
				c += '0';
			} else {
				c += 'A' - 10;
			}
			buf[i * 2 + j] = c;
		}
	}
	print_string_slice(sizeof(buf), buf);
}
