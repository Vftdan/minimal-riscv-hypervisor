#include "hypervisor.h"

#include "csr.h"
#include "exchandlers.h"
#include "print.h"
#include "panic.h"

uint8_t hypstack0[4096] = {};

void boot_main(void)
{
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
