#include "extinterrupts.h"

#include "contexts.h"
#include "uart.h"
#include "print.h"
#include "guestprivilege.h"

#pragma pack(push, 0)
typedef struct {
	uint32_t source_priorities[1024];
	uint32_t pending_bits[32];
	const uint8_t _pad0[0x2000 - sizeof(uint32_t) * (32 + 1024)];
	uint32_t context_source_enable_bits[7936 /* hart */][2 /* is supervisor */][32];
	const uint32_t _pad1[16384 - 64 - 15872][32];
	struct {
		uint32_t priority_threshold;
		union {
			uint32_t complete;
			const uint32_t claim;
		};
		const uint32_t _reserved[1022];
	} contexts[7936 /* hart */][2 /* is supervisor */];
} PlicRegisters;
#pragma pack(pop)

static volatile UartRegisters *uart0 = (volatile UartRegisters*) 0x10000000;
static volatile PlicRegisters *plic0 = (volatile PlicRegisters*) 0x0c000000;

int uart_input_vm = 0;

void uart_on_thre(void)
{
	bool updated = false;
	for (int mid = 0; mid < MAX_GUESTS; ++mid) {
		if (!uart_can_putc(uart0)) {
			break;
		}
		GuestMachineData *guest_mach = &guest_machines[mid];
		while (!brb_is_empty(&guest_mach->uart.output_buffer) && uart_can_putc(uart0)) {
			if (!updated) {
				updated = true;
				print_string("\e[0;92m");
			}
			char value = 0;
			brb_read(&guest_mach->uart.output_buffer, &value);
			uart_putc_blocking(uart0, value);
		}
	}
	if (updated) {
		print_string("\e[0m");
	}
	forward_external_interrupts();
}

void uart_on_rda(void)
{
	int mid = uart_input_vm;
	if (mid >= 0 && mid < MAX_GUESTS) {
		GuestMachineData *guest_mach = &guest_machines[mid];
		while (!brb_is_full(&guest_mach->uart.input_buffer) && uart_can_getc(uart0)) {
			brb_write(&guest_mach->uart.input_buffer, uart_getc_blocking(uart0));
		}
	}
	forward_external_interrupts();
}

void init_external_interrupts(void)
{
	w_mie(r_mie() | MIE_MEIE);

	// TODO index with the correct hart once implemented
	plic0->contexts[0][false].priority_threshold = 0;
	plic0->source_priorities[PLIC_UART_IRQ] = 1;
	plic0->context_source_enable_bits[0][false][0] |= 1 << PLIC_UART_IRQ;

	uart_subscribe_thre(uart0, true);
	uart_subscribe_rda(uart0, true);
}

void handle_external_interrupt(void) {
	uint32_t irq = plic0->contexts[0][false].claim;
	switch (irq) {
	case PLIC_UART_IRQ: {
			uint8_t iir = uart0->iir;
			if (!(iir & 1)) {
				switch (iir & UART_IIR_MASK) {
				case UART_IIR_RDA:
					uart_on_rda();
					break;
				case UART_IIR_THRE:
					uart_on_thre();
					break;
				default:
					break;
				}
			}
		}
		break;
	default:
		break;
	}
	plic0->contexts[0][false].complete = irq;
}

void forward_external_interrupts(void)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	GuestMachineData *guest_mach = &guest_machines[host_thr->current_guest.machine];
	bool has_interrupts = false;
	if (!has_interrupts && guest_mach->uart.ier_rda) {
		if (!brb_is_empty(&guest_mach->uart.input_buffer)) {
			if (!guest_mach->uart.notified_rda) {
				guest_mach->uart.notified_rda = true;
				has_interrupts = true;
			}
		} else {
			guest_mach->uart.notified_rda = false;
		}
	}
	if (!has_interrupts && guest_mach->uart.ier_thre) {
		if (!brb_is_full(&guest_mach->uart.output_buffer)) {
			if (!guest_mach->uart.notified_thre) {
				guest_mach->uart.notified_thre = true;
				has_interrupts = true;
			}
		} else {
			guest_mach->uart.notified_thre = false;
		}
	}
	if (has_interrupts) {
		if (guest_thr->plic.subscribe_uart_irq_machine) {
			guest_thr->plic.pending_uart_irq_machine = true;
			guest_defer_exception(MCAUSE_ASYNC_BIT | MCAUSE_ASYNC_EXTERNAL);
		} else if (guest_thr->plic.subscribe_uart_irq_supervisor) {
			guest_thr->plic.pending_uart_irq_supervisor = true;
			guest_defer_exception(MCAUSE_ASYNC_BIT | SCAUSE_ASYNC_EXTERNAL);
		}
	}
}
