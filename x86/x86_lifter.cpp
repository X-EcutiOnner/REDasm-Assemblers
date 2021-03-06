#include "x86_lifter.h"
#include <climits>

void X86Lifter::lift(RDContext* ctx, ZydisDecoder decoder, rd_address address, const RDBufferView* view, RDILFunction* il)
{
    ZydisDecodedInstruction zinstr;

    if(!X86Lifter::decode(decoder, view, &zinstr))
    {
        RDILFunction_Append(il, RDILFunction_UNKNOWN(il));
        return;
    }

    switch(zinstr.mnemonic)
    {
        case ZYDIS_MNEMONIC_CALL:
        {
            auto* op = X86Lifter::liftOperand(address, &zinstr, 0, il);
            auto* c = RDILFunction_CALL(il, op);
            RDILFunction_Append(il, c);
            break;
        }

        case ZYDIS_MNEMONIC_JMP:
        {
            auto* op = X86Lifter::liftOperand(address, &zinstr, 0, il);
            auto* c = RDILFunction_GOTO(il, op);
            RDILFunction_Append(il, c);
            break;
        }

        case ZYDIS_MNEMONIC_MOV:
        case ZYDIS_MNEMONIC_LEA:
        {
            auto* dst = X86Lifter::liftOperand(address, &zinstr, 0, il);
            auto* src = X86Lifter::liftOperand(address, &zinstr, 1, il);

            auto* c = RDILFunction_COPY(il, dst, src);
            RDILFunction_Append(il, c);
            break;
        }

        case ZYDIS_MNEMONIC_PUSH:
        {
            auto* op = X86Lifter::liftOperand(address, &zinstr, 0, il);
            auto* c = RDILFunction_PUSH(il, op);
            RDILFunction_Append(il, c);
            break;
        }

        case ZYDIS_MNEMONIC_POP:
        {
            auto* op = X86Lifter::liftOperand(address, &zinstr, 0, il);
            auto* c = RDILFunction_POP(il, op);
            RDILFunction_Append(il, c);
            break;
        }

        case ZYDIS_MNEMONIC_CMP:
        {
            auto* l = X86Lifter::liftOperand(address, &zinstr, 0, il);
            auto* r = X86Lifter::liftOperand(address, &zinstr, 1, il);
            auto* v = RDILFunction_VAR(il, 1, "z");
            auto* s = RDILFunction_SUB(il, l, r);
            RDILFunction_Append(il, RDILFunction_COPY(il, v, s));
            break;
        }

        case ZYDIS_MNEMONIC_ENTER:
        {
            auto size = zinstr.operands[0].imm.value.u, depth = zinstr.operands[1].imm.value.u;

            if(!size && !depth) {
                auto* bpreg = ZydisRegisterGetString(X86Lifter::getBP(ctx));
                auto* spreg = ZydisRegisterGetString(X86Lifter::getSP(ctx));
                RDILFunction_Append(il, RDILFunction_PUSH(il, RDILFunction_REG(il, 0, bpreg)));
                RDILFunction_Append(il, RDILFunction_COPY(il, RDILFunction_REG(il, 0, bpreg), RDILFunction_REG(il, 0, spreg)));
            }
            else
                RDILFunction_Append(il, RDILFunction_UNKNOWN(il));

            break;
        }

        case ZYDIS_MNEMONIC_LEAVE:
        {
            auto* sp = RDILFunction_REG(il, 0, ZydisRegisterGetString(X86Lifter::getSP(ctx)));
            auto* bp = RDILFunction_REG(il, 0, ZydisRegisterGetString(X86Lifter::getBP(ctx)));
            RDILFunction_Append(il, RDILFunction_COPY(il, sp, bp));

            bp = RDILFunction_REG(il, 0, ZydisRegisterGetString(X86Lifter::getBP(ctx)));
            RDILFunction_Append(il, RDILFunction_POP(il, bp));
            break;
        }

        case ZYDIS_MNEMONIC_RET:
        {
            RDILFunction_Append(il, RDILFunction_POP(il, RDILFunction_VAR(il, 0, "result")));

            if(zinstr.operand_count) {
                auto* sp = RDILFunction_REG(il, 0, ZydisRegisterGetString(X86Lifter::getSP(ctx)));
                RDILFunction_Append(il, RDILFunction_ADD(il, sp, RDILFunction_CNST(il, 0, zinstr.operands[0].imm.value.u)));
            }

            RDILFunction_Append(il, RDILFunction_RET(il, RDILFunction_VAR(il, 0, "result")));
            break;
        }

        case ZYDIS_MNEMONIC_ADD:
        {
            auto* op1 = X86Lifter::liftOperand(address, &zinstr, 0, il);
            auto* op2 = X86Lifter::liftOperand(address, &zinstr, 1, il);
            RDILFunction_Append(il, RDILFunction_COPY(il, op1, RDILFunction_ADD(il, op1, op2)));
            break;
        }

        case ZYDIS_MNEMONIC_SUB:
        {
            auto* op1 = X86Lifter::liftOperand(address, &zinstr, 0, il);
            auto* op2 = X86Lifter::liftOperand(address, &zinstr, 1, il);
            RDILFunction_Append(il, RDILFunction_COPY(il, op1, RDILFunction_SUB(il, op1, op2)));
            break;
        }

        case ZYDIS_MNEMONIC_MUL:
        {
            auto* op1 = X86Lifter::liftOperand(address, &zinstr, 0, il);
            auto* op2 = X86Lifter::liftOperand(address, &zinstr, 1, il);
            RDILFunction_Append(il, RDILFunction_COPY(il, op1, RDILFunction_MUL(il, op1, op2)));
            break;
        }

        case ZYDIS_MNEMONIC_DIV:
        {
            auto* op1 = X86Lifter::liftOperand(address, &zinstr, 0, il);
            auto* op2 = X86Lifter::liftOperand(address, &zinstr, 1, il);
            RDILFunction_Append(il, RDILFunction_COPY(il, op1, RDILFunction_DIV(il, op1, op2)));
            break;
        }

        case ZYDIS_MNEMONIC_JZ:
        case ZYDIS_MNEMONIC_JNZ: X86Lifter::liftJump(&zinstr, address, il); break;

        case ZYDIS_MNEMONIC_NOP: RDILFunction_Append(il, RDILFunction_NOP(il)); break;
        default: RDILFunction_Append(il, RDILFunction_UNKNOWN(il)); break;
    }
}

