#include "panic.h"

#include "hypervisor.h"
#include "print.h"

void panic(void)
{
	print_string("\n\e[0;1;91m*** Hypervisor panic ***\e[0m\n");
	halt_thread();
}
