#include "exchandlers.h"

#include "csr.h"
#include "hypervisor.h"
#include "print.h"
#include "panic.h"
#include "vmem.h"
#include "contexts.h"
#include "instructions.h"
#include "virtcsr.h"
#include "guestprivilege.h"

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
	switch (mcause) {
		// TODO emulate the instruction when should be allowed
		//      otherwise, forward the error to the guest
	case 2: {  // Illegal instruction
			uintptr_t guest_addr = r_mepc();
			if (guest_addr < 0x80000000 || guest_addr > 0x8080000000) {
				print_string("\nGuest illegal instruction not in memory range");
				panic();
			}
			PackedInstruction *host_addr;
			{
				HostThreadData *ctx = get_host_thread_address();
				GuestSliceIterator it = begin_iter_guestmem((GuestSlice) {
					.virtual_ptr = guest_addr,
					.byte_length = 4,
					.thid = ctx->current_guest
				}, PERMBIT(X));
				if (!it.success) {
					print_string("\nFailed to resolve instruction actual address");
					panic();
				}
				if (it.host.byte_length < 2) {
					print_string("\nOdd instruction address");
					panic();
				}
				unsigned int instr_size = 4;
				if ((*(uint16_t*) it.host.address & 3) != 3) {
					instr_size = 2;
				}
				if (it.host.byte_length < instr_size) {
					print_string("\nMulti-page instruction");
					panic();
				}
				host_addr = it.host.address;
			}
			PackedInstruction packed = dereference_instruction(host_addr);
			UnpackedInstruction unpacked = unpack_instruction(packed);
			if (unpacked.opcode == 0x73) {
				// ecall, ebreak, csr manipulation
				CSRNumber csr_id = (unpacked.funct7 << 5) | unpacked.rs2;
				switch (unpacked.funct3) {
				case 0: {
						switch (unpacked.funct7) {
						case 0:
							print_string("\nGuest ecall/ebreak");
							panic();
						case 9:
							if (unpacked.rd == 0) {
								print_string("[Guest sfence.vma]");  // TODO
								vmem_fence(NULL, NULL);
								w_mepc(guest_addr + 4);  // Advance the program counter
								return;
							}
							break;
						case 24:
							if (unpacked.rs2 == 2 && unpacked.rs1 == 0 && unpacked.rd == 0) {
								guest_mret();
								return;
							}
							break;
						}
					}
					break;
				case 2: {
						// csrr
						HostThreadData *ctx = get_host_thread_address();
						uint64_t value = get_virtual_csr(csr_id);
						if (unpacked.rd) {
							ctx->active_regs.x_plus_one[unpacked.rd - 1] = value;
						}
						w_mepc(guest_addr + 4);  // Advance the program counter
						return;
					}
					break;
				case 1: {
						// csrw
						HostThreadData *ctx = get_host_thread_address();
						uint64_t value = unpacked.rs1 ? ctx->active_regs.x_plus_one[unpacked.rs1 - 1] : 0;
						set_virtual_csr(csr_id, value);
						w_mepc(guest_addr + 4);  // Advance the program counter
						return;
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
			print_string("\nvirt2phys(pc) = ");
			print_addr((uintptr_t) host_addr);
			panic();
		}
		break;
	case 12: {  // Instruction page fault
			switch (handle_page_fault(PERMIDX_X, NULL)) {
				UnpackedPagetableEntry unpacked;
				HostThreadData *ctx;
				uintptr_t addr;
				uint64_t lvl2, lvl3;
			case PFHR_SUCCESS:
				break;
			case PFHR_TOO_LOW:
				print_string("\nGuest program counter below RAM");
				panic();
			case PFHR_TOO_HIGH:
				print_string("\nGuest program counter beyond Sv39");
				panic();
			case PFHR_NOT_CHANGED:
				// We have a fault despite the page having been initialized
				// TODO remove duplicate work
				ctx = get_host_thread_address();
				addr = r_mepc();
				lvl3 = addr >> 30;
				lvl2 = (addr >> 21) & 0x1FF;
				unpacked = unpack_pt_entry(machine_pagetable_roots[ctx->current_guest.machine][lvl3]);
				PackedPagetableEntry *packed_ptr = &(*unpacked.child_table)[lvl2];
				unpacked = unpack_pt_entry(*packed_ptr);  // REASSIGN unpacked to the child level entry
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
		}
		break;
	case 13: {  // Load page fault
			uintptr_t virt_addr;
			switch (handle_page_fault(PERMIDX_R, &virt_addr)) {
			case PFHR_SUCCESS:
				break;
			case PFHR_TOO_LOW:
				print_string("\nGuest read below RAM\nsource address = ");
				print_addr(virt_addr);
				panic();
			case PFHR_TOO_HIGH:
				print_string("\nGuest read beyond Sv39\nsource address = ");
				print_addr(virt_addr);
				panic();
			case PFHR_NOT_CHANGED:
				print_string("\nGuest unknown load page fault");
				panic();
			}
		}
		break;
	case 15: {  // Store page fault
			uintptr_t virt_addr;
			switch (handle_page_fault(PERMIDX_W, &virt_addr)) {
			case PFHR_SUCCESS:
				break;
			case PFHR_TOO_LOW:
				print_string("\nGuest store below RAM\ndestination address = ");
				print_addr(virt_addr);
				panic();
			case PFHR_TOO_HIGH:
				print_string("\nGuest store beyond Sv39\ndestination address = ");
				print_addr(virt_addr);
				panic();
			case PFHR_NOT_CHANGED:
				print_string("\nGuest unknown store page fault");
				panic();
			}
		}
		break;
	default: {
			print_string("\nUnhandled guest-caused exception\nmcause = ");
			print_addr(mcause);
			panic();
		}
	}
}
