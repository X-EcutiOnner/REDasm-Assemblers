#include "lifter.h"
#include "decoder/instructions.h"
#include "decoder/registers.h"

void mips32_lift(RDContext* ctx, RDInstructionVect* v,
                 const RDInstruction* instr, RDProcessor* p) {
    RD_UNUSED(ctx);
    RD_UNUSED(p);

    switch(instr->id) {
        case MIPS_MACRO_LI: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_MOV);
            rd_instr_set_op(il, 0, &instr->operands[0]);
            rd_instr_set_op(il, 1, &instr->operands[1]);
            break;
        }

        case MIPS_INSTR_JR: {
            if(instr->operands[0].reg != MIPS_REG_RA) {
                RDInstruction* il = rd_il_push_instr(v, RD_IL_JUMP);
                rd_instr_set_op(il, 0, &instr->operands[0]);
            }
            else
                rd_il_push_instr(v, RD_IL_RET);

            break;
        }

        default: break;
    }
}
