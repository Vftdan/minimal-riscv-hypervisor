#ifndef SRC_VIRTCSR_H_
#define SRC_VIRTCSR_H_

#include "csr.h"

uint64_t get_virtual_csr(CSRNumber csr_id);
void set_virtual_csr(CSRNumber csr_id, uint64_t value);

#endif /* end of include guard: SRC_VIRTCSR_H_ */
