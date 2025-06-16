#include "virtmmdev.h"

VirtMMAccessResult virtual_mmdev_load(uintptr_t virt_addr, void *reg_ptr, MemoryAccessWidth load_width)
{
	switch (virt_addr) {
	case 0x0200BFF8: {  // clint.mtime
			if (load_width != MAW_64BIT) {
				return VMMAR_BAD_ACCESS;
			}
			if (reg_ptr) {
				*((uint64_t*) reg_ptr) = 0;  // TODO
			}
			return VMMAR_SUCCESS;
		}
		break;
	}
	return VMMAR_UNKNOWN_ADDR;
}

VirtMMAccessResult virtual_mmdev_store(uintptr_t virt_addr, const void *reg_ptr, MemoryAccessWidth store_width)
{
	return VMMAR_UNKNOWN_ADDR;
}
