#pragma once

#include "arm/arm32/common.h"
#include <redasm/redasm.h>

extern const CapstoneInitData THUMB_LE_INIT;
extern const CapstoneInitData THUMB_BE_INIT;

extern const RDProcessorPlugin THUMB_LE;
extern const RDProcessorPlugin THUMB_BE;

ARMCapstone* capstone_thumb_create(const CapstoneInitData* data);

void capstone_thumb_decode(RDContext* ctx, RDInstruction* instr,
                           RDProcessor* p);
