#include "exchandlers.h"

#include "csr.h"
#include "hypervisor.h"
#include "print.h"
#include "panic.h"
#include "vmem.h"
#include "pagealloc.h"
#include "contexts.h"
#include "instructions.h"

void handle_interrupt(uint64_t mcause)
{
	switch (mcause & ~MCAUSE_ASYNC_BIT) {
	case 7: {
			// TODO reschedule the timer (clint)
			// TODO handle timer interrupt
		}
		break;
	case 11: {
			// TODO handle external interrupt (plic)
		}
		break;
	default:
		;  // TODO handle unsupported interrupt
	}
}

void handle_hypervisor_exception(uint64_t mcause)
{
	// Exception caused by the hypervisor itself
	print_string("\nHypervisor-caused exception\nmcause = ");
	print_addr(mcause);
	panic();
}

void handle_guest_exception(uint64_t mcause)
{
	HostThreadData *ctx = get_host_thread_address();
	switch (mcause) {
		// TODO emulate the instruction when should be allowed
		//      otherwise, forward the error to the guest
	case 2: {  // Illegal instruction
			uintptr_t guest_addr = r_mepc();
			if (guest_addr < 0x80000000 || guest_addr > 0x8080000000) {
				print_string("\nGuest illegal instruction not in memory range");
				panic();
			}
			uintptr_t host_addr = guest_addr + GUEST_MEMORY_OFFSET;
			PackedInstruction packed = *(PackedInstruction*) host_addr;
			UnpackedInstruction unpacked = unpack_instruction(packed);
			if (unpacked.opcode == 0x73) {
				// ecall, ebreak, csr manipulation
				int csr_id = (unpacked.funct7 << 5) | unpacked.rs2;
				char *csr_name = "unknown";
				switch (csr_id) {
#define DECLARE_CSR(num, name) case CSR_##name: csr_name = #name; break;
#include "csrs.cc"
#undef DECLARE_CSR
				}
				switch (unpacked.funct3) {
				case 0: {
						print_string("\nGuest ecall/ebreak");
						panic();
					}
					break;
				case 2: {
						// csrr
						print_string("\nGuest csrr: ");
						print_string(csr_name);
						print_string("\nsaved to x[");
						print_addr(unpacked.rd);
						print_string("]");
						panic();
					}
					break;
				}
			}
			print_string("\nGuest illegal instruction\ninstruction = ");
			print_addr(packed.numeric_value);
			print_string("\nopcode = ");
			print_addr(unpacked.opcode);
			print_string("\nrd = ");
			print_addr(unpacked.rd);
			print_string("\nfunct3 = ");
			print_addr(unpacked.funct3);
			print_string("\nrs1 = ");
			print_addr(unpacked.rs1);
			print_string("\nrs2 = ");
			print_addr(unpacked.rs2);
			print_string("\nfunct7 = ");
			print_addr(unpacked.funct7);
			panic();
		}
		break;
	case 12: {  // Instruction page fault
			uint64_t addr = r_mepc();
			if (addr < 0x80000000) {
				print_string("\nGuest program counter below RAM");
				panic();
			}
			uint64_t lvl3 = addr >> 30;
			if (lvl3 >= 512) {
				print_string("\nGuest program counter beyond Sv39");
				panic();
			}
			// Level 3 entries don't seem to allow to resolve directly, so we also handle level 2
			uint64_t lvl2 = (addr >> 21) & 0x1FF;
			// TODO use a mutex to avoid concurrent modification of page tables
			UnpackedPagetableEntry unpacked = unpack_pt_entry(machine_pagetable_roots[ctx->current_guest.machine][lvl3]);
			if (!(unpacked.permissions & PERMBIT(V))) {  // If there is no entry at the top-level table
				// We only have reserved a fixed address for top-level (level 3) of the hierarchical page table
				// So the child level (level 2) is allocated dynamically
				MemoryPage *page = allocate_page();
				if (!page) {
					print_string("\nFailed to allocate a page");
					panic();
				}
				unpacked.resolved_range_start = page;  // The same pointer as .child_table, but of a non-specific MemoryPage type
				for (int i = 0; i < 512; ++i) {
					// Fill the new page with zeros, because its contents may be uninitialized or contain old data
					(*unpacked.child_table)[i] = (PackedPagetableEntry) {};
				}
				unpacked.permissions = PERMBIT(V);
				// Now store the child in the page table in the correct format
				// TODO at this moment other harts should not read this entry neither from hypervisor code nor through a MMU
				//      the virtual machine might need to be interrupted, except for the harts that currently use another page table
				machine_pagetable_roots[ctx->current_guest.machine][lvl3] = pack_pt_entry(unpacked);
			}
			PackedPagetableEntry *packed_ptr = &(*unpacked.child_table)[lvl2];
			unpacked = unpack_pt_entry(*packed_ptr);  // REASSIGN unpacked to the child level entry
			if (unpacked.permissions & PERMBIT(V)) {
				// We have a fault despite the page having been initialized
				print_string("\nGuest instruction page fault\nMapped to range from: ");
				print_addr(unpacked.numeric_address);
				print_string("\npermissions = ");
				print_addr(unpacked.permissions);
				print_string("\nfirst bytes in the range = ");
				print_addr((*unpacked.child_table)[0].numeric_value);
				print_string("\nguest machine = ");
				print_addr(ctx->current_guest.machine);
				panic();
			}
			// Base physical address of the mapping being initialized
			unpacked.numeric_address = ((lvl3 << 30) | (lvl2 << 21)) + GUEST_MEMORY_OFFSET;
			// Make sure to not set the G bit, because it might cause an undefined behavior when there are multiple harts
			unpacked.permissions = PERMBIT(V) | PERMBIT(U) | PERMBIT(R) | PERMBIT(W) | PERMBIT(X);
			*packed_ptr = pack_pt_entry(unpacked);  // Write the child-level entry
			asm volatile("sfence.vma zero, zero");
		}
		break;
	default: {
			print_string("\nUnhandled guest-caused exception\nmcause = ");
			print_addr(mcause);
			panic();
		}
	}
}
