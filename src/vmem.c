#include "vmem.h"

#include <stdalign.h>
#include "print.h"
#include "panic.h"
#include "pagealloc.h"
#include "contexts.h"

PagetablePage machine_pagetable_roots[MAX_GUESTS] = {};

_Static_assert(_Alignof(typeof(machine_pagetable_roots[0])) == 4096, "Page table type is not properly aligned");
_Static_assert(sizeof(typeof(machine_pagetable_roots[0])) == 4096, "Page table type is of incorrect size");

PageFaultHandlerResult handle_page_fault(MempermIndex access_type)
{
	HostThreadData *ctx = get_host_thread_address();
	uintptr_t instr_addr = r_mepc();
	uintptr_t fault_addr;
	switch (access_type) {
	case PERMIDX_R:
		print_string("\nLoad page fault");
		panic();
	case PERMIDX_W:
		print_string("\nStore page fault");
		panic();
	case PERMIDX_X:
		fault_addr = instr_addr;
		break;
	default:
		print_string("\nInvalid page fault type");
		panic();
	}

	if (fault_addr < 0x80000000) {
		return PFHR_TOO_LOW;
	}
	uint64_t lvl3 = fault_addr >> 30;
	if (lvl3 >= 512) {
		return PFHR_TOO_HIGH;
	}

	// Level 3 entries don't seem to allow to resolve directly, so we also handle level 2
	uint64_t lvl2 = (fault_addr >> 21) & 0x1FF;
	// TODO use a mutex to avoid concurrent modification of page tables
	UnpackedPagetableEntry unpacked = unpack_pt_entry(machine_pagetable_roots[ctx->current_guest.machine][lvl3]);
	if (!(unpacked.permissions & PERMBIT(V))) {  // If there is no entry at the top-level table
		// We only have reserved a fixed address for top-level (level 3) of the hierarchical page table
		// So the child level (level 2) is allocated dynamically
		MemoryPage *page = allocate_page();
		if (!page) {
			print_string("\nFailed to allocate a page");
			panic();
		}
		unpacked.resolved_range_start = page;  // The same pointer as .child_table, but of a non-specific MemoryPage type
		for (int i = 0; i < 512; ++i) {
			// Fill the new page with zeros, because its contents may be uninitialized or contain old data
			(*unpacked.child_table)[i] = (PackedPagetableEntry) {};
		}
		unpacked.permissions = PERMBIT(V);
		// Now store the child in the page table in the correct format
		// TODO at this moment other harts should not read this entry neither from hypervisor code nor through a MMU
		//      the virtual machine might need to be interrupted, except for the harts that currently use another page table
		machine_pagetable_roots[ctx->current_guest.machine][lvl3] = pack_pt_entry(unpacked);
	}
	PackedPagetableEntry *packed_ptr = &(*unpacked.child_table)[lvl2];
	unpacked = unpack_pt_entry(*packed_ptr);  // REASSIGN unpacked to the child level entry
	if (unpacked.permissions & PERMBIT(V)) {
		return PFHR_NOT_CHANGED;
	}
	// Base physical address of the mapping being initialized
	unpacked.numeric_address = ((lvl3 << 30) | (lvl2 << 21)) + GUEST_MEMORY_OFFSET;
	// Make sure to not set the G bit, because it might cause an undefined behavior when there are multiple harts
	unpacked.permissions = PERMBIT(V) | PERMBIT(U) | PERMBIT(R) | PERMBIT(W) | PERMBIT(X);
	*packed_ptr = pack_pt_entry(unpacked);  // Write the child-level entry
	asm volatile("sfence.vma zero, zero");
	return PFHR_SUCCESS;
}
