#ifndef SRC_VIRTMMDEV_H_
#define SRC_VIRTMMDEV_H_

#include "vmem.h"

typedef enum {
	VMMAR_SUCCESS,
	VMMAR_UNKNOWN_ADDR,
	VMMAR_BAD_ACCESS,  // Virtual address in the middle of device register or width mismatch
} VirtMMAccessResult;

VirtMMAccessResult virtual_mmdev_load(uintptr_t virt_addr, void *reg_ptr, MemoryAccessWidth load_width);
VirtMMAccessResult virtual_mmdev_store(uintptr_t virt_addr, const void *reg_ptr, MemoryAccessWidth store_width);

#endif /* end of include guard: SRC_VIRTMMDEV_H_ */
