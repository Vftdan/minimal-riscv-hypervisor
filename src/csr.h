#ifndef SRC_CSR_H_
#define SRC_CSR_H_

#include <stdint.h>
#include <stddef.h>

// Enumeration with numeric values
typedef enum {
#define DECLARE_CSR(num, name) CSR_##name = num,
#include "csrs.cc"
#undef DECLARE_CSR
} CSRNumber;

// Read from CSR
#define DECLARE_CSR(num, name) __attribute__((unused)) static inline uintptr_t r_##name() { uintptr_t x; asm volatile("csrr %0, " #name : "=r" (x) ); return x; }
#include "csrs.cc"
#undef DECLARE_CSR

// Write to CSR
#define DECLARE_CSR(num, name) __attribute__((unused)) static inline void w_##name(uintptr_t x) { asm volatile("csrw " #name ", %0" : : "r" (x)); }
#include "csrs.cc"
#undef DECLARE_CSR

#define MSTATUS_MPP_MASK (3L << 11)  // Previous mode
#define MSTATUS_MPP_M    (3L << 11)
#define MSTATUS_MPP_S    (1L << 11)
#define MSTATUS_MPP_U    (0L << 11)
#define MSTATUS_MIE      (1L << 3)   // Machine-mode interrupt enable

#define MIE_MEIE (1L << 11)  // External
#define MIE_MTIE (1L << 7)   // Timer
#define MIE_MSIE (1L << 3)   // Software

#define MCAUSE_ASYNC_BIT (1ULL << 63)

#endif /* end of include guard: SRC_CSR_H_ */
