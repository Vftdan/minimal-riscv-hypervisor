#include "panic.h"

#include "hypervisor.h"
#include "print.h"
#include "contexts.h"
#include "stacktrace.h"

void panic(void)
{
	print_string("\n\e[0;1;91m*** Hypervisor panic ***\e[0m\n");
#ifdef USE_STACK_TRACE
	HostThreadData *ctx = get_host_thread_address();
	if (ctx) {
		StackTraceEntry *entry = ctx->stack_trace_caller;
		StackTraceEntry *entry_slow = entry;
		bool update_slow = false;
		while (entry) {
			print_string("\tat ");
			if (entry->description) {
				print_string(entry->description);
			} else {
				print_string("(null)");
			}
			print_string(" (stacktrace entry is at ");
			print_addr((uintptr_t) entry);
			print_string(")\n");
			if (entry_slow == entry->caller) {
				print_string("\tStacktrace cycle to ");
				print_addr((uintptr_t) entry_slow);
				print_string("!\n");
				break;
			}
			entry = entry->caller;
			if (update_slow) {
				entry_slow = entry_slow->caller;
				update_slow = false;
			} else {
				update_slow = true;
			}
		}
	}
#endif
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
