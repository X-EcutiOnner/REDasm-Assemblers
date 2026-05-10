#pragma once

#include "decoder/formats.h"
#include <redasm/redasm.h>

void mips_simplify(MIPSDecodedInstruction* dec);
void mips_decode_macro(const MIPSDecodedInstruction* dec, RDInstruction* instr);
