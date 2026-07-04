#pragma once

#include <Zydis/Zydis.h>
#include <redasm/redasm.h>

typedef struct X86UserData {
    ZydisMachineMode mode;
    ZydisStackWidth width;
} X86UserData;

typedef struct X86Processor {
    const X86UserData* userdata;
    ZydisDecoder decoder;
    char buffer[ZYDIS_MAX_INSTRUCTION_LENGTH];
    RDLexer* lex;
    // const RDCallingConvention** calling_conventions{nullptr};
} X86Processor;

typedef struct X86Address {
    RDAddress value;
    bool has_value;
} X86Address;

X86Address x86_read_address(const RDContext* ctx, RDAddress address);

ZydisRegister x86_get_ip(const RDContext* ctx);
ZydisRegister x86_get_sp(const RDContext* ctx);
ZydisRegister x86_get_bp(const RDContext* ctx);
