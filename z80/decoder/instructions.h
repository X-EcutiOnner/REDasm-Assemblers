#pragma once

#include "table.h"
#include <string.h>

typedef struct Z80Opcode {
    u8 z : 3;
    u8 y : 3;
    u8 x : 2;
} Z80Opcode;

static_assert(sizeof(u8) == sizeof(Z80Opcode), "invalid Z80 opcode byte size");

typedef struct Z80InstructionResult {
    Z80Instruction instr;
    Z80Opcode opcode;
    usize cursor;
    i8 displ; // pre-read displacement for DDCB/FDCB, unused otherwise
} Z80InstructionResult;

// portable: no reliance on bitfield order
static inline Z80Opcode z80_split(u8 b) {
    Z80Opcode ob;
    memcpy(&ob, &b, sizeof(ob));
    return ob;
}

const char* z80_get_mnemonic(Z80InstructionId id);
bool z80_find_instruction(RDContext* ctx, RDAddress address,
                          Z80InstructionResult* res);
