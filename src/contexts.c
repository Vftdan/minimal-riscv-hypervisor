#include "contexts.h"

#define GUEST_THREAD_CONTEXT_INIT (GuestThreadContext) {.csr.mstatus_mdt = true}

GuestThreadContext guest_threads[MAX_GUESTS][MAX_VIRT_HARTS] = {{GUEST_THREAD_CONTEXT_INIT}};
HostThreadData host_threads[MAX_PHYS_HARTS] = {};
GuestMachineData guest_machines[MAX_GUESTS] = {};
