#include <stdint.h>
#include <stddef.h>

__attribute__((optimize("no-tree-loop-distribute-patterns")))
void memset(char *dst, int val, size_t size)
{
	for (size_t i = 0; i < size; ++i) {
		dst[i] = (char) val;
	}
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
void memcpy(char *dst, const char *src, size_t size)
{
	for (size_t i = 0; i < size; ++i) {
		dst[i] = src[i];
	}
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
int memcmp(const unsigned char *s1, const unsigned char *s2, size_t size)
{
	for (size_t i = 0; i < size; ++i) {
		short diff = s1[i] - (short) s2[i];
		if (diff < 0) {
			return -1;
		}
		if (diff > 0) {
			return 1;
		}
	}
	return 0;
}

__attribute__((optimize("no-tree-loop-distribute-patterns")))
void memmove(volatile char *dst, volatile const char *src, size_t size)
{
	if ((uintptr_t) dst <= (uintptr_t) src) {
		for (size_t i = 0; i < size; ++i) {
			dst[i] = src[i];
		}
	} else {
		for (size_t i = size; i > 0; --i) {
			dst[i - 1] = src[i - 1];
		}
	}
}
