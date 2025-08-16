#include "guestprivilege.h"

#include "contexts.h"
#include "print.h"
#include "panic.h"

static void apply_satp(GuestThreadContext *guest_thr)
{
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
}

void guest_mret(void)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	PrivilegeLevel source_level = guest_thr->privelege_level;
	// TODO differentiate MRET and SRET
	if (source_level == PL_USER) {
		print_string("\nmret from user mode");
		panic();
	}
	PrivilegeLevel target_level = guest_thr->csr.mstatus_mpp;
	if (source_level == PL_SUPER) {
		target_level = guest_thr->csr.sstatus_spp ? PL_SUPER : PL_USER;
	}
	if (target_level > guest_thr->privelege_level) {
		print_string("\nmret into a higher privilege level");
		panic();
	}
	if (target_level == PL_HYPER) {
		print_string("\nmret into hypervisor mode");
		panic();
	}
	guest_thr->privelege_level = target_level;
	if (source_level == PL_MACHINE) {
		w_mepc(guest_thr->csr.mepc);
	} else {
		w_mepc(guest_thr->csr.sepc);
	}
	if (source_level == PL_MACHINE && target_level != PL_MACHINE) {
		guest_thr->csr.mstatus_mdt = false;
		apply_satp(guest_thr);
	}
	switch (source_level) {
	case PL_MACHINE:
		guest_thr->csr.mstatus_mie = guest_thr->csr.mstatus_mpie;
		guest_thr->csr.mstatus_mpie = true;
		break;
	case PL_SUPER:
		guest_thr->csr.sstatus_sie = guest_thr->csr.sstatus_spie;
		guest_thr->csr.sstatus_spie = true;
		break;
	default:
		break;
	}
	guest_check_deferred();
}

static PrivilegeLevel get_exception_target_mode(uint64_t mcause, PrivilegeLevel *source_level_out)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	bool isAsync = (mcause & MCAUSE_ASYNC_BIT) != 0;
	uint64_t delegMask = 1 << (mcause & 63);
	PrivilegeLevel source_level = guest_thr->privelege_level;
	PrivilegeLevel target_level = PL_MACHINE;
	if (source_level_out) {
		*source_level_out = source_level;
	}
	uint64_t delegValue = isAsync ? guest_thr->csr.mideleg : guest_thr->csr.medeleg;
	if ((delegValue & delegMask) && source_level != PL_MACHINE && guest_thr->csr.sstatus_sie) {
		target_level = PL_SUPER;
	}
	return target_level;
}

void guest_exception(uint64_t mcause)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	PrivilegeLevel source_level;
	PrivilegeLevel target_level = get_exception_target_mode(mcause, &source_level);
	switch (target_level) {
	case PL_MACHINE:
		if (guest_thr->shadow_pt_active) {
			guest_thr->shadow_pt_active = false;
			w_satp(MAKE_SATP(&machine_pagetable_roots[host_thr->current_guest.machine][0]));
			vmem_fence(NULL, NULL);
		}
		guest_thr->csr.mstatus_mdt = true;
		guest_thr->csr.mstatus_mpie = guest_thr->csr.mstatus_mie;
		guest_thr->csr.mstatus_mie = false;
		guest_thr->csr.mepc = r_mepc();
		w_mepc(guest_thr->csr.mtvec);
		guest_thr->privelege_level = PL_MACHINE;
		guest_thr->csr.mcause = mcause;
		guest_thr->csr.mtval = r_mtval();
		break;
	case PL_SUPER:
		guest_thr->csr.sstatus_spie = guest_thr->csr.sstatus_sie;
		guest_thr->csr.sstatus_sie = false;
		guest_thr->csr.sepc = r_mepc();
		w_mepc(guest_thr->csr.stvec);
		guest_thr->privelege_level = PL_SUPER;
		guest_thr->csr.scause = mcause;
		guest_thr->csr.stval = r_mtval();
		break;
	default:
		panic();
	}
}

bool guest_defer_exception(uint64_t mcause)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	if (guest_thr->deferred_exception) {
		return false;  // Cannot defer additional ones
	}
	guest_thr->deferred_exception = true;
	guest_thr->deferred_mcause = mcause;
	guest_check_deferred();
	return true;
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

void guest_on_satp_write(void)
{
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadContext *guest_thr = &guest_threads[host_thr->current_guest.machine][host_thr->current_guest.thread];
	if (guest_thr->privelege_level != PL_MACHINE) {
		apply_satp(guest_thr);
	}
}
