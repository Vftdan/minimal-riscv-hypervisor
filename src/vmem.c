#include "vmem.h"

#include <stdalign.h>

PagetablePage machine_pagetable_roots[MAX_GUESTS] = {};

_Static_assert(_Alignof(typeof(machine_pagetable_roots[0])) == 4096, "Page table type is not properly aligned");
_Static_assert(sizeof(typeof(machine_pagetable_roots[0])) == 4096, "Page table type is of incorrect size");
