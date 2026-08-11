#include "registers.h"

const char* z80_cc_name(Z80Condition cc) {
    switch(cc) {
        case Z80_CC_NZ: return "nz";
        case Z80_CC_Z: return "z";
        case Z80_CC_NC: return "nc";
        case Z80_CC_C: return "c";
        case Z80_CC_PO: return "po";
        case Z80_CC_PE: return "pe";
        case Z80_CC_P: return "p";
        case Z80_CC_M: return "m";
        default: break;
    }

    return "???";
}

const char* z80_reg_name(Z80RegId reg) {
    switch(reg) {
        case Z80_REG_I: return "i";
        case Z80_REG_R: return "r";

        case Z80_REG_A: return "a";
        case Z80_REG_B: return "b";
        case Z80_REG_C: return "c";
        case Z80_REG_D: return "d";
        case Z80_REG_E: return "e";
        case Z80_REG_H: return "h";
        case Z80_REG_L: return "l";

        case Z80_REG_BC: return "bc";
        case Z80_REG_DE: return "de";
        case Z80_REG_HL: return "hl";
        case Z80_REG_SP: return "sp";
        case Z80_REG_AF: return "af";

        case Z80_REG_IX: return "ix";
        case Z80_REG_IXH: return "ixh";
        case Z80_REG_IXL: return "ixl";
        case Z80_REG_IY: return "iy";
        case Z80_REG_IYH: return "iyh";
        case Z80_REG_IYL: return "iyl";

        case Z80_REG_SHD_BC: return "bc'";
        case Z80_REG_SHD_DE: return "de'";
        case Z80_REG_SHD_HL: return "hl'";
        case Z80_REG_SHD_AF: return "af'";

        default: break;
    }

    return NULL;
}
