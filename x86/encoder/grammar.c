#include "grammar.h"
#include <string.h>

static ZydisMnemonic _x86_mnemonic_from_token(const RDToken* tok) {
    for(int i = 0; i < ZYDIS_MNEMONIC_MAX_VALUE; i++) {
        const char* s = ZydisMnemonicGetString((ZydisMnemonic)i);

        if(s && strlen(s) == tok->length &&
           !rd_strnicmp(s, tok->value, (int)tok->length))
            return (ZydisMnemonic)i;
    }

    return ZYDIS_MNEMONIC_INVALID;
}

static ZydisRegister _x86_register_from_token(const RDToken* tok) {
    for(int i = 0; i < ZYDIS_REGISTER_MAX_VALUE; i++) {
        const char* s = ZydisRegisterGetString((ZydisRegister)i);

        if(s && strlen(s) == tok->length &&
           !rd_strnicmp(s, tok->value, (int)tok->length))
            return (ZydisRegister)i;
    }

    return ZYDIS_REGISTER_NONE;
}

static bool _x86_size_keyword_to_bytes(const RDToken* tok, usize* out) {
    static const struct {
        const char* name;
        usize size;
    } SIZES[] = {
        {"byte", 1},
        {"word", 2},
        {"dword", 4},
        {"qword", 8},
    };

    for(usize i = 0; i < rd_count_of(SIZES); i++) {
        if(strlen(SIZES[i].name) == tok->length &&
           !rd_strnicmp(SIZES[i].name, tok->value, (int)tok->length)) {
            *out = SIZES[i].size;
            return true;
        }
    }

    return false;
}

static bool _reg_operand_rule(RDLexer* lex, void* userdata) {
    X86GrammarData* data = (X86GrammarData*)userdata;

    RDToken tok;
    if(!rd_lexer_next_expect(lex, RD_TOK_IDENTIFIER, &tok)) return false;

    ZydisRegister reg = _x86_register_from_token(&tok);
    if(reg == ZYDIS_REGISTER_NONE) return false;

    ZydisEncoderOperand* zop = &data->req.operands[data->req.operand_count++];
    zop->type = ZYDIS_OPERAND_TYPE_REGISTER;
    zop->reg.value = reg;
    return true;
}

static bool _imm_operand_rule(RDLexer* lex, void* userdata) {
    X86GrammarData* data = (X86GrammarData*)userdata;

    RDToken tok;
    if(!rd_lexer_next(lex, &tok)) return false;

    bool negative = tok.type == RD_TOK_MINUS;
    if(negative && !rd_lexer_next(lex, &tok)) return false;

    i64 value;

    if(tok.type == RD_TOK_NUMBER) {
        value = negative ? -(i64)tok.u_value : (i64)tok.u_value;
    }
    else if(tok.type == RD_TOK_IDENTIFIER) {
        const char* name = rd_lexer_token_value(lex, &tok);
        RDAddress addr;
        if(!rd_get_address(data->ctx, name, &addr)) return false;
        value = negative ? -(i64)addr : (i64)addr;
    }
    else
        return false;

    ZydisEncoderOperand* zop = &data->req.operands[data->req.operand_count++];
    zop->type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
    zop->imm.s = value;
    return true;
}

