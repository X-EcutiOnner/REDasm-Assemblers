#pragma once

#include <redasm/redasm.h>

void x86_snapshot_regs(RDContext* ctx, const RDInstruction* instr,
                       RDAddress target);

RDAddress x86_get_ip_value(const RDInstruction* instr);
bool x86_track_segment_reg(RDContext* ctx, const RDInstruction* instr);
void x86_track_mov(RDContext* ctx, const RDInstruction* instr);
bool x86_track_pop_reg(RDContext* ctx, const RDInstruction* instr);
