#include "hypervisor.h"

#include "csr.h"
#include "exchandlers.h"

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
		handle_sync_exception(mcause);
	}
}
