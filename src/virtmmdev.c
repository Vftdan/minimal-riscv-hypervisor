#include "virtmmdev.h"

#include "print.h"
#include "uart.h"
#include "timer.h"
#include "contexts.h"

static volatile UartRegisters *uart0 = (volatile UartRegisters*) 0x10000000;

VirtMMAccessResult virtual_mmdev_load(uintptr_t virt_addr, void *reg_ptr, MemoryAccessWidth load_width)
{
	switch (virt_addr) {
	case 0x02004000: {  // clint.harts[0].mtimecmp
			if (load_width != MAW_64BIT) {
				return VMMAR_BAD_ACCESS;
			}
			if (reg_ptr) {
				HostThreadData *host_thr = get_host_thread_address();
				GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
				*(uint64_t*) reg_ptr = guest_thr->timer_deadline;
			}
			return VMMAR_SUCCESS;
		}
		break;
	case 0x0200BFF8: {  // clint.mtime
			if (load_width != MAW_64BIT) {
				return VMMAR_BAD_ACCESS;
			}
			if (reg_ptr) {
				*(uint64_t*) reg_ptr = timer_get_time_virtual();
			}
			return VMMAR_SUCCESS;
		}
		break;
	case 0x10000001: {  // uart.{ier,dlh}
			if (load_width != MAW_8BIT) {
				return VMMAR_BAD_ACCESS;
			}
			HostThreadData *host_thr = get_host_thread_address();
			GuestMachineData *guest_mach = &guest_machines[host_thr->current_guest.machine];
			uint8_t result;
			if (guest_mach->uart.dlab) {
				result = 0;  // dlh, divisor = 1
			} else {
				result = 0;  // TODO
			}
			if (reg_ptr) {
				*(uint32_t*) reg_ptr = result;
			}
			return VMMAR_SUCCESS;
		}
		break;
	case 0x0C002000: {  // plic.context_source_enable_bits[0][0]
			if (load_width != MAW_32BIT) {
				return VMMAR_BAD_ACCESS;
			}
			if (reg_ptr) {
				*(uint32_t*) reg_ptr = 0;  // TODO
			}
			return VMMAR_SUCCESS;
		}
		break;
	case 0x10000005: {  // uart.lsr
			if (load_width != MAW_8BIT) {
				return VMMAR_BAD_ACCESS;
			}
			if (reg_ptr) {
				uint8_t actual = uart0->lsr;
				uint8_t result = UART_LSR_THRE | UART_LSR_TEMT;
				result |= actual & UART_LSR_DR;
				*(uint8_t*) reg_ptr = result;
			}
			return VMMAR_SUCCESS;
		}
		break;
	case 0x10000000: {  // uart.{rbr,dll}
			if (load_width != MAW_8BIT) {
				return VMMAR_BAD_ACCESS;
			}
			HostThreadData *host_thr = get_host_thread_address();
			GuestMachineData *guest_mach = &guest_machines[host_thr->current_guest.machine];
			uint8_t result;
			if (guest_mach->uart.dlab) {
				result = 1;  // dll, divisor = 1
			} else {
				result = uart0->rbr;
			}
			if (reg_ptr) {
				*(uint8_t*) reg_ptr = result;
			}
			return VMMAR_SUCCESS;
		}
		break;
	case 0x10000003: {  // uart.lcr
			if (load_width != MAW_8BIT) {
				return VMMAR_BAD_ACCESS;
			}
			if (reg_ptr) {
				uint8_t result = UART_LCR_8bpB;
				HostThreadData *host_thr = get_host_thread_address();
				GuestMachineData *guest_mach = &guest_machines[host_thr->current_guest.machine];
				if (guest_mach->uart.dlab) {
					result |= UART_LCR_DLAB;
				}
				*(uint8_t*) reg_ptr = result;
			}
			return VMMAR_SUCCESS;
		}
		break;
	}
	return VMMAR_UNKNOWN_ADDR;
}

