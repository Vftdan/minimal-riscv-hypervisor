#ifndef SRC_HYPERVISOR_H_
#define SRC_HYPERVISOR_H_

#include <stdint.h>
#include <stddef.h>

// Imported from hypenterleave.S
void halt_thread(void);
void _exc_entry(void);
void enter_guest(void);

// Exported from hypervisor.c
void boot_main(void);
void exception(void);
extern uint8_t hypstack0[4096];

#endif /* end of include guard: SRC_HYPERVISOR_H_ */
