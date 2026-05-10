#include "lifter.h"
#include <Zydis/Zydis.h>

static void _x86_lift_op(RDInstruction* il, const RDInstruction* instr,
                         usize dst_idx, usize src_idx) {
    const RDOperand* op = &instr->operands[src_idx];

    switch(op->kind) {
        case RD_OP_CNST:
        case RD_OP_REG:
        case RD_OP_ADDR:
        case RD_OP_IMM:
        case RD_OP_DISPL: rd_instr_set_op(il, dst_idx, op); break;

        case RD_OP_MEM: {
            if(instr->id == ZYDIS_MNEMONIC_LEA)
                rd_instr_set_op_addr(il, dst_idx, op->mem);
            else
                rd_instr_set_op(il, dst_idx, op);

            break;
        }

        default: break;
    }
}

static void _x86_lift_jump(RDInstructionVect* v, const RDInstruction* instr) {
    RDInstruction* il = NULL;

    switch(instr->id) {
        case ZYDIS_MNEMONIC_JZ: {
            il = rd_il_push_instr(v, RD_IL_JUMP_EQ);
            rd_instr_set_op_sym(il, 0, "z");
            rd_instr_set_op_cnst(il, 1, 0);
            break;
        }

        case ZYDIS_MNEMONIC_JNZ:
            il = rd_il_push_instr(v, RD_IL_JUMP_NE);
            rd_instr_set_op_sym(il, 0, "z");
            rd_instr_set_op_cnst(il, 1, 0);
            break;

        default: return;
    }

    _x86_lift_op(il, instr, 2, 0);
}

void x86_lift(RDContext* ctx, RDInstructionVect* v, const RDInstruction* instr,
              RDProcessor* p) {
    RD_UNUSED(ctx);
    RD_UNUSED(p);

    switch(instr->id) {
        case ZYDIS_MNEMONIC_NOP: rd_il_push_instr(v, RD_IL_NOP); break;

        case ZYDIS_MNEMONIC_CALL: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_CALL);
            _x86_lift_op(il, instr, 0, 0);
            break;
        }

        case ZYDIS_MNEMONIC_JMP: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_JUMP);
            _x86_lift_op(il, instr, 0, 0);
            break;
        }

        case ZYDIS_MNEMONIC_MOV:
        case ZYDIS_MNEMONIC_LEA: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_MOV);
            _x86_lift_op(il, instr, 0, 0);
            _x86_lift_op(il, instr, 1, 1);
            break;
        }

        case ZYDIS_MNEMONIC_PUSH: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_PUSH);
            _x86_lift_op(il, instr, 0, 0);
            break;
        }

        case ZYDIS_MNEMONIC_POP: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_POP);
            _x86_lift_op(il, instr, 0, 0);
            break;
        }

        case ZYDIS_MNEMONIC_CMP: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_SUB);
            rd_instr_set_op_sym(il, 0, "z");
            _x86_lift_op(il, instr, 1, 0);
            _x86_lift_op(il, instr, 2, 1);
            break;
        }

        case ZYDIS_MNEMONIC_RET: rd_il_push_instr(v, RD_IL_RET); break;

        case ZYDIS_MNEMONIC_ADD: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_ADD);
            _x86_lift_op(il, instr, 0, 0);
            _x86_lift_op(il, instr, 1, 0);
            _x86_lift_op(il, instr, 2, 1);
            break;
        }

        case ZYDIS_MNEMONIC_SUB: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_SUB);
            _x86_lift_op(il, instr, 0, 0);
            _x86_lift_op(il, instr, 1, 0);
            _x86_lift_op(il, instr, 2, 1);
            break;
        }

        case ZYDIS_MNEMONIC_MUL: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_MUL);
            _x86_lift_op(il, instr, 0, 0);
            _x86_lift_op(il, instr, 1, 0);
            _x86_lift_op(il, instr, 2, 1);
            break;
        }

        case ZYDIS_MNEMONIC_DIV: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_DIV);
            _x86_lift_op(il, instr, 0, 0);
            _x86_lift_op(il, instr, 1, 0);
            _x86_lift_op(il, instr, 2, 1);
            break;
        }

        case ZYDIS_MNEMONIC_AND: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_AND);
            _x86_lift_op(il, instr, 0, 0);
            _x86_lift_op(il, instr, 1, 0);
            _x86_lift_op(il, instr, 2, 1);
            break;
        }

        case ZYDIS_MNEMONIC_OR: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_OR);
            _x86_lift_op(il, instr, 0, 0);
            _x86_lift_op(il, instr, 1, 0);
            _x86_lift_op(il, instr, 2, 1);
            break;
        }

        case ZYDIS_MNEMONIC_XOR: {
            RDInstruction* il = rd_il_push_instr(v, RD_IL_XOR);
            _x86_lift_op(il, instr, 0, 0);
            _x86_lift_op(il, instr, 1, 0);
            _x86_lift_op(il, instr, 2, 1);
            break;
        }

        case ZYDIS_MNEMONIC_JZ:
        case ZYDIS_MNEMONIC_JNZ: _x86_lift_jump(v, instr); break;

        default: break;
    }
}
