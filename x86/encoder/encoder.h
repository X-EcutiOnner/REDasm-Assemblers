#pragma once

#include <redasm/redasm.h>

bool x86_encode(RDContext* ctx, RDAddress address, const char* s,
                RDScratchBuffer* scratch, RDProcessor* proc);
