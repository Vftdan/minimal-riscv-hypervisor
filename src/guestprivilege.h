#ifndef SRC_GUESTPRIVILEGE_H_
#define SRC_GUESTPRIVILEGE_H_

#include <stdint.h>

void guest_mret(void);  // GM -> GU
void guest_exception(uint64_t mcause);  // GU -> GM

#endif /* end of include guard: SRC_GUESTPRIVILEGE_H_ */
