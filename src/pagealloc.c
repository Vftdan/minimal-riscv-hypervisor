#include "pagealloc.h"

#include "sync.h"

typedef struct free_list_entry FreeListEntry;
struct free_list_entry {
	FreeListEntry *next;
};

typedef union {
	MemoryPage page;
	FreeListEntry deallocated;
} PagePoolElement;

SpinLock allocator_lock = SPIN_LOCK_UNLOCKED;
PagePoolElement page_pool[MAX_ALLOCATEABLE_PAGES];  // Uninitialized
FreeListEntry *free_list_top = NULL;
size_t next_unallocated_idx = 0;

_Static_assert(_Alignof(typeof(page_pool[0])) == 4096, "Page pool element type is not properly aligned");
_Static_assert(sizeof(typeof(page_pool[0])) == 4096, "Page pool element type is of incorrect size");

MemoryPage *allocate_page(void)
{
	MemoryPage *result = NULL;

	spin_lock_acquire(&allocator_lock);
	FreeListEntry *entry;
	if ((entry = free_list_top)) {
		free_list_top = entry->next;
		result = &((PagePoolElement*) entry)->page;
	} else if (next_unallocated_idx < MAX_ALLOCATEABLE_PAGES) {
		result = &page_pool[next_unallocated_idx].page;
		++next_unallocated_idx;
	}
	spin_lock_release(&allocator_lock);

	return result;
}

void deallocate_page(MemoryPage *page)
{
	if (!page) {
		return;
	}
	PagePoolElement *element = (PagePoolElement*) page;
	// TODO check bounds and alignment

	spin_lock_acquire(&allocator_lock);
	element->deallocated.next = free_list_top;
	free_list_top = &element->deallocated;
	spin_lock_release(&allocator_lock);
}
