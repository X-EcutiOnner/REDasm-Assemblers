#pragma once

#include <rdapi/rdapi.h>
#include <utility>
#include "../capstone.h"

class ARM32Common
{
    public:
        ARM32Common() = delete;
        static void emulate(Capstone* capstone, RDEmulateResult* result, const cs_insn* insn);
        static void render(Capstone* capstone, const RDRendererParams* rp, const cs_insn* insn);
        static const char* checkAssembler(Capstone* capstone, rd_address address);

    private:
        static void renderMemory(Capstone* capstone, const cs_arm& arm, const cs_arm_op& op, const RDRendererParams* rp);
        static void checkFlowFrom(const cs_arm& arm, RDEmulateResult* result, int startidx);
        static void checkFlow(const cs_arm& arm, RDEmulateResult* result, int opidx);
        static void processOperands(const cs_arm& arm, rd_address address, RDEmulateResult* result);
        static bool isMemPC(const arm_op_mem& mem);
        static rd_address pc(rd_address address);
        static std::pair<size_t, size_t> checkWrap(const cs_insn* insn);
        static rd_type mnemonicTheme(const cs_insn* insn);
};
