#include "hypervisor.h"

#include "csr.h"
#include "exchandlers.h"
#include "print.h"
#include "panic.h"
#include "contexts.h"
#include "timer.h"
#include "stacktrace.h"

uint8_t hypstack0[4096] = {};
uint8_t hypstack_nested[4096] = {};

void boot_main(void)
{
	// TODO set mstatus.TW=1 to provide a custom implementation for wfi
	// Set M Previous Privilege mode to User so mret returns to user mode
	{
		uint64_t mstatus = r_mstatus();
		mstatus &= ~MSTATUS_MPP_MASK;
		mstatus |= MSTATUS_MPP_U;
		w_mstatus(mstatus);
	}

	// Enable machine-mode interrupts
	w_mstatus(r_mstatus() | MSTATUS_MIE);

	// Set the machine-mode trap handler to jump to function "_exc_entry" when a trap occurs
	w_mtvec((uint64_t) _exc_entry);

	// Disable paging for now
	w_satp(0);

	// Configure Physical Memory Protection to give guests access to all of physical memory
	w_pmpaddr0(0x3fffffffffffffULL);
	w_pmpcfg0(0xf);

	// Guest kernels must put their entry at 0x80000000
	w_mepc(0x80000000);

	// Update host thread data if needed
	host_threads[0].exception_handler_stack = hypstack0 + sizeof(hypstack0);
	set_host_thread_address(&host_threads[0]);
	// TODO
	{
		w_satp(MAKE_SATP(&machine_pagetable_roots[0][0]));
		vmem_fence(NULL, NULL);
	}
	enter_guest();
}

void exception(void)
{
	HostThreadData *ctx = get_host_thread_address();
	uint8_t *current_stack = ctx->exception_handler_stack;
	ctx->exception_handler_stack = hypstack_nested + sizeof(hypstack_nested);  // Do not overwrite the stack on which an error occured
	timer_suspend_virtual();
	uint64_t mcause = r_mcause();
	if (mcause & MCAUSE_ASYNC_BIT) {
		handle_interrupt(mcause);
	} else {
		uint64_t mstatus = r_mstatus();
		if ((mstatus & MSTATUS_MPP_MASK) != MSTATUS_MPP_U) {
			handle_hypervisor_exception(mcause);
		} else {
			handle_guest_exception(mcause);
		}
	}
	timer_resume_virtual();
	timer_reschedule();
	ctx->exception_handler_stack = current_stack;
#ifdef USE_STACK_TRACE
	stack_trace_clear();  // Clean up returns and breaks bypassing stack trace updates
#endif
}
