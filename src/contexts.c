#include "contexts.h"

#define GUEST_THREAD_CONTEXT_INIT (GuestThreadContext) {.csr.mstatus_mdt = true, .csr.mstatus_mpie = true, .privelege_level = PL_MACHINE}
#define GUEST_MACHINE_DATA_INIT (GuestMachineData) {.uart.input_buffer = BYTE_RING_BUFFER_INIT_LIT, .uart.output_buffer = BYTE_RING_BUFFER_INIT_LIT}

GuestThreadContext guest_threads[MAX_GUESTS][MAX_VIRT_HARTS] = {{GUEST_THREAD_CONTEXT_INIT}};
HostThreadData host_threads[MAX_PHYS_HARTS] = {};
GuestMachineData guest_machines[MAX_GUESTS] = {GUEST_MACHINE_DATA_INIT};
