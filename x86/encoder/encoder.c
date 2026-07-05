#include "encoder.h"
#include "common.h"
#include "grammar.h"

bool x86_encode(RDContext* ctx, RDAddress address, const char* s,
                RDScratchBuffer* buf, RDProcessor* proc) {
    if(!s) s = "nop";

    const X86Processor* self = (X86Processor*)proc;

    X86GrammarData g = {
        .ctx = ctx,
        .req = {.machine_mode = self->userdata->mode},
        .buf = buf,
    };

    if(!x86_encoder_parse(self->lex, s, &g)) return false;

    char buffer[ZYDIS_MAX_INSTRUCTION_LENGTH] = {0};
    ZyanUSize length = rd_count_of(buffer);

    if(!ZYAN_SUCCESS(ZydisEncoderEncodeInstructionAbsolute(
           &g.req, buffer, &length, (ZyanU64)address))) {
        RD_LOG_FAIL_TO(buf, "failed to encode '%s'", s);
        return false;
    }

    rd_scratch_append(buf, buffer, (usize)length);
    return true;
}
