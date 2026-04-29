#include "x86_common.h"

X86Address x86_read_address(const RDContext* ctx, RDAddress address) {
    const unsigned int PTR_SIZE = rd_processor_get_ptr_size(ctx);
    X86Address res = {0};

    if(PTR_SIZE == sizeof(u64)) {
        u64 v = 0;
        res.has_value = rd_read_le64(ctx, address, &v);
        res.value = (RDAddress)v;
    }
    else if(PTR_SIZE == sizeof(u32)) {
        u32 v = 0;
        res.has_value = rd_read_le32(ctx, address, &v);
        res.value = (RDAddress)v;
    }

    return res;
}

ZydisRegister x86_get_ip(const RDContext* ctx) {
    switch(rd_processor_get_ptr_size(ctx)) {
        case sizeof(u32): return ZYDIS_REGISTER_EIP;
        case sizeof(u64): return ZYDIS_REGISTER_RIP;
        default: break;
    }

    return ZYDIS_REGISTER_IP;
}

ZydisRegister x86_get_sp(const RDContext* ctx) {
    switch(rd_processor_get_ptr_size(ctx)) {
        case sizeof(u32): return ZYDIS_REGISTER_ESP;
        case sizeof(u64): return ZYDIS_REGISTER_RSP;
        default: break;
    }

    return ZYDIS_REGISTER_SP;
}

ZydisRegister x86_get_bp(const RDContext* ctx) {
    switch(rd_processor_get_ptr_size(ctx)) {
        case sizeof(u32): return ZYDIS_REGISTER_EBP;
        case sizeof(u64): return ZYDIS_REGISTER_RBP;
        default: break;
    }

    return ZYDIS_REGISTER_BP;
}
