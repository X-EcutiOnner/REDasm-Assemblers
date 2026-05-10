#pragma once

#include <redasm/redasm.h>

void mips32_lift(RDContext* ctx, RDInstructionVect* v,
                 const RDInstruction* instr, RDProcessor* p);
