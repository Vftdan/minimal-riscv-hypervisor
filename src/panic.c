#include "panic.h"

#include "hypervisor.h"
#include "print.h"
#include "contexts.h"

void panic(void)
{
	print_string("\n\e[0;1;91m*** Hypervisor panic ***\e[0m\n");
	dump_caller_registers();
	halt_thread();
}

void dump_caller_registers(void)
{
	print_string("mepc = ");
	print_addr(r_mepc());
	print_string("\n");

	HostThreadData *ctx = get_host_thread_address();
	if (ctx == NULL) {
		print_string("mscratch = NULL\n");
		return;
	}
#define DECLARE_GP_REGISTER(num, name) \
		print_string(#name " = "); \
		print_addr(ctx->active_regs.name); \
		print_string("\n");
#include "registers.cc"
#undef DECLARE_GP_REGISTER
}
