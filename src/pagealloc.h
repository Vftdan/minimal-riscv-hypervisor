#ifndef SRC_PAGEALLOC_H_
#define SRC_PAGEALLOC_H_

#include "vmem.h"

#define MAX_ALLOCATEABLE_PAGES 16384

MemoryPage *allocate_page(void);
void deallocate_page(MemoryPage *page);

#endif /* end of include guard: SRC_PAGEALLOC_H_ */
