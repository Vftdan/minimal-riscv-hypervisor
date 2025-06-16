#include "contexts.h"

GuestThreadContext guest_threads[MAX_GUESTS][MAX_VIRT_HARTS] = {};
HostThreadData host_threads[MAX_PHYS_HARTS] = {};
