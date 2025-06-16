#include "hypervisor.h"

#include "csr.h"
#include "exchandlers.h"
#include "print.h"
#include "panic.h"
#include "contexts.h"

uint8_t hypstack0[4096] = {};

void boot_main(void)
{
	// Update host thread data if needed
	host_threads[0].exception_handler_stack = hypstack0 + sizeof(hypstack0);
	set_host_thread_address(&host_threads[0]);
	// TODO
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
