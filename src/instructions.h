#ifndef SRC_INSTRUCTIONS_H_
#define SRC_INSTRUCTIONS_H_

#include <stdint.h>

#define EXTRACT_BITS(num, lower, count) (((num) >> (lower)) & ((1ULL << (count)) - 1))
#define EXTEND_SIGN(num, from_bits) (((num) & (1ULL << ((from_bits) - 1))) ? (num) - (1ULL << (from_bits)) : (num))

// Depending on the compiler, might actually be represented exactly the same as the packed one
// In that case, unpack_instruction can be optimized out by the compiler
typedef struct {
	uint32_t opcode : 7;
	uint32_t rd : 5;
	uint32_t funct3 : 3;
	uint32_t rs1 : 5;
	uint32_t rs2 : 5;
	uint32_t funct7 : 7;
} UnpackedInstruction;

typedef struct {
	uint16_t opcode : 2;
	uint16_t rs2 : 5;
	uint16_t rs1 : 5;
	uint16_t funct4 : 4;
} UnpackedCompressedInstruction;

typedef union {
	uint32_t numeric_value;
	uint16_t compressed_numeric_value;
} PackedInstruction;

__attribute__((unused)) inline static PackedInstruction dereference_instruction(PackedInstruction *ptr)
{
	// Avoid reading potentially inaccessible bytes
	PackedInstruction instr = {};
	uint16_t *src_ptr = &ptr->compressed_numeric_value;
	uint16_t *dst_ptr = &instr.compressed_numeric_value;
	dst_ptr[0] = src_ptr[0];
	if ((instr.compressed_numeric_value & 3) == 3) {
		dst_ptr[1] = src_ptr[1];
	}
	return instr;
}

__attribute__((unused)) inline static UnpackedInstruction unpack_instruction(PackedInstruction packed)
{
	const uint32_t p = packed.numeric_value;
	return (UnpackedInstruction) {
		.opcode = EXTRACT_BITS(p, 0, 7),
		.rd = EXTRACT_BITS(p, 7, 5),
		.funct3 = EXTRACT_BITS(p, 12, 3),
		.rs1 = EXTRACT_BITS(p, 15, 5),
		.rs2 = EXTRACT_BITS(p, 20, 5),
		.funct7 = EXTRACT_BITS(p, 25, 7),
	};
}

__attribute__((unused)) inline static UnpackedCompressedInstruction unpack_compressed_instruction(PackedInstruction packed)
{
	const uint32_t p = packed.compressed_numeric_value;
	return (UnpackedCompressedInstruction) {
		.opcode = EXTRACT_BITS(p, 0, 2),
		.rs2 = EXTRACT_BITS(p, 2, 5),
		.rs1 = EXTRACT_BITS(p, 7, 5),
		.funct4 = EXTRACT_BITS(p, 12, 4),
	};
}

#endif /* end of include guard: SRC_INSTRUCTIONS_H_ */
