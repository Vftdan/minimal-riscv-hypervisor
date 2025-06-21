#include "vmem.h"

// #include <assert.h>
#include <stdalign.h>
#include "print.h"
#include "panic.h"
#include "pagealloc.h"
#include "contexts.h"
#include "instructions.h"
#include "virtmmdev.h"

PagetablePage machine_pagetable_roots[MAX_GUESTS] = {};

_Static_assert(_Alignof(typeof(machine_pagetable_roots[0])) == 4096, "Page table type is not properly aligned");
_Static_assert(sizeof(typeof(machine_pagetable_roots[0])) == 4096, "Page table type is of incorrect size");
// _Static_assert((uintptr_t)((PagetablePage*)(0) + 1) == 4096, "Page table type is not properly aligned or sized");

static uintptr_t parse_source_address(PackedInstruction *instr_ptr, HostThreadData *ctx, MemoryAccessWidth *width_out, int *reg_out, int *pc_advance_out, bool *load_is_signed_out)
{
	if (load_is_signed_out) {
		*load_is_signed_out = false;
	}
	PackedInstruction packed = dereference_instruction(instr_ptr);
	if ((packed.numeric_value & 3) == 3) {
		if (pc_advance_out) {
			*pc_advance_out = 4;
		}
		UnpackedInstruction unpacked = unpack_instruction(packed);

		// if (r_mepc() == 0x80000766) goto fail;
		switch (unpacked.opcode) {
		case 0b0000011: {
				int addr_reg = unpacked.rs1;
				int data_reg = unpacked.rd;
				int imm = EXTEND_SIGN((unsigned) (unpacked.funct7 << 5) | unpacked.rs2, 12);
				uintptr_t reg_value = addr_reg ? ctx->active_regs.x_plus_one[addr_reg - 1] : 0;
				uintptr_t addr = reg_value + imm;
				if (reg_out) {
					*reg_out = data_reg;
				}
				switch (unpacked.funct3) {
				case 0:  // int8_t
					if (load_is_signed_out) {
						*load_is_signed_out = true;
					}
					// FALLTHROUGH
				case 4:  // uint8_t
					if (width_out) {
						*width_out = MAW_8BIT;
					}
					return addr;
				case 1:  // int16_t
					if (load_is_signed_out) {
						*load_is_signed_out = true;
					}
					// FALLTHROUGH
				case 5:  // uint16_t
					if (width_out) {
						*width_out = MAW_16BIT;
					}
					return addr;
				case 2:  // int32_t
					if (load_is_signed_out) {
						*load_is_signed_out = true;
					}
					// FALLTHROUGH
				case 6:  // uint32_t
					if (width_out) {
						*width_out = MAW_32BIT;
					}
					return addr;
				case 3:  // int64_t | uint64_t
					if (width_out) {
						*width_out = MAW_64BIT;
					}
					return addr;
				}
			}
		}

// fail:
		print_string("\nExtract source address\ninstruction = ");
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
	} else {
		if (pc_advance_out) {
			*pc_advance_out = 2;
		}
		UnpackedCompressedInstruction unpacked = unpack_compressed_instruction(packed);
		int funct3 = unpacked.funct4 >> 1;

		switch (unpacked.opcode) {
		case 0: {
				int reg1 = 8 + (unpacked.rs2 & 7);  // data
				int reg2 = 8 + (unpacked.rs1 & 7);  // address
				int imm_6 = (unpacked.rs2 >> 3) & 1;
				int imm_2_or_7 = (unpacked.rs2 >> 4);
				int imm_5 = unpacked.funct4 & 1;
				int imm_4 = unpacked.rs1 >> 4;
				int imm_3_or_8 = (unpacked.rs1 >> 3) & 1;
				uintptr_t reg_value = ctx->active_regs.x_plus_one[reg2 - 1];
				if (reg_out) {
					*reg_out = reg1;
				}
				switch (funct3) {
					int imm;
				case 2:  // C.LW
					if (width_out) {
						*width_out = MAW_32BIT;
					}
					if (load_is_signed_out) {
						// I haven't found it being said explicity
						// But given that it's named `c.lw`, and not `c.lwu` I assume that it's signed
						*load_is_signed_out = true;
					}
					// Immediate is zero-extended
					imm = (imm_5 << 5) | (imm_4 << 4) | (imm_3_or_8 << 3) | (imm_2_or_7 << 2) | (imm_6 << 6);
					return reg_value + imm;
				case 3:  // C.LD
					if (width_out) {
						*width_out = MAW_64BIT;
					}
					// Immediate is zero-extended
					imm = (imm_5 << 5) | (imm_4 << 4) | (imm_3_or_8 << 3) | (imm_2_or_7 << 7) | (imm_6 << 6);
					return reg_value + imm;
				}
			}
		}

		print_string("\nExtract source address\ninstruction = ");
		print_addr(packed.compressed_numeric_value);
		print_string("\ncompressed opcode = ");
		print_addr(unpacked.opcode);
		print_string("\nrs2 = ");
		print_addr(unpacked.rs2);
		print_string("\nrs1 = ");
		print_addr(unpacked.rs1);
		print_string("\nfunct4 = ");
		print_addr(unpacked.funct4);
		panic();
	}
}

