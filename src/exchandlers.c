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

void handle_sync_exception(uint64_t mcause)
{
	uint64_t mstatus = r_mstatus();
	if ((mstatus & MSTATUS_MPP_MASK) != MSTATUS_MPP_U) {
		// Exception caused by the hypervisor itself
		// TODO panic
	}

	switch (mcause) {
		// TODO emulate the instruction when should be allowed
		//      otherwise, forward the error to the guest
	}
}
