#include "hypervisor.h"

#include "csr.h"
#include "exchandlers.h"
#include "print.h"
#include "panic.h"
#include "contexts.h"

uint8_t hypstack0[4096] = {};

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
#define SATP_SV39 (8L << 60)
#define MAKE_SATP(ptptr) (SATP_SV39 | ((uint64_t)(ptptr) >> 12))
		w_satp(MAKE_SATP(&machine_pagetable_roots[0][0]));
		vmem_fence(NULL, NULL);
#undef MAKE_SATP
#undef SATP_SV39
	}
	enter_guest();
}

void exception(void)
{
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
}
