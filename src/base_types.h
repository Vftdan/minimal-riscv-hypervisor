#ifndef SRC_BASE_TYPES_H_
#define SRC_BASE_TYPES_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "csr.h"

typedef struct {
#define DECLARE_GP_REGISTER(num, name) uint64_t name;
#include "registers.cc"
#undef DECLARE_GP_REGISTER
} SavedRegisters;

// Number of supported guest machines
#define MAX_GUESTS 1
// Number of emulated hardware threads
#define MAX_VIRT_HARTS 1
// Number of used hardware threads
#define MAX_PHYS_HARTS 1

typedef struct {
	int machine;
	int thread;
} GuestThreadId;

#endif /* end of include guard: SRC_BASE_TYPES_H_ */