static uintptr_t parse_destination_address(PackedInstruction *instr_ptr, HostThreadData *ctx, MemoryAccessWidth *width_out, int *reg_out, int *pc_advance_out)
{
	PackedInstruction packed = dereference_instruction(instr_ptr);
	if ((packed.numeric_value & 3) == 3) {
		if (pc_advance_out) {
			*pc_advance_out = 4;
		}
		UnpackedInstruction unpacked = unpack_instruction(packed);

		switch (unpacked.opcode) {
		case 0b0100011: {
				int addr_reg = unpacked.rs1;
				int data_reg = unpacked.rs2;
				int imm = EXTEND_SIGN((unsigned) (unpacked.funct7 << 5) | unpacked.rd, 12);
				uintptr_t reg_value = addr_reg ? ctx->active_regs.x_plus_one[addr_reg - 1] : 0;
				uintptr_t addr = reg_value + imm;
				if (reg_out) {
					*reg_out = data_reg;
				}
				switch (unpacked.funct3) {
				case 0:
					if (width_out) {
						*width_out = MAW_8BIT;
					}
					return addr;
				case 1:
					if (width_out) {
						*width_out = MAW_16BIT;
					}
					return addr;
				case 2:
					if (width_out) {
						*width_out = MAW_32BIT;
					}
					return addr;
				case 3:
					if (width_out) {
						*width_out = MAW_64BIT;
					}
					return addr;
				}
			}
		}

		print_string("\nExtract destination address\ninstruction = ");
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
	} else {
		if (pc_advance_out) {
			*pc_advance_out = 2;
		}
		UnpackedCompressedInstruction unpacked = unpack_compressed_instruction(packed);
		int funct3 = unpacked.funct4 >> 1;

		switch (unpacked.opcode) {
		case 0: {
				int reg1 = 8 + (unpacked.rs2 & 7);  // data
				int reg2 = 8 + (unpacked.rs1 & 7);  // address
				int imm_6 = (unpacked.rs2 >> 3) & 1;
				int imm_2_or_7 = (unpacked.rs2 >> 4);
				int imm_5 = unpacked.funct4 & 1;
				int imm_4 = unpacked.rs1 >> 4;
				int imm_3_or_8 = (unpacked.rs1 >> 3) & 1;
				uintptr_t reg_value = ctx->active_regs.x_plus_one[reg2 - 1];
				if (reg_out) {
					*reg_out = reg1;
				}
				switch (funct3) {
					int imm;
				case 6:  // C.SW
					if (width_out) {
						*width_out = MAW_32BIT;
					}
					// Immediate is zero-extended
					imm = (imm_5 << 5) | (imm_4 << 4) | (imm_3_or_8 << 3) | (imm_2_or_7 << 2) | (imm_6 << 6);
					return reg_value + imm;
				case 7:  // C.SD
					if (width_out) {
						*width_out = MAW_64BIT;
					}
					// Immediate is zero-extended
					imm = (imm_5 << 5) | (imm_4 << 4) | (imm_3_or_8 << 3) | (imm_2_or_7 << 7) | (imm_6 << 6);
					return reg_value + imm;
				}
			}
		}

		print_string("\nExtract destination address\ninstruction = ");
		print_addr(packed.compressed_numeric_value);
		print_string("\ncompressed opcode = ");
		print_addr(unpacked.opcode);
		print_string("\nrs2 = ");
		print_addr(unpacked.rs2);
		print_string("\nrs1 = ");
		print_addr(unpacked.rs1);
		print_string("\nfunct4 = ");
		print_addr(unpacked.funct4);
		panic();
	}
}

