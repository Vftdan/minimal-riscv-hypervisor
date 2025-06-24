#include "panic.h"

#include "hypervisor.h"
#include "print.h"
#include "contexts.h"
#include "stacktrace.h"

void panic(void)
{
	print_string("\n\e[0;1;91m*** Hypervisor panic ***\e[0m\n");
	HostThreadData *ctx = get_host_thread_address();
	if (ctx) {
		StackTraceEntry *entry = ctx->stack_trace_caller;
		while (entry) {
			print_string("\tat ");
			if (entry->description) {
				print_string(entry->description);
			} else {
				print_string("(null)");
			}
			print_string("\n");
			entry = entry->caller;
		}
	}
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
