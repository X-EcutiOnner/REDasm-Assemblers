#include "instructions.h"

static const Z80Instruction Z80_NONI = {
    Z80_INSTR_NONI,
    RD_IF_NONE,
    0,
    {Z80_OP_NONE, Z80_OP_NONE},
};

static inline bool _z80_is_prefix_byte(u8 b) {
    return b == 0xDD || b == 0xED || b == 0xFD;
}

static Z80OperandType _z80_substitute(Z80OperandType t, bool is_iy) {
    switch(t) {
        case Z80_OP_HL: return is_iy ? Z80_OP_IY : Z80_OP_IX;
        case Z80_OP_H: return is_iy ? Z80_OP_IYH : Z80_OP_IXH;
        case Z80_OP_L: return is_iy ? Z80_OP_IYL : Z80_OP_IXL;
        case Z80_OP_IND_HL: return is_iy ? Z80_OP_IND_IY_D : Z80_OP_IND_IX_D;
        default: return t;
    }
}

static const char* const Z80_MNEMONICS[Z80_INSTR_COUNT] = {
    [Z80_INSTR_NONI] = "noni", [Z80_INSTR_NOP] = "nop",
    [Z80_INSTR_LD] = "ld",     [Z80_INSTR_ADD] = "add",
    [Z80_INSTR_ADC] = "adc",   [Z80_INSTR_SUB] = "sub",
    [Z80_INSTR_SBC] = "sbc",   [Z80_INSTR_AND] = "and",
    [Z80_INSTR_XOR] = "xor",   [Z80_INSTR_OR] = "or",
    [Z80_INSTR_CP] = "cp",     [Z80_INSTR_JP] = "jp",
    [Z80_INSTR_JR] = "jr",     [Z80_INSTR_INC] = "inc",
    [Z80_INSTR_DEC] = "dec",   [Z80_INSTR_DJNZ] = "djnz",
    [Z80_INSTR_CALL] = "call", [Z80_INSTR_RET] = "ret",
    [Z80_INSTR_RST] = "rst",   [Z80_INSTR_PUSH] = "push",
    [Z80_INSTR_POP] = "pop",   [Z80_INSTR_EX] = "ex",
    [Z80_INSTR_DI] = "di",     [Z80_INSTR_EI] = "ei",
    [Z80_INSTR_EXX] = "exx",   [Z80_INSTR_HALT] = "halt",

    [Z80_INSTR_RLC] = "rlc",   [Z80_INSTR_RRC] = "rrc",
    [Z80_INSTR_RL] = "rl",     [Z80_INSTR_RR] = "rr",
    [Z80_INSTR_SLA] = "sla",   [Z80_INSTR_SRA] = "sra",
    [Z80_INSTR_SLL] = "sll",   [Z80_INSTR_SRL] = "srl",
    [Z80_INSTR_BIT] = "bit",   [Z80_INSTR_RES] = "res",
    [Z80_INSTR_SET] = "set",

    [Z80_INSTR_NEG] = "neg",   [Z80_INSTR_RETN] = "retn",
    [Z80_INSTR_RETI] = "reti", [Z80_INSTR_IM] = "im",
    [Z80_INSTR_LDI] = "ldi",   [Z80_INSTR_LDIR] = "ldir",
    [Z80_INSTR_LDD] = "ldd",   [Z80_INSTR_LDDR] = "lddr",
    [Z80_INSTR_CPI] = "cpi",   [Z80_INSTR_CPIR] = "cpir",
    [Z80_INSTR_IN] = "in",     [Z80_INSTR_OUT] = "out",

    [Z80_INSTR_RLCA] = "rlca", [Z80_INSTR_RRCA] = "rrca",
    [Z80_INSTR_RLA] = "rla",   [Z80_INSTR_RRA] = "rra",
    [Z80_INSTR_DAA] = "dda",   [Z80_INSTR_CPL] = "cpl",
    [Z80_INSTR_SCF] = "scf",   [Z80_INSTR_CCF] = "ccf",

    [Z80_INSTR_CPD] = "cpd",   [Z80_INSTR_IND] = "ind",
    [Z80_INSTR_RRD] = "rrd",   [Z80_INSTR_RLD] = "rld",
    [Z80_INSTR_INI] = "ini",   [Z80_INSTR_OUTI] = "outi",
    [Z80_INSTR_OUTD] = "outd", [Z80_INSTR_INIR] = "inir",
    [Z80_INSTR_OTIR] = "otir", [Z80_INSTR_CPDR] = "cpdr",
    [Z80_INSTR_INDR] = "indr", [Z80_INSTR_OTDR] = "otdr",
};

