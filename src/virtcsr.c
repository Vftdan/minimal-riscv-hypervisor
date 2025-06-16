#include "virtcsr.h"

#include <stdbool.h>
#include "print.h"
#include "panic.h"
#include "contexts.h"

uint64_t get_virtual_csr(CSRNumber csr_id)
{
	char *csr_name = "unknown";
	switch (csr_id) {
#define DECLARE_CSR(num, name) case CSR_##name: csr_name = #name; break;
#include "csrs.cc"
#undef DECLARE_CSR
	}
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadId guest_thid = host_thr->current_guest;
	GuestThreadContext *guest_thr = &guest_threads[guest_thid.machine][guest_thid.thread];

	switch (csr_id) {
	case CSR_mstatus: {
			uint64_t result = 0;
			uint64_t uxl = 2;  // Guest user mode is always 64-bit
			result |= uxl << 32;
			result |= guest_thr->csr.mstatus_mpp << 11;
			return result;
		}
		break;
	default:
		print_string("\nGet emulated csr: ");
		print_string(csr_name);
		panic();
	}
}

void set_virtual_csr(CSRNumber csr_id, uint64_t value)
{
	char *csr_name = "unknown";
	switch (csr_id) {
#define DECLARE_CSR(num, name) case CSR_##name: csr_name = #name; break;
#include "csrs.cc"
#undef DECLARE_CSR
	}
	print_string("\nSet emulated csr: ");
	print_string(csr_name);
	print_string("\nvalue = ");
	print_addr(value);
	panic();
}
