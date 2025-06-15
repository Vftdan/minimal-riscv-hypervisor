#include "exchandlers.h"

#include "csr.h"
#include "hypervisor.h"

void handle_interrupt(uint64_t mcause)
{
	switch (mcause & ~MCAUSE_ASYNC_BIT) {
	case 7: {
			// TODO reschedule the timer (clint)
			// TODO handle timer interrupt
		}
		break;
	case 11: {
			// TODO handle external interrupt (plic)
		}
		break;
	default:
		;  // TODO handle unsupported interrupt
	}
}

void handle_hypervisor_exception(uint64_t mcause)
{
	// Exception caused by the hypervisor itself
	// TODO panic
	(void) mcause;
}

void handle_guest_exception(uint64_t mcause)
{
	switch (mcause) {
		// TODO emulate the instruction when should be allowed
		//      otherwise, forward the error to the guest
	}
}
