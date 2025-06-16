#ifndef SRC_INSTRUCTIONS_H_
#define SRC_INSTRUCTIONS_H_

#include <stdint.h>

#define EXTRACT_BITS(num, lower, count) (((num) >> (lower)) & ((1 << (count)) - 1))
#define EXTEND_SIGN(num, from_bits) (((num) & (1 << ((from_bits) - 1))) ? (num) - (1 << (from_bits)) : (num))

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
	uint32_t numeric_value;
} PackedInstruction;

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

#endif /* end of include guard: SRC_INSTRUCTIONS_H_ */
