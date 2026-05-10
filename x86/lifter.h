#pragma once

#include <redasm/redasm.h>

void x86_lift(RDContext* ctx, RDInstructionVect* v, const RDInstruction* instr,
              RDProcessor* p);
