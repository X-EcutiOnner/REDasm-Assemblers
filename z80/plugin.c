#include "decoder/decoder.h"
#include "decoder/registers.h"
#include <redasm/redasm.h>

static const char* _z80_get_mnemonic(const RDInstruction* instr,
                                     RDProcessor* p) {
    RD_UNUSED(p);
    return z80_get_mnemonic(instr->id);
}

static const char* _z80_get_reg_name(RDReg r, RDProcessor* p) {
    RD_UNUSED(p);
    return z80_reg_name((Z80RegId)r);
}

static void _z80_decode(RDContext* ctx, RDInstruction* instr, RDProcessor* p) {
    RD_UNUSED(p);

    Z80InstructionResult res;
    if(!z80_find_instruction(ctx, instr->address, &res)) return;
    if(!z80_decode_op(ctx, instr, 0, &res)) return;
    if(!z80_decode_op(ctx, instr, 1, &res)) return;

    // set instruction properties when decoding is completed
    // in this way "success" is reported to the core
    instr->id = res.instr.id;
    instr->length = res.instr.trailing + res.cursor;
    instr->flow = res.instr.flow;
}

static void _z80_emulate(RDContext* ctx, const RDInstruction* instr,
                         RDProcessor* p) {
    RD_UNUSED(p);

    if(rd_instr_is_branch(instr)) { // jump OR call, gates everything below
        RDXRefType kind = rd_instr_is_call(instr) ? RD_CR_CALL : RD_CR_JUMP;

        for(int i = 0; i < Z80_MAX_OPERANDS; i++) {
            if(instr->operands[i].kind == RD_OP_ADDR)
                rd_add_xref(ctx, instr->address, instr->operands[i].addr, kind);
        }
    }

    if(rd_instr_can_flow(instr)) rd_flow(ctx, instr->address + instr->length);
}

static bool _z80_render_operand(RDRenderer* r, const RDInstruction* instr,
                                int idx, RDProcessor* p) {
    RD_UNUSED(p);

    const RDOperand* op = &instr->operands[idx];

    switch(op->kind) {
        case RD_OP_DISPL: {
            rd_renderer_norm(r, "(");
            rd_renderer_reg(r, op->displ.base);
            rd_renderer_num(r, op->displ.offset, 16, 0, RD_NUM_SIGNED);
            rd_renderer_norm(r, ")");
            return true;
        }

        case Z80_USEROP_CC: {
            rd_renderer_text(r, z80_cc_name((Z80Condition)op->cnst),
                             RD_THEME_REG, RD_THEME_DEFAULT);
            return true;
        }

        case Z80_USEROP_IND_N: {
            rd_renderer_norm(r, "(");
            rd_renderer_num(r, (i64)op->imm, 16, 1, RD_NUM_DEFAULT);
            rd_renderer_norm(r, ")");
            return true;
        }

        case Z80_USEROP_IND_NN: {
            rd_renderer_norm(r, "(");
            rd_renderer_num(r, (i64)op->imm, 16, 2, RD_NUM_DEFAULT);
            rd_renderer_norm(r, ")");
            return true;
        }

        case Z80_USEROP_IND_REG: {
            rd_renderer_norm(r, "(");
            rd_renderer_reg(r, op->reg);
            rd_renderer_norm(r, ")");
            return true;
        }

        case Z80_USEROP_IND_IDX_COPY: {
            rd_renderer_norm(r, "(");
            rd_renderer_reg(r, op->displ.base);
            rd_renderer_num(r, op->displ.offset, 16, 0, RD_NUM_SIGNED);
            rd_renderer_norm(r, ")");

            if(op->displ.index != Z80_REG_INVALID) {
                rd_renderer_norm(r, ", ");
                rd_renderer_reg(r, op->displ.index);
            }

            return true;
        }

        default: break;
    }

    return false;
}

static const RDProcessorPlugin Z80 = {
    .level = RD_API_LEVEL,
    .id = "z80",
    .name = "Zilog 80",
    .ptr_size = sizeof(u8),
    .get_mnemonic = _z80_get_mnemonic,
    .get_reg_name = _z80_get_reg_name,
    .decode = _z80_decode,
    .emulate = _z80_emulate,
    .render_operand = _z80_render_operand,
};

void rd_plugin_create(void) { rd_register_processor(&Z80); }

const char* rd_plugin_version(void) { return "1.0"; }
