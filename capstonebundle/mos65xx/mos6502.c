#include "mos6502.h"
#include "capstone.h"

#define MOS6502_INSTRUCTION_MAX 3

static const CapstoneInitData MOS6502_INIT = {
    .arch = CS_ARCH_MOS65XX,
    .mode = CS_MODE_MOS65XX_6502,
};

static void mos6502_decode(RDContext* ctx, RDInstruction* instr,
                           RDProcessor* p) {
    char data[MOS6502_INSTRUCTION_MAX];
    usize n = rd_read(ctx, instr->address, data, rd_count_of(data));
    if(!n) return;

    const cs_insn* cs_insn = capstone_plugin_decode(instr, data, n, p);
    if(!cs_insn) return;

    const cs_mos65xx* d = &cs_insn->detail->mos65xx;

    switch(cs_insn->id) {
        case MOS65XX_INS_JMP: instr->flow = RD_IF_JUMP; break;
        case MOS65XX_INS_JSR: instr->flow = RD_IF_CALL; break;

        case MOS65XX_INS_RTS:
        case MOS65XX_INS_RTI: instr->flow = RD_IF_STOP; break;

        case MOS65XX_INS_BCC:
        case MOS65XX_INS_BCS:
        case MOS65XX_INS_BEQ:
        case MOS65XX_INS_BNE:
        case MOS65XX_INS_BMI:
        case MOS65XX_INS_BPL:
        case MOS65XX_INS_BVC:
        case MOS65XX_INS_BVS: instr->flow = RD_IF_JUMP_COND; break;

        default: break;
    }

    for(u8 i = 0; i < d->op_count && i < RD_MAX_OPERANDS; i++) {
        const cs_mos65xx_op* cop = &d->operands[i];
        RDOperand* op = &instr->operands[i];

        switch(cop->type) {
            case MOS65XX_OP_REG:
                op->kind = RD_OP_REG;
                op->reg = cop->reg;
                break;

            case MOS65XX_OP_IMM:
                op->kind = RD_OP_IMM;
                op->imm = (u8)cop->imm;
                break;

            case MOS65XX_OP_MEM: {
                RDAddress addr = (RDAddress)cop->mem;

                switch(d->am) {
                    case MOS65XX_AM_REL:
                    case MOS65XX_AM_ABS:
                    case MOS65XX_AM_ZP:
                        op->kind = RD_OP_ADDR;
                        op->addr = addr;
                        break;

                    case MOS65XX_AM_ZP_X:
                    case MOS65XX_AM_ABS_X:
                        rd_instr_set_op_displ(instr, i, RD_REGID_UNKNOWN,
                                              MOS65XX_REG_X, 1, (i64)addr);
                        break;

                    case MOS65XX_AM_ZP_Y:
                    case MOS65XX_AM_ABS_Y:
                        rd_instr_set_op_displ(instr, i, RD_REGID_UNKNOWN,
                                              MOS65XX_REG_Y, 1, (i64)addr);
                        break;

                    default: op->kind = RD_OP_STUB; break;
                }

                break;
            }

            default: op->kind = RD_OP_STUB; break;
        }
    }
}

static void mos6502_emulate(RDContext* ctx, const RDInstruction* instr,
                            RDProcessor* p) {
    RD_UNUSED(p);

    if(rd_instr_is_branch(instr)) {
        const RDOperand* op = &instr->operands[0];
        RDXRefType t = rd_instr_is_call(instr) ? RD_CR_CALL : RD_CR_JUMP;

        if(op->kind == RD_OP_ADDR) {
            rd_add_xref(ctx, instr->address, op->addr, t);
        }
        else if(op->kind == RD_OP_MEM && instr->id == MOS65XX_INS_JMP) {
            u16 target;
            if(rd_read_le16(ctx, op->mem, &target))
                rd_add_xref(ctx, instr->address, (RDAddress)target, t);
        }
    }

    if(rd_instr_can_flow(instr)) rd_flow(ctx, instr->address + instr->length);
}

static bool mos6502_render_operand(RDRenderer* r, const RDInstruction* instr,
                                   int idx, RDProcessor* p) {
    RD_UNUSED(p);

    const RDOperand* op = &instr->operands[idx];

    switch(op->kind) {
        // 6502 immediates are conventionally written "#$nn" .
        // without the "#" this is visually indistinguishable from a zero-page
        // address, which is a real (not just cosmetic) ambiguity for a reader.
        case RD_OP_IMM:
            rd_renderer_norm(r, "#$");
            rd_renderer_num(r, op->s_imm, 16, 2, RD_NUM_NOADDR);
            return true;

        case RD_OP_DISPL: {
            // Only ever reached for abs,X / abs,Y / zp,X / zp,Y
            // base is always RD_REGID_UNKNOWN (6502 has no base-register
            // addressing mode, only address+index), so the generic
            // "[base+index+disp]" renderer doesn't apply.
            // This is really "$addr,IndexReg".

            unsigned width = op->userdata2 ? sizeof(u16) : sizeof(u32);
            rd_renderer_norm(r, "$");
            rd_renderer_loc(r, (RDAddress)op->displ.offset, width,
                            RD_NUM_NOADDR);
            rd_renderer_norm(r, ",");
            rd_renderer_reg(r, op->displ.index);
            return true;
        }

        default: break;
    }

    return false;
}

const RDProcessorPlugin MOS6502 = {
    .level = RD_API_LEVEL,
    .id = "mos6502",
    .name = "MOS 6502",
    .ptr_size = sizeof(u16),
    .userdata = (void*)&MOS6502_INIT,
    .create = capstone_plugin_create,
    .destroy = capstone_plugin_destroy,
    .decode = mos6502_decode,
    .emulate = mos6502_emulate,
    .get_mnemonic = capstone_plugin_get_mnemonic,
    .get_reg_name = capstone_plugin_get_reg_name,
    .render_operand = mos6502_render_operand,
};
