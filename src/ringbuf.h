#ifndef SRC_RINGBUF_H_
#define SRC_RINGBUF_H_

#include <stdint.h>
#include "sync.h"

typedef struct {
	volatile char data[256];
	volatile uint8_t writeidx,
		 readidx;
	SpinLock lock;
} ByteRingBuffer;

#define BYTE_RING_BUFFER_INIT_LIT { .writeidx = 0, .readidx = 0, .lock = SPIN_LOCK_UNLOCKED_LIT }
#define BYTE_RING_BUFFER_INIT (ByteRingBuffer) BYTE_RING_BUFFER_INIT_LIT

__attribute__((unused)) static inline bool
brb_write(ByteRingBuffer *rb, char value)
{
	spin_lock_acquire(&rb->lock);  // Acquire
	uint8_t lastidx = rb->readidx - 1;
	if (lastidx == rb->writeidx) {
		spin_lock_release(&rb->lock);  // Release
		return false;
	}
	rb->data[rb->writeidx++] = value;
	spin_lock_release(&rb->lock);  // Release
	return true;
}

__attribute__((unused)) static inline bool
brb_read(ByteRingBuffer *rb, char *outvalue)
{
	spin_lock_acquire(&rb->lock);  // Acquire
	if (rb->readidx == rb->writeidx) {
		spin_lock_release(&rb->lock);  // Release
		return false;
	}
	*outvalue = rb->data[rb->readidx++];
	spin_lock_release(&rb->lock);  // Release
	return true;
}

__attribute__((unused)) static inline bool
brb_is_empty(ByteRingBuffer *rb)
{
	spin_lock_acquire(&rb->lock);  // Acquire
	bool result = rb->readidx == rb->writeidx;
	spin_lock_release(&rb->lock);  // Release
	return result;
}

__attribute__((unused)) static inline bool
brb_is_full(ByteRingBuffer *rb)
{
	spin_lock_acquire(&rb->lock);  // Acquire
	bool result = (uint8_t) (rb->readidx - 1) == rb->writeidx;
	spin_lock_release(&rb->lock);  // Release
	return result;
}

#endif /* end of include guard: SRC_RINGBUF_H_ */
