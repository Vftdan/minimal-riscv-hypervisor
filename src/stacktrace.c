#include "stacktrace.h"

#include "contexts.h"

void stack_trace_push_descr(const char *descr, StackTraceEntry *buffer)
{
	HostThreadData *ctx = get_host_thread_address();
	buffer->description = descr;
	buffer->caller = ctx->stack_trace_caller;
	ctx->stack_trace_caller = buffer;
}

void stack_trace_pop()
{
	HostThreadData *ctx = get_host_thread_address();
	ctx->stack_trace_caller = ctx->stack_trace_caller->caller;
}
