#pragma once

#include <redasm/redasm.h>

void capstone_arm32_lift(RDContext* ctx, RDInstructionVect* v,
                         const RDInstruction* instr, RDProcessor* p);
