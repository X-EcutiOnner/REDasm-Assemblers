#include "lifter.h"
#include <capstone/capstone.h>

void capstone_arm32_lift(RDContext* ctx, RDInstructionVect* v,
                         const RDInstruction* instr, RDProcessor* p) {
    RD_UNUSED(ctx);
    RD_UNUSED(p);

    switch(instr->id) {
        case ARM_INS_LDR: {
            if(instr->operands[0].kind == RD_OP_NULL ||
               instr->operands[1].kind == RD_OP_NULL) {
                // alias with missing operands (e.g. pophs pc)
                // flow is already set correctly by decoder, RDIL unkn is honest
                break;
            }

            if(instr->operands[0].reg == ARM_REG_PC) {
                RDInstruction* il = rd_il_push_instr(v, RD_IL_JUMP);
                rd_instr_set_op(il, 0, &instr->operands[1]);
            }
            else {
                RDInstruction* il = rd_il_push_instr(v, RD_IL_MOV);
                rd_instr_set_op(il, 0, &instr->operands[0]);
                rd_instr_set_op(il, 1, &instr->operands[1]);
            }

            break;
        }

        case ARM_INS_MOV: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_MOV);
            rd_instr_set_op(il, 0, &instr->operands[0]);
            rd_instr_set_op(il, 1, &instr->operands[1]);
            break;
        }

        case ARM_INS_B: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_JUMP);
            rd_instr_set_op(il, 0, &instr->operands[0]);
            break;
        }

        case ARM_INS_BL: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_CALL);
            rd_instr_set_op(il, 0, &instr->operands[0]);
            break;
        }

        default: break;
    }
}
