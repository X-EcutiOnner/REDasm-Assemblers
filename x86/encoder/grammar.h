#pragma once

#include <Zydis/Zydis.h>
#include <redasm/redasm.h>

typedef struct X86GrammarData {
    RDContext* ctx;
    ZydisEncoderRequest req;
    RDScratchBuffer* buf;
} X86GrammarData;

bool x86_encoder_parse(RDLexer* lex, const char* s, X86GrammarData* data);
