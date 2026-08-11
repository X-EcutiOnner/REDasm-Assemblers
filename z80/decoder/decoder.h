#pragma once

// http://www.z80.info/decoding.htm

#include "instructions.h"
#include <redasm/redasm.h>

typedef enum {
    Z80_USEROP_IND_REG = RD_OP_USERBASE,
    Z80_USEROP_IND_N,
    Z80_USEROP_IND_NN,
    Z80_USEROP_CC,
    Z80_USEROP_IND_IDX_COPY, // (ix/iy+d)[, copy-target register]
} Z80Operands;

static inline u8 z80_p(Z80Opcode op) { return op.y >> 1; }
static inline u8 z80_q(Z80Opcode op) { return op.y % 2; }

bool z80_decode_op(const RDContext* ctx, RDInstruction* instr, usize idx,
                   Z80InstructionResult* res);
