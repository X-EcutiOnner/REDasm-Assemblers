#pragma once

#include "formats.h"
#include <redasm/redasm.h>

void mips_simplify(RDContext* ctx, MIPSDecodedInstruction* dec,
                   const RDInstruction* instr);

void mips_decode_macro(const MIPSDecodedInstruction* dec, RDInstruction* instr);
