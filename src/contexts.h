#ifndef SRC_CONTEXTS_H_
#define SRC_CONTEXTS_H_

#include "base_types.h"
#include "vmem.h"
#include "ringbuf.h"

typedef struct {
	// Order expected by assembly
	SavedRegisters active_regs;
	void *exception_handler_stack;  // One past end

	// Additional data for C code
	GuestThreadId current_guest;
#ifdef USE_STACK_TRACE
	struct stack_trace_entry *stack_trace_caller;
#endif
} HostThreadData;

typedef enum {
	PL_USER = 0,
	PL_SUPER = 1,
	PL_HYPER = 2,
	PL_MACHINE = 3,
} PrivilegeLevel;

static const char PL_NAMES[] = {
	[PL_MACHINE] = 'M',
	[PL_HYPER] = 'H',
	[PL_SUPER] = 'S',
	[PL_USER] = 'U',
};

typedef struct {
	PagetablePage *shadow_page_table;  // Guest virtual to host physical
	bool shadow_pt_active;
	bool timer_scheduled;
	bool timer_retry;
	bool deferred_exception;
	PrivilegeLevel privelege_level;
	uint64_t timer_deadline;
	uint64_t timer_suspended_at;
	uint64_t timer_adjustment;
	uint64_t deferred_mcause;
	struct {
		uint64_t mtvec;
		uint64_t mepc;
		uint64_t mscratch;
		uint64_t mcause;
		uint64_t mtval;
		uint64_t medeleg;
		uint64_t mideleg;
		uint64_t stvec;
		uint64_t sepc;
		uint64_t sscratch;
		uint64_t scause;
		uint64_t stval;
		uint64_t mstatus_mpp : 2;
		uint64_t sstatus_spp : 1;
		uint64_t mstatus_mie : 1;
		uint64_t sstatus_sie : 1;
		uint64_t mstatus_mpie : 1;
		uint64_t sstatus_spie : 1;
		uint64_t mstatus_mdt : 1;
		uint64_t mie_msie : 1;
		uint64_t mie_mtie : 1;
		uint64_t mie_meie : 1;
		uint64_t sie_ssie : 1;
		uint64_t sie_stie : 1;
		uint64_t sie_seie : 1;
		uint64_t satp_mode : 4;
		uint64_t satp_ppn : 44;
	} csr;
	struct {
		bool subscribe_uart_rda;
		bool subscribe_uart_thre;
	} plic;
} GuestThreadContext;

typedef struct {
	struct {
		bool dlab;
		bool ier_rda;
		bool ier_thre;
		char old_rbr;
		ByteRingBuffer input_buffer, output_buffer;
	} uart;
} GuestMachineData;

extern GuestThreadContext guest_threads[MAX_GUESTS][MAX_VIRT_HARTS];
extern HostThreadData host_threads[MAX_PHYS_HARTS];
extern GuestMachineData guest_machines[MAX_GUESTS];

__attribute__((unused)) inline static void set_host_thread_address(HostThreadData *ptr)
{
	w_mscratch((uintptr_t) ptr);
}

__attribute__((unused)) inline static HostThreadData *get_host_thread_address(void)
{
	return (HostThreadData*) r_mscratch();
}

#endif /* end of include guard: SRC_CONTEXTS_H_ */
