#include "guestprivilege.h"

#include "contexts.h"
#include "print.h"
#include "panic.h"

void guest_mret(void)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	if (guest_thr->user_mode) {
		print_string("\nmret from user mode");
		panic();
	}
	guest_thr->user_mode = true;
	w_mepc(guest_thr->csr.mepc);
	guest_thr->csr.mstatus_mdt = false;
	switch ((uint64_t) guest_thr->csr.satp_mode << SATP_MODE_SHIFT) {
	case SATP_MODE_NONE:  // GM didn't enable paging
		// Retain the GM -> HM pagetable
		break;
	case SATP_MODE_SV39:
		guest_thr->shadow_pt_active = true;
		if (!guest_thr->shadow_page_table) {
			guest_thr->shadow_page_table = allocate_pagepable();
		}
		w_satp(MAKE_SATP(guest_thr->shadow_page_table));
		vmem_fence(NULL, NULL);
		break;
	default:
		print_string("\nUnsupported pagetable mode: ");
		print_addr(guest_thr->csr.satp_mode);
		panic();
	}
	guest_check_deferred();
}

void guest_exception(uint64_t mcause)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	if (!guest_thr->user_mode) {
		print_string("\nGM exception");
		panic();
	}
	if (guest_thr->shadow_pt_active) {
		guest_thr->shadow_pt_active = false;
		w_satp(MAKE_SATP(&machine_pagetable_roots[host_thr->current_guest.machine][0]));
		vmem_fence(NULL, NULL);
	}
	guest_thr->csr.mstatus_mdt = true;
	guest_thr->csr.mepc = r_mepc();
	w_mepc(guest_thr->csr.mtvec);
	guest_thr->user_mode = false;
	guest_thr->csr.mcause = mcause;
	guest_thr->csr.mtval = r_mtval();
}

void guest_defer_exception(uint64_t mcause)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	if (guest_thr->deferred_exception) {
		return;  // Cannot defer additional ones
	}
	guest_thr->deferred_exception = true;
	guest_thr->deferred_mcause = mcause;
	guest_check_deferred();
}

bool guest_check_deferred(void)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	if(!guest_thr->csr.mstatus_mie || guest_thr->csr.mstatus_mdt) {
		return false;
	}
	if (guest_thr->deferred_exception) {
		guest_thr->deferred_exception = false;
		guest_exception(guest_thr->deferred_mcause);
		return true;
	}
	return false;
}
