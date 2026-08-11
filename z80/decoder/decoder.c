#include "decoder.h"
#include "registers.h"

static const Z80RegId Z80_R_TABLE[8] = {
    Z80_REG_B, Z80_REG_C, Z80_REG_D,  Z80_REG_E,
    Z80_REG_H, Z80_REG_L, Z80_REG_HL, Z80_REG_A,
};

static const u8 Z80_IM_TABLE[8] = {
    0, 0, 1, 2, 0, 0, 1, 2,
};

bool z80_decode_op(const RDContext* ctx, RDInstruction* instr, usize idx,
                   Z80InstructionResult* res) {
    RDOperand* op = &instr->operands[idx];
    RDAddress cur_address = instr->address + res->cursor;
    Z80OperandType t = res->instr.operands[idx];

    switch(t) {
        case Z80_OP_CONST_0: {
            op->kind = RD_OP_CNST;
            op->cnst = 0;
            return true;
        }

        case Z80_OP_DISP: {
            u8 v;
            if(!rd_read_byte(ctx, cur_address, &v)) return false;

            op->kind = RD_OP_ADDR;
            op->addr = instr->address + (i8)v + 2;
            return true;
        }

        case Z80_OP_Y: {
            op->kind = RD_OP_CNST;
            op->cnst = (u64)res->opcode.y;
            return true;
        }

        case Z80_OP_Y8: {
            op->kind = RD_OP_CNST;
            op->cnst = (u64)res->opcode.y * 8;
            return true;
        }

        case Z80_OP_CC_Y: {
            op->kind = Z80_USEROP_CC;
            op->cnst = (Z80Condition)res->opcode.y;
            return true;
        }

        case Z80_OP_CC_Y4: {
            op->kind = Z80_USEROP_CC;
            op->cnst = (Z80Condition)(res->opcode.y - 4);
            return true;
        }

        case Z80_OP_N: {
            u8 v;
            if(!rd_read_byte(ctx, cur_address, &v)) return false;

            op->kind = RD_OP_IMM;
            op->imm = (u64)v;
            return true;
        }

        case Z80_OP_NN: {
            u16 v;
            if(!rd_read_le16(ctx, cur_address, &v)) return false;

            bool is_branch = (res->instr.flow == RD_IF_JUMP ||
                              res->instr.flow == RD_IF_JUMP_COND ||
                              res->instr.flow == RD_IF_CALL ||
                              res->instr.flow == RD_IF_CALL_COND);

            if(is_branch) {
                op->kind = RD_OP_ADDR;
                op->addr = (RDAddress)v;
            }
            else {
                op->kind = RD_OP_IMM;
                op->imm = (u64)v;
            }

            return true;
        }

        case Z80_OP_IND_N: {
            u8 v;
            if(!rd_read_byte(ctx, cur_address, &v)) return false;

            op->kind = Z80_USEROP_IND_N;
            op->imm = (u64)v;
            return true;
        }

        case Z80_OP_IND_NN: {
            u16 v;
            if(!rd_read_le16(ctx, cur_address, &v)) return false;

            op->kind = Z80_USEROP_IND_NN;
            op->imm = (u64)v;
            return true;
        }

        case Z80_OP_IND_IX_D:
        case Z80_OP_IND_IY_D: {
            u8 v;
            if(!rd_read_byte(ctx, cur_address, &v)) return false;

            op->kind = RD_OP_DISPL;
            op->displ.base = (t == Z80_OP_IND_IX_D) ? Z80_REG_IX : Z80_REG_IY;
            op->displ.offset = (i64)v;
            return true;
        }

        case Z80_OP_IND_IX_COPY:
        case Z80_OP_IND_IY_COPY: {
            op->kind = Z80_USEROP_IND_IDX_COPY;
            op->displ.base =
                (t == Z80_OP_IND_IX_COPY) ? Z80_REG_IX : Z80_REG_IY;
            op->displ.offset = (i64)res->displ;
            op->displ.index = (res->opcode.z == 6) ? Z80_REG_INVALID
                                                   : Z80_R_TABLE[res->opcode.z];
            return true;
        }

        case Z80_OP_IM_Y: {
            op->kind = RD_OP_CNST;
            op->cnst = Z80_IM_TABLE[res->opcode.y];
            return true;
        }

            // clang-format off
        case Z80_OP_I: { op->kind = RD_OP_REG; op->reg = Z80_REG_I; return true; }
        case Z80_OP_R: { op->kind = RD_OP_REG; op->reg = Z80_REG_R; return true; }
        case Z80_OP_A: { op->kind = RD_OP_REG; op->reg = Z80_REG_A; return true; }
        case Z80_OP_B: { op->kind = RD_OP_REG; op->reg = Z80_REG_B; return true; }
        case Z80_OP_C: { op->kind = RD_OP_REG; op->reg = Z80_REG_C; return true; }
        case Z80_OP_D: { op->kind = RD_OP_REG; op->reg = Z80_REG_D; return true; }
        case Z80_OP_E: { op->kind = RD_OP_REG; op->reg = Z80_REG_E; return true; }
        case Z80_OP_H: { op->kind = RD_OP_REG; op->reg = Z80_REG_H; return true; }
        case Z80_OP_L: { op->kind = RD_OP_REG; op->reg = Z80_REG_L; return true; }
        case Z80_OP_BC: { op->kind = RD_OP_REG; op->reg = Z80_REG_BC; return true; }
        case Z80_OP_DE: { op->kind = RD_OP_REG; op->reg = Z80_REG_DE; return true; }
        case Z80_OP_HL: { op->kind = RD_OP_REG; op->reg = Z80_REG_HL; return true; }
        case Z80_OP_SP: { op->kind = RD_OP_REG; op->reg = Z80_REG_SP; return true; }
        case Z80_OP_AF: { op->kind = RD_OP_REG; op->reg = Z80_REG_AF; return true; }
        case Z80_OP_IX: { op->kind = RD_OP_REG; op->reg = Z80_REG_IX; return true; }
        case Z80_OP_IXH: { op->kind = RD_OP_REG; op->reg = Z80_REG_IXH; return true; }
        case Z80_OP_IXL: { op->kind = RD_OP_REG; op->reg = Z80_REG_IXL; return true; }
        case Z80_OP_IY: { op->kind = RD_OP_REG; op->reg = Z80_REG_IY; return true; }
        case Z80_OP_IYH: { op->kind = RD_OP_REG; op->reg = Z80_REG_IYH; return true; }
        case Z80_OP_IYL: { op->kind = RD_OP_REG; op->reg = Z80_REG_IYL; return true; }
        case Z80_OP_SHD_BC: { op->kind = RD_OP_REG; op->reg = Z80_REG_SHD_BC; return true; }
        case Z80_OP_SHD_DE: { op->kind = RD_OP_REG; op->reg = Z80_REG_SHD_DE; return true; }
        case Z80_OP_SHD_HL: { op->kind = RD_OP_REG; op->reg = Z80_REG_SHD_HL; return true; }
        case Z80_OP_SHD_AF: { op->kind = RD_OP_REG; op->reg = Z80_REG_SHD_AF; return true; }
        case Z80_OP_IND_C: { op->kind = Z80_USEROP_IND_REG; op->reg = Z80_REG_C; return true; }
        case Z80_OP_IND_BC:{ op->kind = Z80_USEROP_IND_REG; op->reg = Z80_REG_BC; return true; }
        case Z80_OP_IND_DE:{ op->kind = Z80_USEROP_IND_REG; op->reg = Z80_REG_DE; return true; }
        case Z80_OP_IND_HL:{ op->kind = Z80_USEROP_IND_REG; op->reg = Z80_REG_HL; return true; }
        case Z80_OP_IND_SP:{ op->kind = Z80_USEROP_IND_REG; op->reg = Z80_REG_SP; return true; }
            // clang-format on

        case Z80_OP_NONE: return true;
        default: break;
    }

    return false;
}
