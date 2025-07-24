#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include "base_types.h"

typedef enum {
	CMP_LT = -1,
	CMP_EQ = 0,
	CMP_GT = 1,
} CompareResult;

#define MILLISECONDS(n) (100 * (n))

uint64_t timer_get_time(void);
uint64_t timer_get_time_virtual(void);
void timer_interrupt_at(uint64_t deadline);
bool timer_interrupt_sooner(uint64_t deadline);
void timer_reschedule(void);
void timer_on_interrupt(void);
void timer_suspend_virtual(void);
void timer_resume_virtual(void);

__attribute__((unused)) inline static CompareResult timer_compare(uint64_t lhs, uint64_t rhs)
{
	uint64_t diff = lhs - rhs;
	if (!diff) {
		return CMP_EQ;
	}
	if ((int64_t) diff < 0) {
		return CMP_LT;
	}
	return CMP_GT;
}

#endif /* end of include guard: SRC_TIMER_H_ */
