#include "timer.h"

#include "contexts.h"
#include "print.h"
#include "panic.h"
#include "guestprivilege.h"

#pragma pack(push, 0)
typedef struct {
  uint32_t msip;
  const uint8_t _pad0[0x4000 - sizeof(uint32_t)];
  uint64_t mtimecmp;
  const uint8_t _pad1[0xBFF8 - sizeof(uint64_t) - 0x4000];
  uint64_t mtime;
} ClintRegisters;
#pragma pack(pop)

volatile ClintRegisters* clint0 = (volatile ClintRegisters*)0x02000000;
GuestThreadId interrupt_target = {-1, -1};  // TODO move into HostThreadData

uint64_t timer_get_time(void)
{
	return clint0->mtime;
}

uint64_t timer_get_time_virtual(void)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	return guest_thr->timer_suspended_at - guest_thr->timer_adjustment;
}

void timer_interrupt_at(uint64_t deadline)
{
	clint0->mtimecmp = deadline;
	w_mie(r_mie() | MIE_MTIE);
}

bool timer_interrupt_sooner(uint64_t deadline)
{
	uint64_t old_deadline = clint0->mtimecmp;
	switch (timer_compare(deadline, old_deadline)) {
	case CMP_LT:
		clint0->mtimecmp = deadline;
		w_mie(r_mie() | MIE_MTIE);
		return true;
	default:
		return false;
	}
}

void timer_reschedule(void)
{
	interrupt_target = (GuestThreadId) {-1, -1};
	timer_interrupt_at(timer_get_time() + MILLISECONDS(200));
	for (int i = 0; i < MAX_GUESTS; ++i) {
		for (int j = 0; j < MAX_VIRT_HARTS; ++j) {
			GuestThreadContext *guest_thr = &guest_threads[i][j];
			if (guest_thr->timer_retry) {
				timer_interrupt_at(guest_thr->timer_deadline + guest_thr->timer_adjustment);
				interrupt_target = (GuestThreadId) {i, j};
			} else if (guest_thr->timer_scheduled) {
				if (timer_interrupt_sooner(guest_thr->timer_deadline + guest_thr->timer_adjustment)) {
					interrupt_target = (GuestThreadId) {i, j};
				}
			}
		}
	}
}

void timer_on_interrupt(void)
{
	if (interrupt_target.machine < 0) {
		return;  // TODO
	}
	HostThreadData *host_thr = get_host_thread_address();
	if (host_thr->current_guest.machine != interrupt_target.machine || host_thr->current_guest.thread != interrupt_target.thread) {
		print_string("TODO: Guest machine/thread switch");
		panic();
	}
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	guest_thr->timer_scheduled = false;
	guest_thr->timer_retry = false;

	if (!guest_defer_exception(MCAUSE_ASYNC_BIT | MCAUSE_ASYNC_TIMER)) {
		guest_thr->timer_retry = true;
	}
}

void timer_suspend_virtual(void)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	guest_thr->timer_suspended_at = clint0->mtime;
}

void timer_resume_virtual(void)
{
	const int additional_cycles = 50;  // lower estimate of how many cycles is spent between the interrupt and timer_suspend_virtual + timer_resume_virtual and mret

	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	guest_thr->timer_adjustment += clint0->mtime - guest_thr->timer_suspended_at + additional_cycles;
}
