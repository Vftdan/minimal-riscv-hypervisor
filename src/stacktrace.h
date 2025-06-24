#ifndef SRC_STACKTRACE_H_
#define SRC_STACKTRACE_H_

#ifdef USE_STACK_TRACE
typedef struct stack_trace_entry StackTraceEntry;
struct stack_trace_entry {
	StackTraceEntry *caller;
	const char *description;
};

#define STACK_TRACE_ENTRY_STR_INNER(s) #s
#define STACK_TRACE_ENTRY_STR(s) STACK_TRACE_ENTRY_STR_INNER(s)
#define MAKE_STACK_TRACE_ENTRY_DESCRIPTION(msg) (__FILE__ ":" STACK_TRACE_ENTRY_STR(__LINE__) ": " msg)

void stack_trace_push_descr(const char *descr, StackTraceEntry *buffer);
void stack_trace_pop();
#define stack_trace_push_msg(msg, buffer) stack_trace_push_descr(MAKE_STACK_TRACE_ENTRY_DESCRIPTION(msg), buffer)
#define stack_trace_push(buffer) stack_trace_push_descr(MAKE_STACK_TRACE_ENTRY_DESCRIPTION(""), buffer)

#define WITH_STACK_TRACE(msg) for (StackTraceEntry stack_trace_entry = {}; stack_trace_entry.description ? 0 : (stack_trace_push_msg(msg, &stack_trace_entry), 1); stack_trace_pop())
#else
#define WITH_STACK_TRACE(msg)
#endif

#endif /* end of include guard: SRC_STACKTRACE_H_ */
