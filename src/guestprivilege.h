#ifndef SRC_GUESTPRIVILEGE_H_
#define SRC_GUESTPRIVILEGE_H_

void guest_mret(void);  // GM -> GU
void guest_exception(void);  // GU -> GM

#endif /* end of include guard: SRC_GUESTPRIVILEGE_H_ */
