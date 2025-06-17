#ifndef SRC_VMEM_H_
#define SRC_VMEM_H_

#include "base_types.h"

// First guest should resolve guest ram_start to host (ram_start + GUEST_MEMORY_OFFSET)
// It's also the offset at which the first kernel and its data should be loaded
#define GUEST_MEMORY_OFFSET 0x5000000

// Number of lower bits inside a memory address that are withing a single page
#define PGSHIFT 12
#define PGSIZE (1 << PGSHIFT)

// Page table entry in a form that is understood by MMU
typedef struct {
	uint64_t numeric_value;
} PackedPagetableEntry;

typedef PackedPagetableEntry __attribute__((aligned(PGSIZE))) PagetablePage[512];
typedef uint8_t __attribute__((aligned(PGSIZE))) MemoryPage[PGSIZE];

// Pre-allocate top-level entries of guest physical -> host physical pagetables
extern PagetablePage machine_pagetable_roots[MAX_GUESTS];

#define PERMSHIFT 10
#define PGOFFSET_MASK (PGSIZE - 1)
typedef enum {
	PERMIDX_V,  // The page table entry is valid
	PERMIDX_R,  // Allow reading, is not a lower-level pagetable
	PERMIDX_W,  // Allow writing, is not a lower-level pagetable
	PERMIDX_X,  // Allow contained machine code execution, is not a lower-level pagetable
	PERMIDX_U,  // Accessible in the U-mode, otherwise in the S-mode
	PERMIDX_G,  // This entry is the same for all address space identifiers (global)
	PERMIDX_A,  // Was read from, written to, or executed from (accessed)
	PERMIDX_D,  // Was written to (dirty)
	PERMIDX_RESERVED1,
	PERMIDX_RESERVED2,
} MempermIndex;

typedef uint64_t MempermMask;

// Additional typecheck
__attribute__((unused)) inline static MempermMask _permbit_from_index(MempermIndex idx)
{
	return 1 << idx;
}
#define PERMBIT(f) _permbit_from_index(PERMIDX_##f)

typedef struct {
	union {
		PagetablePage *child_table;
		MemoryPage *resolved_range_start;
		uintptr_t numeric_address;
	};
	MempermMask permissions : PERMSHIFT;
} UnpackedPagetableEntry;

__attribute__((unused)) inline static PackedPagetableEntry pack_pt_entry(UnpackedPagetableEntry unpacked)
{
	PackedPagetableEntry packed;
	packed.numeric_value = unpacked.numeric_address >> PGSHIFT << PERMSHIFT;
	packed.numeric_value |= unpacked.permissions;
	return packed;
}

__attribute__((unused)) inline static UnpackedPagetableEntry unpack_pt_entry(PackedPagetableEntry packed)
{
	UnpackedPagetableEntry unpacked;
	unpacked.numeric_address = packed.numeric_value >> PERMSHIFT << PGSHIFT;
	unpacked.permissions = packed.numeric_value & ((1 << PERMSHIFT) - 1);
	return unpacked;
}

typedef enum {
	PFHR_SUCCESS,      // Page tables were modified, the faulted instruction should be retried
	PFHR_NOT_CHANGED,  // Page tables already have all relevant entries, there is probably a bug in the page handling code
	PFHR_TOO_LOW,      // The virtual address is below the handled range
	PFHR_TOO_HIGH,     // The virtual address is above the handled range
} PageFaultHandlerResult;

PageFaultHandlerResult handle_page_fault(MempermIndex access_type, uintptr_t *virt_out);

typedef enum {
	MAW_8BIT = 1,
	MAW_16BIT = 2,
	MAW_32BIT = 4,
	MAW_64BIT = 8,
} MemoryAccessWidth;

__attribute__((unused)) inline static void vmem_fence(const uintptr_t *vaddr_ptr, const uint16_t *asid_ptr)
{
	if (!vaddr_ptr && !asid_ptr) {
		asm volatile("sfence.vma zero, zero");
	} else if (vaddr_ptr && !asid_ptr) {
		asm volatile("sfence.vma %0, zero" : : "r" (*vaddr_ptr));
	} else if (!vaddr_ptr && asid_ptr) {
		asm volatile("sfence.vma zero, %0" : : "r" (*asid_ptr));
	} else if (vaddr_ptr && asid_ptr) {
		asm volatile("sfence.vma %0, %1" : : "r" (*vaddr_ptr), "r" (*asid_ptr));
	}
}

#endif /* end of include guard: SRC_VMEM_H_ */
