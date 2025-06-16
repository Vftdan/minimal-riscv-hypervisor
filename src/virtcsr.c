#include "virtcsr.h"

#include "print.h"
#include "panic.h"

uint64_t get_virtual_csr(CSRNumber csr_id)
{
	char *csr_name = "unknown";
	switch (csr_id) {
#define DECLARE_CSR(num, name) case CSR_##name: csr_name = #name; break;
#include "csrs.cc"
#undef DECLARE_CSR
	}

	switch (csr_id) {
	case CSR_mstatus: {
			uint64_t result = 0;
			uint64_t uxl = 2;  // Guest user mode is always 64-bit
			result |= uxl << 32;
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
