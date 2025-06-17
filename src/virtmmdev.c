#include "virtmmdev.h"

VirtMMAccessResult virtual_mmdev_load(uintptr_t virt_addr, void *reg_ptr, MemoryAccessWidth load_width)
{
	switch (virt_addr) {
	case 0x0200BFF8: {  // clint.mtime
			if (load_width != MAW_64BIT) {
				return VMMAR_BAD_ACCESS;
			}
			if (reg_ptr) {
				*(uint64_t*) reg_ptr = 0;  // TODO
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
			(void) value;  // TODO
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
	case 0x10000001: {  // uart.ier
			if (store_width != MAW_8BIT) {
				return VMMAR_BAD_ACCESS;
			}
			uint8_t value = reg_ptr ? *(uint8_t*) reg_ptr : 0;
			(void) value;  // TODO
			return VMMAR_SUCCESS;
		}
		break;
	}
	return VMMAR_UNKNOWN_ADDR;
}
