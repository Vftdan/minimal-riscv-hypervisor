#include "virtcsr.h"

#include <stdbool.h>
#include "print.h"
#include "panic.h"
#include "contexts.h"
#include "instructions.h"  // Bit manipulation

uint64_t get_virtual_csr(CSRNumber csr_id)
{
	char *csr_name = "unknown";
	switch (csr_id) {
#define DECLARE_CSR(num, name) case CSR_##name: csr_name = #name; break;
#include "csrs.cc"
#undef DECLARE_CSR
	}
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadId guest_thid = host_thr->current_guest;
	GuestThreadContext *guest_thr = &guest_threads[guest_thid.machine][guest_thid.thread];

	switch (csr_id) {
	case CSR_mstatus: {
			uint64_t result = 0;
			uint64_t uxl = 2;  // Guest user mode is always 64-bit
			result |= uxl << 32;
			result |= guest_thr->csr.mstatus_mpp << 11;
			if (guest_thr->csr.mstatus_mie) {
				result |= MSTATUS_MIE;
			}
			if (guest_thr->csr.mstatus_mdt) {
				result |= MSTATUS_MDT;
			}
			return result;
		}
		break;
	case CSR_mie: {
			uint64_t result = 0;
			if (guest_thr->csr.mie_msie) {
				result |= MIE_MSIE;
			}
			if (guest_thr->csr.mie_mtie) {
				result |= MIE_MTIE;
			}
			if (guest_thr->csr.mie_meie) {
				result |= MIE_MEIE;
			}
			return result;
		}
		break;
	case CSR_mtvec: {
			return guest_thr->csr.mtvec;
		}
		break;
	case CSR_satp: {
			uint64_t result = 0;
			uint64_t mode = guest_thr->csr.satp_mode;
			uint64_t ppn = guest_thr->csr.satp_ppn;
			result |= mode << 60;
			result |= ppn;
			return result;
		}
		break;
	case CSR_pmpcfg0 ... CSR_pmpcfg15:
	case CSR_pmpaddr0 ... CSR_pmpaddr63: {
			return 0;
		}
		break;
	case CSR_mepc: {
			return guest_thr->csr.mepc;
		}
		break;
	case CSR_mscratch: {
			return guest_thr->csr.mscratch;
		}
		break;
	case CSR_mcause: {
			return guest_thr->csr.mcause;
		}
		break;
	case CSR_mtval: {
			return guest_thr->csr.mtval;
		}
		break;
	case CSR_mhartid: {
			return guest_thid.thread;
		}
		break;
	case CSR_medeleg: {
			return guest_thr->csr.medeleg;
		}
		break;
	case CSR_mideleg: {
			return guest_thr->csr.mideleg;
		}
		break;
	case CSR_mip: {
			return 0;  //TODO
		}
		break;
	default:
		print_string("\nGet emulated csr: ");
		print_string(csr_name);
		print_string(" (");
		print_addr(csr_id);
		print_string(")");
		panic();
	}
}

void set_virtual_csr(CSRNumber csr_id, uint64_t value)
{
	char *csr_name = "unknown";
	switch (csr_id) {
#define DECLARE_CSR(num, name) case CSR_##name: csr_name = #name; break;
#include "csrs.cc"
#undef DECLARE_CSR
	}
	HostThreadData *host_thr = get_host_thread_address();
	GuestThreadId guest_thid = host_thr->current_guest;
	GuestThreadContext *guest_thr = &guest_threads[guest_thid.machine][guest_thid.thread];

	switch (csr_id) {
	case CSR_mstatus: {
			bool should_panic = false;
			guest_thr->csr.mstatus_mpp = (value >> 11) & 3;
			guest_thr->csr.mstatus_mie = !!(value & MSTATUS_MIE);
			if (value & MSTATUS_MDT) {
				guest_thr->csr.mstatus_mdt = true;
			}
			// We seem to need to unset MDT if MIE is being set, but unsetting MDT not being atomic with mret sounds dangerous
			if (should_panic) {
				panic();
			}
		}
		break;
	case CSR_mie: {
			guest_thr->csr.mie_msie = !!(value & MIE_MSIE);
			guest_thr->csr.mie_mtie = !!(value & MIE_MTIE);
			guest_thr->csr.mie_meie = !!(value & MIE_MEIE);
			value &= ~(MIE_MSIE | MIE_MTIE | MIE_MEIE);
			if (value) {
				print_string("\nUnhandled mie fields: ");
				print_addr(value);
				panic();
			}
		}
		break;
	case CSR_mtvec: {
			guest_thr->csr.mtvec = value;
		}
		break;
	case CSR_satp: {
			guest_thr->csr.satp_ppn = EXTRACT_BITS(value, 0, 44);
			guest_thr->csr.satp_mode = EXTRACT_BITS(value, 60, 4);
			// Ignore satp.ASID
		}
		break;
	case CSR_pmpcfg0 ... CSR_pmpcfg15:
	case CSR_pmpaddr0 ... CSR_pmpaddr63:
		break;  // Ignore
	case CSR_mepc: {
			guest_thr->csr.mepc = value;
		}
		break;
	case CSR_mscratch: {
			guest_thr->csr.mscratch = value;
		}
		break;
	case CSR_mcause: {
			guest_thr->csr.mcause = value;
		}
		break;
	case CSR_mtval: {
			guest_thr->csr.mtval = value;
		}
		break;
	case CSR_medeleg: {
			guest_thr->csr.medeleg = value;
		}
		break;
	case CSR_mideleg: {
			guest_thr->csr.mideleg = value;
		}
		break;
	case CSR_mip: {
			(void) value;  //TODO
		}
		break;
	default:
		print_string("\nSet emulated csr: ");
		print_string(csr_name);
		print_string(" (");
		print_addr(csr_id);
		print_string(")");
		print_string("\nvalue = ");
		print_addr(value);
		panic();
	}
}