void X86Lifter::liftJump(const ZydisDecodedInstruction* zinstr, rd_address address, RDILFunction* il)
{
    RDILExpression* cond = nullptr;

    switch(zinstr->mnemonic)
    {
        case ZYDIS_MNEMONIC_JZ:
            cond = RDILFunction_EQ(il, RDILFunction_VAR(il, 1, "z"), RDILFunction_CNST(il, 1, 0));
            break;

        case ZYDIS_MNEMONIC_JNZ:
            cond = RDILFunction_NE(il, RDILFunction_VAR(il, 1, "z"), RDILFunction_CNST(il, 1, 0));
            break;

        default: break;
    }

    if(!cond)
    {
        RDILFunction_Append(il, RDILFunction_UNKNOWN(il));
        return;
    }

    auto addr = X86Lifter::calcAddress(zinstr, 0, address);
    auto* t = RDILFunction_CNST(il, zinstr->operands[0].size, addr ? *addr : zinstr->operands[0].imm.value.u);
    auto* f = RDILFunction_CNST(il, zinstr->operands[0].size, address + zinstr->length);
    RDILFunction_Append(il, RDILFunction_IF(il, cond, RDILFunction_GOTO(il, t), RDILFunction_GOTO(il, f)));
}

RDILExpression* X86Lifter::liftOperand(rd_address address, const ZydisDecodedInstruction* zinstr, size_t idx, const RDILFunction* il)
{
    const auto& op = zinstr->operands[idx];

    size_t sz = op.size / CHAR_BIT;
    RDILExpression* e = nullptr;

    switch(op.type)
    {
        case ZYDIS_OPERAND_TYPE_REGISTER: e = RDILFunction_REG(il, sz, ZydisRegisterGetString(op.reg.value)); break;

        case ZYDIS_OPERAND_TYPE_IMMEDIATE:
        {
            u64 c = op.imm.value.u;

            if(X86Lifter::needsCalcAddress(zinstr)) {
                auto addr = X86Lifter::calcAddress(zinstr, idx, address);
                if(addr) c = static_cast<u64>(*addr);
            }

            e = RDILFunction_CNST(il, sz, c);
            break;
        }

        case ZYDIS_OPERAND_TYPE_MEMORY:
        {
            RDILExpression *base = nullptr, *index = nullptr, *scale = nullptr, *disp = nullptr;
            auto addr = X86Lifter::calcAddress(zinstr, idx, address);

            if(!addr) {
                if(op.mem.base != ZYDIS_REGISTER_NONE) base = RDILFunction_REG(il, sz, ZydisRegisterGetString(op.mem.base));
                if(op.mem.index != ZYDIS_REGISTER_NONE) index = RDILFunction_REG(il, sz, ZydisRegisterGetString(op.mem.index));
                if(op.mem.scale > 1) scale = RDILFunction_CNST(il, sz, op.mem.scale);
                if(op.mem.disp.has_displacement) disp = RDILFunction_CNST(il, sz, op.mem.disp.value);
            }
            else
                disp = RDILFunction_CNST(il, 0, *addr);

            RDILExpression* lhs = nullptr;
            auto* indexscale = (scale && index) ? RDILFunction_MUL(il, index, scale) : index;

            if(base && indexscale) lhs = RDILFunction_ADD(il, base, indexscale);
            else if(base) lhs = base;
            else if(indexscale) lhs = indexscale;

            RDILExpression* m = nullptr;

            if(disp) {
                if(lhs) m = RDILFunction_ADD(il, lhs, disp);
                else m = disp;
            }
            else if(lhs) m = lhs;
            else m = RDILFunction_UNKNOWN(il);

            if((zinstr->mnemonic == ZYDIS_MNEMONIC_LEA) && idx) e = m;
            else e = RDILFunction_MEM(il, m);
            break;
        }

        default: e = RDILFunction_UNKNOWN(il); break;
    }

    return e;
}

bool X86Lifter::needsCalcAddress(const ZydisDecodedInstruction* zinstr)
{
    switch(zinstr->meta.category)
    {
        case ZYDIS_CATEGORY_CALL:
        case ZYDIS_CATEGORY_UNCOND_BR:
        case ZYDIS_CATEGORY_COND_BR:
            return true;

        default: break;
    }

    return false;
}