static void reset_upper_bits(int64_t *reg_ptr, MemoryAccessWidth width, bool is_signed)
{
	if (!reg_ptr) {
		return;
	}
	switch (width) {
	case MAW_8BIT:
		*reg_ptr = is_signed ? (int64_t)*(int8_t*) reg_ptr : (int64_t)(uint64_t)*(uint8_t*) reg_ptr;
		return;
	case MAW_16BIT:
		*reg_ptr = is_signed ? (int64_t)*(int16_t*) reg_ptr : (int64_t)(uint64_t)*(uint16_t*) reg_ptr;
		return;
	case MAW_32BIT:
		*reg_ptr = is_signed ? (int64_t)*(int32_t*) reg_ptr : (int64_t)(uint64_t)*(uint32_t*) reg_ptr;
		return;
	case MAW_64BIT:
		return;
	}
}

PageFaultHandlerResult handle_page_fault(MempermIndex access_type, uintptr_t *virt_out)
{
	HostThreadData *ctx = get_host_thread_address();
	uintptr_t instr_addr = r_mepc();
	uintptr_t fault_addr, fault_gm_addr;
	MemoryAccessWidth rw_width;
	int rw_reg = 0;
	int pc_advance = 4;
	bool load_is_signed = false;
	PackedInstruction *instr_host_addr = NULL;
	if (access_type == PERMIDX_R || access_type == PERMIDX_W) {
		GuestSliceIterator it = begin_iter_guestmem((GuestSlice) {
			.virtual_ptr = instr_addr,
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
		if ((*(uint16_t*) it.host.address & 3) != 3) {
			pc_advance = 2;
		}
		if (it.host.byte_length < (unsigned) pc_advance) {
			print_string("\nMulti-page instruction");
			panic();
		}
		instr_host_addr = it.host.address;
	}
	switch (access_type) {
	case PERMIDX_R:
		fault_addr = parse_source_address(instr_host_addr, ctx, &rw_width, &rw_reg, &pc_advance, &load_is_signed);
		break;
	case PERMIDX_W:
		fault_addr = parse_destination_address(instr_host_addr, ctx, &rw_width, &rw_reg, &pc_advance);
		break;
	case PERMIDX_X:
		fault_addr = instr_addr;
		rw_width = 4;
		break;
	default:
		print_string("\nInvalid page fault type");
		panic();
	}
	(void) rw_width;  // TODO handle accesses at page boundary
	if (virt_out) {
		*virt_out = fault_addr;
	}

	{
		GuestThreadContext *guest_ctx = &guest_threads[ctx->current_guest.machine][ctx->current_guest.thread];
		if (!guest_ctx->shadow_pt_active) {
			fault_gm_addr = fault_addr;
		} else {
			print_string("\nGU page fault");
			panic();  // TODO
		}
	}

	if (fault_gm_addr < 0x80000000) {
		// Devices are mapped below RAM
		uint64_t *reg_ptr = rw_reg ? &ctx->active_regs.x_plus_one[rw_reg - 1] : NULL;
		switch (access_type) {
		case PERMIDX_R:
			if (virtual_mmdev_load(fault_gm_addr, reg_ptr, rw_width) == VMMAR_SUCCESS) {
				reset_upper_bits((int64_t*) reg_ptr, rw_width, load_is_signed);  // If I understand correctly, lw and lwu reset the upper bits
				w_mepc(instr_addr + pc_advance);
				return PFHR_SUCCESS;
			}
			break;
		case PERMIDX_W:
			if (virtual_mmdev_store(fault_gm_addr, reg_ptr, rw_width) == VMMAR_SUCCESS) {
				w_mepc(instr_addr + pc_advance);
				return PFHR_SUCCESS;
			}
			break;
		default:
			break;
		}
		return PFHR_TOO_LOW;
	}
	uint64_t lvl3 = fault_gm_addr >> 30;
	if (lvl3 >= 512) {
		return PFHR_TOO_HIGH;
	}

	// Level 3 entries don't seem to allow to resolve directly, so we also handle level 2
	uint64_t lvl2 = (fault_gm_addr >> 21) & 0x1FF;
	// TODO use a mutex to avoid concurrent modification of page tables
	UnpackedPagetableEntry unpacked = unpack_pt_entry(machine_pagetable_roots[ctx->current_guest.machine][lvl3]);
	if (!(unpacked.permissions & PERMBIT(V))) {  // If there is no entry at the top-level table
		// We only have reserved a fixed address for top-level (level 3) of the hierarchical page table
		// So the child level (level 2) is allocated dynamically
		// MemoryPage *page = allocate_page();
		// if (!page) {
		// 	print_string("\nFailed to allocate a page");
		// 	panic();
		// }
		// unpacked.resolved_range_start = page;  // The same pointer as .child_table, but of a non-specific MemoryPage type
		unpacked.child_table = allocate_pagepable();
		if (!unpacked.child_table) {
			print_string("\nFailed to allocate a page");
			panic();
		}
		// for (int i = 0; i < 512; ++i) {
		// 	// Fill the new page with zeros, because its contents may be uninitialized or contain old data
		// 	(*unpacked.child_table)[i] = (PackedPagetableEntry) {};
		// }
		unpacked.permissions = PERMBIT(V);
		// Now store the child in the page table in the correct format
		// TODO at this moment other harts should not read this entry neither from hypervisor code nor through a MMU
		//      the virtual machine might need to be interrupted, except for the harts that currently use another page table
		machine_pagetable_roots[ctx->current_guest.machine][lvl3] = pack_pt_entry(unpacked);
		print_string("[Lazy-initializing level 3 page of ");
		print_addr(lvl3 << 30);
		print_string("]");
	}
	PackedPagetableEntry *packed_ptr = &(*unpacked.child_table)[lvl2];
	unpacked = unpack_pt_entry(*packed_ptr);  // REASSIGN unpacked to the child level entry
	if (unpacked.permissions & PERMBIT(V)) {
		return PFHR_NOT_CHANGED;
	}
	// Base physical address of the mapping being initialized
	unpacked.numeric_address = ((lvl3 << 30) | (lvl2 << 21)) + GUEST_MEMORY_OFFSET;
	// Make sure to not set the G bit, because it might cause an undefined behavior when there are multiple harts
	unpacked.permissions = PERMBIT(V) | PERMBIT(U) | PERMBIT(R) | PERMBIT(W) | PERMBIT(X);
	*packed_ptr = pack_pt_entry(unpacked);  // Write the child-level entry
	print_string("[Lazy-initializing level 2 page of ");
	print_addr((lvl3 << 30) | (lvl2 << 21));
	print_string("]");
	vmem_fence(NULL, NULL);
	return PFHR_SUCCESS;
}

PagetablePage *allocate_pagepable(void)
{
	union {
		PagetablePage *table;
		MemoryPage *page;
	} allocated;
	allocated.page = allocate_page();
	if (!allocated.page) {
		return NULL;
	}
	for (int i = 0; i < 512; ++i) {
		// Fill the new page with zeros, because its contents may be uninitialized or contain old data
		(*allocated.table)[i] = (PackedPagetableEntry) {};
	}
	return allocated.table;
}

void deallocate_pagepable(PagetablePage *subtree)
{
	if (!subtree) {
		return;
	}
	for (int i = 0; i < 512; ++i) {
		UnpackedPagetableEntry unpacked = unpack_pt_entry((*subtree)[i]);
		(*subtree)[i] = (PackedPagetableEntry) {};
		if (!(unpacked.permissions & PERMBIT(V))) {
			// Empty
			continue;
		}
		if (unpacked.permissions & (PERMBIT(R) | PERMBIT(V) | PERMBIT(X))) {
			// Does not store another level
			continue;
		}
		deallocate_pagepable(unpacked.child_table);
	}
	deallocate_page((MemoryPage*) subtree);
}

void resolve_guestmem_slice(GuestSliceIterator *it)
{
	GuestThreadContext *guest_ctx = &guest_threads[it->guest.thid.machine][it->guest.thid.thread];
	PagetablePage *active_pt;
	{
		PagetablePage *machine_pt_root = &machine_pagetable_roots[it->guest.thid.machine];
		PagetablePage *shadow_pt_root = guest_ctx->shadow_page_table;
		active_pt = guest_ctx->shadow_pt_active ? shadow_pt_root : machine_pt_root;
		if (!active_pt) {
			print_string("\nNull page table");
			panic();
		}
	}
	if (!active_pt) {
		print_string("\nNull page table");
		panic();
	}
	// pid_t proc = it->user.pid;
	uint64_t virtaddr = it->guest.address.virtual_ptr + it->offset;
	// uint64_t pgoffset = virtaddr & PGOFFSET_MASK;
	uint64_t remaining_length = it->guest.byte_length - it->offset;
	if (remaining_length == 0) {
		// Always succeed
		it->success = true;
		it->host = (HostSlice) {
			.address = NULL,
			.byte_length = 0,
		};
		return;
	}
	if (remaining_length > it->guest.byte_length) {
		print_string("\nVirtual memory iterator out of range, size = ");
		print_addr(it->guest.byte_length);
		print_string(", offset = ");
		print_addr(it->offset);
		panic();
	}

	// UnpackedPagetableEntry entry = {{pagetables[proc]}, PERMBIT(V)};
	UnpackedPagetableEntry entry = { .child_table = active_pt, .permissions = PERMBIT(V) };
	int component_offset = PGSHIFT + 9 + 9;
	while (true) {
		if (!(entry.permissions & PERMBIT(V))) {
			// print_string("\nAttempting to access an unmapped page from kernel, guest = ");
			// print_addr(proc);
			// print_string(", virtual address = ");
			// print_addr(virtaddr);
			// print_string("\n");
			print_string("\nUnmapped page");
			panic();
		}
		if (entry.permissions & (PERMBIT(R) | PERMBIT(W) | PERMBIT(X))) {
			if (!(entry.permissions & PERMBIT(U))) {
				print_string("\nNon-guest page");
				panic();
				// print_string("Attempting to access a non-user-accessible page from kernel, pid = ");
				// print_addr(proc);
				// print_string(", virtual address = ");
				// print_addr(virtaddr);
				// print_string(", physical address = ");
				// print_addr((uint64_t) entry.address);
				// print_string("\n");
				// goto fail;
			}
			// Host address
			uint64_t pgsize = 1 << (component_offset + 9);
			uint64_t pgoffset = virtaddr & (pgsize - 1);
			uint64_t provided_length = pgsize - pgoffset;
			if (remaining_length < provided_length) {
				provided_length = remaining_length;
			}
			it->success = true;
			it->host = (HostSlice) {
				.address = (void*) entry.numeric_address + pgoffset,
				.byte_length = provided_length,
			};
			return;
		} else {
			// Next page
			if (component_offset <= PGSHIFT) {
				print_string("\nNo RWX bits at the third-level pagetable entry");
				panic();
			}
			int vpn_i = 511 & (virtaddr >> component_offset);
			component_offset -= 9;
			entry = unpack_pt_entry((*entry.child_table)[vpn_i]);
		}
	}

	print_string("\nVirtual memory resolution failure");
	panic();
}
