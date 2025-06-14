#ifndef SRC_VMEM_H_
#define SRC_VMEM_H_

#include "base_types.h"

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

#endif /* end of include guard: SRC_VMEM_H_ */