const char* z80_get_mnemonic(const RDInstruction* instr, RDProcessor* p) {
    RD_UNUSED(p);
    return instr->id < Z80_INSTR_COUNT ? Z80_MNEMONICS[instr->id] : NULL;
}

bool z80_instr_is_load_store(const Z80Instruction* instr) {
    if(instr->id != Z80_INSTR_LD) return false;

    // only SP destination counts, per policy
    return instr->operands[0] == Z80_OP_SP;
}

bool z80_instr_is_branch(const Z80Instruction* instr) {
    return (instr->flow == RD_IF_JUMP || instr->flow == RD_IF_JUMP_COND ||
            instr->flow == RD_IF_CALL || instr->flow == RD_IF_CALL_COND);
}

bool z80_find_instruction(RDContext* ctx, RDAddress address,
                          Z80InstructionResult* res) {
    u8 op;
    if(!rd_read_byte(ctx, address, &op)) return false;

    u8 prefix = op; // might be a prefix

    res->cursor = 1;
    const Z80Instruction* sel_table = NULL;

    switch(op) {
        case 0xCB: {
            if(!rd_read_byte(ctx, address + res->cursor, &op)) return false;

            res->cursor++;
            sel_table = Z80_PREFIX_CB;
            break;
        }

        case 0xED: {
            if(!rd_read_byte(ctx, address + res->cursor, &op)) return false;

            res->cursor++;
            sel_table = Z80_PREFIX_ED;
            break;
        }

        case 0xDD:
        case 0xFD: {
            if(!rd_read_byte(ctx, address + res->cursor, &op)) return false;

            if(_z80_is_prefix_byte(op)) {
                res->cursor = 1;
                res->opcode = (Z80Opcode){0};
                res->instr = Z80_NONI;
                return true;
            }

            bool is_iy = prefix == 0xFD;

            if(op == 0xCB) { // DDCB/FDCB
                u8 dv, opcode2;
                if(!rd_read_byte(ctx, address + res->cursor + 1, &dv))
                    return false; // displacement
                if(!rd_read_byte(ctx, address + res->cursor + 2, &opcode2))
                    return false; // real opcode

                // CB byte + displacement byte + opcode byte
                // (prefix already counted)
                res->cursor += 3;
                res->displ = (i8)dv;
                res->opcode = z80_split(opcode2);

                static const Z80InstructionId ROT_IDS[8] = {
                    Z80_INSTR_RLC, Z80_INSTR_RRC, Z80_INSTR_RL,  Z80_INSTR_RR,
                    Z80_INSTR_SLA, Z80_INSTR_SRA, Z80_INSTR_SLL, Z80_INSTR_SRL,
                };

                res->instr.flow = RD_IF_NONE;
                res->instr.trailing = 0;
                res->instr.operands[1] =
                    is_iy ? Z80_OP_IND_IY_COPY : Z80_OP_IND_IX_COPY;

                switch(res->opcode.x) {
                    case 0:
                        res->instr.id = ROT_IDS[res->opcode.y];
                        res->instr.operands[0] = Z80_OP_NONE;
                        break;

                    case 1:
                        res->instr.id = Z80_INSTR_BIT;
                        res->instr.operands[0] = Z80_OP_Y;
                        break;

                    case 2:
                        res->instr.id = Z80_INSTR_RES;
                        res->instr.operands[0] = Z80_OP_Y;
                        break;

                    case 3:
                        res->instr.id = Z80_INSTR_SET;
                        res->instr.operands[0] = Z80_OP_Y;
                        break;

                    default: return false;
                }

                return true;
            }

            res->cursor++;

            const Z80Instruction* base = &Z80_PREFIX_NONE[op];
            if(base->id == Z80_INSTR_INVALID) return false;

            bool had_ind_hl = (base->operands[0] == Z80_OP_IND_HL ||
                               base->operands[1] == Z80_OP_IND_HL);

            res->opcode = z80_split(op);
            res->instr = *base;

            if(op != 0xEB) {
                res->instr.operands[0] =
                    _z80_substitute(res->instr.operands[0], is_iy);
                res->instr.operands[1] =
                    _z80_substitute(res->instr.operands[1], is_iy);
            }

            if(had_ind_hl) res->instr.trailing += 1;

            return true;
        }

        default: {
            sel_table = Z80_PREFIX_NONE;
            break;
        }
    }

    if(!sel_table) return false;

    const Z80Instruction* e = &sel_table[op];
    if(e->id == Z80_INSTR_INVALID) return false;

    res->opcode = z80_split(op);
    res->instr = *e;
    return true;
}
