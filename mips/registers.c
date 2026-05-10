#include "registers.h"
#include "decoder/registers.h"

bool mips_get_regval(RDContext* ctx, RDAddress addr, RDReg reg,
                     RDRegValue* val) {
    if(reg == MIPS_REG_ZERO) {
        *val = 0;
        return true; // always valid
    }

    if(reg == MIPS_REG_GP) return rd_get_sregval_id(ctx, addr, reg, val);
    return rd_get_regval_id(ctx, reg, val);
}

void mips_set_regval(RDContext* ctx, RDAddress addr, RDReg reg,
                     RDRegValue val) {
    if(reg == MIPS_REG_ZERO) return; // discard, mirrors hardware

    if(reg == MIPS_REG_GP)
        rd_auto_sregval_id(ctx, addr, reg, val);
    else
        rd_set_regval_id(ctx, reg, val);
}

void mips_del_regval(RDContext* ctx, RDAddress addr, RDReg reg) {
    if(reg == MIPS_REG_ZERO) return; // discard, mirrors hardware

    if(reg == MIPS_REG_GP)
        rd_del_auto_sregval_id(ctx, addr, reg);
    else
        rd_del_regval_id(ctx, reg);
}
