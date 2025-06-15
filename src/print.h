#ifndef SRC_PRINT_H_
#define SRC_PRINT_H_

#include <stddef.h>
#include <stdint.h>

void print_string_slice(size_t len, const char slc[len]);
void print_string(const char *str);
void print_addr(uintptr_t num);

#endif /* end of include guard: SRC_PRINT_H_ */