VirtMMAccessResult virtual_mmdev_store(uintptr_t virt_addr, const void *reg_ptr, MemoryAccessWidth store_width)
{
	switch (virt_addr) {
	case 0x02004000: {  // clint.harts[0].mtimecmp
			if (store_width != MAW_64BIT) {
				return VMMAR_BAD_ACCESS;
			}
			uint64_t value = reg_ptr ? *(uint64_t*) reg_ptr : 0;
			HostThreadData *host_thr = get_host_thread_address();
			GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
			guest_thr->timer_deadline = value;
			guest_thr->timer_scheduled = true;
			return VMMAR_SUCCESS;
		}
		break;
	case 0x0C000028: {  // plic.source_priorities[UART_IRQ = 10]
			if (store_width != MAW_32BIT) {
				return VMMAR_BAD_ACCESS;
			}
			uint32_t value = reg_ptr ? *(uint32_t*) reg_ptr : 0;
			(void) value;  // TODO
			return VMMAR_SUCCESS;
		}
		break;
	case 0x0C002000: {  // plic.context_source_enable_bits[0][0]
			if (store_width != MAW_32BIT) {
				return VMMAR_BAD_ACCESS;
			}
			uint32_t value = reg_ptr ? *(uint32_t*) reg_ptr : 0;
			(void) value;  // TODO
			return VMMAR_SUCCESS;
		}
		break;
	case 0x0C200000: {  // plic.contexts[0].priority_threshold
			if (store_width != MAW_32BIT) {
				return VMMAR_BAD_ACCESS;
			}
			uint32_t value = reg_ptr ? *(uint32_t*) reg_ptr : 0;
			(void) value;  // TODO
			return VMMAR_SUCCESS;
		}
		break;
	case 0x10000001: {  // uart.{ier,dlh}
			if (store_width != MAW_8BIT) {
				return VMMAR_BAD_ACCESS;
			}
			uint8_t value = reg_ptr ? *(uint8_t*) reg_ptr : 0;
			HostThreadData *host_thr = get_host_thread_address();
			GuestMachineData *guest_mach = &guest_machines[host_thr->current_guest.machine];
			if (guest_mach->uart.dlab) {
				(void) value;  // Virtual UART divisor is not configurable
			} else {
				(void) value;  // TODO
			}
			return VMMAR_SUCCESS;
		}
		break;
	case 0x10000000: {  // uart.{thr,dll}
			if (store_width != MAW_8BIT) {
				return VMMAR_BAD_ACCESS;
			}
			uint8_t value = reg_ptr ? *(uint8_t*) reg_ptr : 0;
			HostThreadData *host_thr = get_host_thread_address();
			GuestMachineData *guest_mach = &guest_machines[host_thr->current_guest.machine];
			if (guest_mach->uart.dlab) {
				(void) value;  // Virtual UART divisor is not configurable
			} else {
				print_string("\e[0;92m");
				print_string_slice(1, (char*) &value);
				print_string("\e[0m");
			}
			return VMMAR_SUCCESS;
		}
		break;
	case 0x10000003: {  // uart.lcr
			if (store_width != MAW_8BIT) {
				return VMMAR_BAD_ACCESS;
			}
			uint8_t value = reg_ptr ? *(uint8_t*) reg_ptr : 0;
			HostThreadData *host_thr = get_host_thread_address();
			GuestMachineData *guest_mach = &guest_machines[host_thr->current_guest.machine];
			guest_mach->uart.dlab = !!(value & UART_LCR_DLAB);
			return VMMAR_SUCCESS;
		}
		break;
	case 0x10000002: {  // uart.fcr
			if (store_width != MAW_8BIT) {
				return VMMAR_BAD_ACCESS;
			}
			uint8_t value = reg_ptr ? *(uint8_t*) reg_ptr : 0;
			(void) value;  // Do not configure virtual FIFO
			return VMMAR_SUCCESS;
		}
		break;
	}
	return VMMAR_UNKNOWN_ADDR;
}
