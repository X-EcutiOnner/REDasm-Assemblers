#include "x86_registers.h"
#include <Zydis/Zydis.h>

// clang-format off
static const RDReg X86_SREGS[] = {
    ZYDIS_REGISTER_ES,
    ZYDIS_REGISTER_CS,
    ZYDIS_REGISTER_SS,
    ZYDIS_REGISTER_DS,
    ZYDIS_REGISTER_FS,
    ZYDIS_REGISTER_GS,
};

static const RDReg X86_GPRS[] = {
    // 8-bit
    ZYDIS_REGISTER_AL,  ZYDIS_REGISTER_AH,
    ZYDIS_REGISTER_BL,  ZYDIS_REGISTER_BH,
    ZYDIS_REGISTER_CL,  ZYDIS_REGISTER_CH,
    ZYDIS_REGISTER_DL,  ZYDIS_REGISTER_DH,
    // 16-bit
    ZYDIS_REGISTER_AX,  ZYDIS_REGISTER_BX,
    ZYDIS_REGISTER_CX,  ZYDIS_REGISTER_DX,
    ZYDIS_REGISTER_SI,  ZYDIS_REGISTER_DI,
    ZYDIS_REGISTER_BP,  ZYDIS_REGISTER_SP,
    // 32-bit
    ZYDIS_REGISTER_EAX, ZYDIS_REGISTER_EBX,
    ZYDIS_REGISTER_ECX, ZYDIS_REGISTER_EDX,
    ZYDIS_REGISTER_ESI, ZYDIS_REGISTER_EDI,
    ZYDIS_REGISTER_EBP, ZYDIS_REGISTER_ESP,
    // 64-bit
    ZYDIS_REGISTER_RAX, ZYDIS_REGISTER_RBX,
    ZYDIS_REGISTER_RCX, ZYDIS_REGISTER_RDX,
    ZYDIS_REGISTER_RSI, ZYDIS_REGISTER_RDI,
    ZYDIS_REGISTER_R8,  ZYDIS_REGISTER_R9,
    ZYDIS_REGISTER_R10, ZYDIS_REGISTER_R11,
    ZYDIS_REGISTER_R12, ZYDIS_REGISTER_R13,
    ZYDIS_REGISTER_R14, ZYDIS_REGISTER_R15,
};
// clang-format on

bool x86_is_segment_reg(const RDOperand* op) {
    if(op->kind != RD_OP_REG) return false;

    switch(op->reg) {
        case ZYDIS_REGISTER_ES:
        case ZYDIS_REGISTER_CS:
        case ZYDIS_REGISTER_SS:
        case ZYDIS_REGISTER_DS:
        case ZYDIS_REGISTER_FS:
        case ZYDIS_REGISTER_GS: return true;

        default: break;
    }

    return false;
}

RDAddress x86_get_ip_value(const RDInstruction* instr) {
    return instr->address + instr->length;
}

void x86_snapshot_regs(RDContext* ctx, const RDInstruction* instr,
                       RDAddress target) {

    for(int i = 0; i < rd_count_of(X86_SREGS); i++) {
        u64 val;
        if(rd_get_regval(ctx, instr->address, X86_SREGS[i], &val))
            rd_auto_regval(ctx, target, X86_SREGS[i], val);
    }

    for(int i = 0; i < rd_count_of(X86_GPRS); i++) {
        u64 val;
        if(rd_get_regval(ctx, instr->address, X86_GPRS[i], &val))
            rd_auto_regval(ctx, target, X86_GPRS[i], val);
    }
}

void x86_track_mov(RDContext* ctx, const RDInstruction* instr) {
    if(instr->operands[0].kind != RD_OP_REG) return;

    const RDOperand* dst = &instr->operands[0];
    const RDOperand* src = &instr->operands[1];
    RDAddress next = x86_get_ip_value(instr);

    switch(src->kind) {
        case RD_OP_IMM: rd_auto_regval(ctx, next, dst->reg, src->imm); break;
        case RD_OP_ADDR: rd_auto_regval(ctx, next, dst->reg, src->addr); break;

        case RD_OP_REG: {
            u64 v;

            if(rd_get_regval(ctx, instr->address, src->reg, &v))
                rd_auto_regval(ctx, next, dst->reg, v);
            else
                rd_auto_regval(ctx, next, dst->reg, RD_REGVAL_UNKNOWN);

            break;
        }

        default: { // memory source: invalidate if 'dst' is a segment reg
            if(x86_is_segment_reg(dst))
                rd_auto_regval(ctx, next, dst->reg, RD_REGVAL_UNKNOWN);

            break;
        }
    }
}

bool x86_track_pop(RDContext* ctx, const RDInstruction* instr) {
    // TODO: davide - stack tracking needed to resolve POP sreg values
    // statically. For now, always invalidate, correct but conservative.
    const RDOperand* dst = &instr->operands[0];
    if(dst->kind != RD_OP_REG) return false;

    RDAddress next = x86_get_ip_value(instr);
    rd_auto_regval(ctx, next, dst->reg, RD_REGVAL_UNKNOWN);
    return true;
}
