#ifndef SRC_GUESTPRIVILEGE_H_
#define SRC_GUESTPRIVILEGE_H_

#include <stdint.h>
#include <stdbool.h>

void guest_mret(void);  // GM -> GU
void guest_exception(uint64_t mcause);  // GU -> GM
void guest_defer_exception(uint64_t mcause);  // GU -> GM, after MDT is cleared
bool guest_check_deferred(void);  // Peform the guest_defer_exception action if allowed
void guest_on_satp_write(void);

#endif /* end of include guard: SRC_GUESTPRIVILEGE_H_ */
