#ifndef SRC_EXCHANDLERS_H_
#define SRC_EXCHANDLERS_H_

#include <stdint.h>
#include <stddef.h>

void handle_interrupt(uint64_t mcause);
void handle_sync_exception(uint64_t mcause);

#endif /* end of include guard: SRC_EXCHANDLERS_H_ */
