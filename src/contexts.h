#ifndef SRC_CONTEXTS_H_
#define SRC_CONTEXTS_H_

#include "base_types.h"
#include "vmem.h"

typedef struct {
	// Order expected by assembly
	SavedRegisters active_regs;
	void *exception_handler_stack;  // One past end

	// Additional data for C code
	GuestThreadId current_guest;
} HostThreadData;

typedef struct {
	struct {
		uint64_t mtvec;
		uint64_t mepc;
		uint64_t mscratch;
		uint64_t mstatus_mpp : 2;
		uint64_t mstatus_mie : 1;
		uint64_t mie_msie : 1;
		uint64_t mie_mtie : 1;
		uint64_t mie_meie : 1;
		uint64_t satp_mode : 4;
		uint64_t satp_ppn : 44;
	} csr;
} GuestThreadContext;

extern GuestThreadContext guest_threads[MAX_GUESTS][MAX_VIRT_HARTS];
extern HostThreadData host_threads[MAX_PHYS_HARTS];

__attribute__((unused)) inline static void set_host_thread_address(HostThreadData *ptr)
{
	w_mscratch((uintptr_t) ptr);
}

__attribute__((unused)) inline static HostThreadData *get_host_thread_address(void)
{
	return (HostThreadData*) r_mscratch();
}

#endif /* end of include guard: SRC_CONTEXTS_H_ */