static bool _displ_operand_rule(RDLexer* lex, void* userdata) {
    X86GrammarData* data = (X86GrammarData*)userdata;
    usize size = 0;
    RDToken tok;

    if(rd_lexer_peek_expect(lex, RD_TOK_IDENTIFIER, &tok) &&
       _x86_size_keyword_to_bytes(&tok, &size)) {
        rd_lexer_consume(lex);

        if(!rd_lexer_next_expect(lex, RD_TOK_IDENTIFIER, &tok) ||
           tok.length != 3 || rd_strnicmp("ptr", tok.value, 3))
            return false;
    }

    if(!rd_lexer_try_consume(lex, RD_TOK_OPEN_SQUARE, &tok)) return false;

    ZydisRegister base = ZYDIS_REGISTER_NONE, index = ZYDIS_REGISTER_NONE;
    u8 scale = 1;
    i64 disp = 0;
    bool have_disp = false, first = true;

    while(!rd_lexer_try_consume(lex, RD_TOK_CLOSE_SQUARE, &tok)) {
        bool minus = false;

        if(!first) {
            if(!rd_lexer_next(lex, &tok)) {
                rd_format_to(data->buf, "unterminated memory operand");
                return false;
            }

            bool plus = tok.type == RD_TOK_PLUS;
            minus = tok.type == RD_TOK_MINUS;

            if(!plus && !minus) {
                rd_format_to(data->buf, "expected '+' or '-'");
                return false;
            }
        }

        if(!rd_lexer_next(lex, &tok)) {
            rd_format_to(data->buf, "unterminated memory operand");
            return false;
        }

        if(tok.type == RD_TOK_IDENTIFIER) {
            ZydisRegister reg = _x86_register_from_token(&tok);

            if(reg != ZYDIS_REGISTER_NONE) {
                u8 curr_scale = 1;
                bool scaled = rd_lexer_try_consume(lex, RD_TOK_ASTERISK, &tok);

                if(scaled && !rd_lexer_next_expect(lex, RD_TOK_NUMBER, &tok)) {
                    rd_format_to(data->buf, "expected scale value after '*'");
                    return false;
                }

                if(scaled) curr_scale = (u8)tok.u_value;

                if(scaled || base != ZYDIS_REGISTER_NONE) {
                    if(index != ZYDIS_REGISTER_NONE) {
                        rd_format_to(data->buf,
                                     "only one index register is allowed");
                        return false;
                    }

                    if(reg == ZYDIS_REGISTER_ESP || reg == ZYDIS_REGISTER_RSP) {
                        rd_format_to(
                            data->buf,
                            "ESP/RSP cannot be used as an index register");
                        return false;
                    }

                    index = reg;
                    scale = curr_scale;
                }
                else
                    base = reg;
            }
            else { // not a register, try as a symbol
                const char* name = rd_lexer_token_value(lex, &tok);

                RDAddress addr;
                if(!rd_get_address(data->ctx, name, &addr)) {
                    rd_format_to(
                        data->buf,
                        "unknown register or symbol '%s' in memory operand",
                        name);
                    return false;
                }

                disp += minus ? -(i64)addr
                              : (i64)addr; // folds into the displacement

                have_disp = true;
            }
        }
        else if(tok.type == RD_TOK_NUMBER) {
            if(have_disp) {
                rd_format_to(data->buf, "only one displacement is allowed");
                return false;
            }

            disp = minus ? -(i64)tok.u_value : (i64)tok.u_value;
            have_disp = true;
        }
        else {
            rd_format_to(data->buf, "unexpected token in memory operand");
            return false;
        }

        first = false;
    }

    if(base == ZYDIS_REGISTER_NONE && index == ZYDIS_REGISTER_NONE &&
       !have_disp) {
        rd_format_to(data->buf, "empty memory operand");
        return false;
    }

    ZydisEncoderOperand* zop = &data->req.operands[data->req.operand_count++];
    zop->type = ZYDIS_OPERAND_TYPE_MEMORY;
    zop->mem.base = base;
    zop->mem.index = index;
    zop->mem.scale = index != ZYDIS_REGISTER_NONE ? scale : 0;
    zop->mem.displacement = disp;
    zop->mem.size = (ZyanU16)size;

    return true;
}

bool x86_encoder_parse(RDLexer* lex, const char* s, X86GrammarData* data) {
    rd_lexer_reset(lex, s);

    RDToken tok;
    if(!rd_lexer_next_expect(lex, RD_TOK_IDENTIFIER, &tok)) {
        rd_format_to(data->buf, "expected mnemonic in '%s'", s);
        return false;
    }

    ZydisMnemonic mnem = _x86_mnemonic_from_token(&tok);
    if(mnem == ZYDIS_MNEMONIC_INVALID) {
        rd_format_to(data->buf, "unknown mnemonic '%.*s' in '%s'",
                     (int)tok.length, tok.value, s);
        return false;
    }

    data->req.mnemonic = mnem;

    static const RDLexerTryFn OPERAND_ENTRIES[] = {
        _reg_operand_rule,
        _imm_operand_rule,
        _displ_operand_rule,
        NULL,
    };

    RDToken peek;
    if(rd_lexer_peek(lex, &peek)) {
        if(!rd_lexer_try_any(lex, OPERAND_ENTRIES, data)) {
            rd_format_to(data->buf, "expected operand in '%s'", s);
            return false;
        }

        while(rd_lexer_peek_expect(lex, RD_TOK_COMMA, &tok)) {
            rd_lexer_consume(lex);

            if(data->req.operand_count >= ZYDIS_ENCODER_MAX_OPERANDS) {
                rd_format_to(data->buf, "too many operands in '%s'", s);
                return false;
            }

            if(!rd_lexer_try_any(lex, OPERAND_ENTRIES, data)) {
                rd_format_to(data->buf, "expected operand after ',' in '%s'",
                             s);
                return false;
            }
        }
    }

    if(!rd_lexer_at_end(lex)) {
        rd_format_to(data->buf, "unexpected trailing input in '%s'", s);
        return false;
    }

    // clang-format off
    usize reg_size = 0;
    for(usize i = 0; i < data->req.operand_count; i++) {
        if(data->req.operands[i].type == ZYDIS_OPERAND_TYPE_REGISTER) {
            reg_size = ZydisRegisterGetWidth(data->req.machine_mode,
                                             data->req.operands[i].reg.value) / 8;
            break;
        }
    }
    // clang-format on

    for(usize i = 0; i < data->req.operand_count; i++) {
        ZydisEncoderOperand* op = &data->req.operands[i];
        if(op->type != ZYDIS_OPERAND_TYPE_MEMORY || op->mem.size != 0) continue;

        if(!reg_size) {
            rd_format_to(data->buf,
                         "ambiguous memory operand size in '%s' "
                         "specify byte/word/dword/qword ptr",
                         s);
            return false;
        }

        op->mem.size = (ZyanU16)reg_size;
    }

    return true;
}
