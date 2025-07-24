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

#define MSTATUS_MPP_MASK (3UL << 11)  // Previous mode
#define MSTATUS_MPP_M    (3UL << 11)
#define MSTATUS_MPP_S    (1UL << 11)
#define MSTATUS_MPP_U    (0UL << 11)
#define MSTATUS_MIE      (1UL << 3)   // Machine-mode interrupt enable
#define MSTATUS_MDT      (1UL << 42)  // Machine-mode disable trap

#define MIE_MEIE (1UL << 11)  // External
#define MIE_MTIE (1UL << 7)   // Timer
#define MIE_MSIE (1UL << 3)   // Software

#define MCAUSE_ASYNC_BIT (1ULL << 63)

#define SATP_MODE_SHIFT 60
#define SATP_MODE_NONE  0
#define SATP_MODE_SV39  (8ULL  << SATP_MODE_SHIFT)
#define SATP_MODE_SV48  (9ULL  << SATP_MODE_SHIFT)
#define SATP_MODE_SV57  (10ULL << SATP_MODE_SHIFT)
#define SATP_MODE_SV64  (11ULL << SATP_MODE_SHIFT)
#define MAKE_SATP(ptptr) (SATP_MODE_SV39 | ((uint64_t)(ptptr) >> 12))

#endif /* end of include guard: SRC_CSR_H_ */
