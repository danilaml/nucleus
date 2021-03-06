/**
 * (c) 2015 Alexandro Sanchez Bach. All rights reserved.
 * Released under GPL v2 license. Read LICENSE for more details.
 */

#include "ppu_recompiler.h"
#include "nucleus/logger/logger.h"

namespace cpu {
namespace frontend {
namespace ppu {

using namespace cpu::hir;

// Register names
const char* string_vr[] = {
    "vr0_",  "vr1_",  "vr2_",  "vr3_",  "vr4_",  "vr5_",  "vr6_",  "vr7_",
    "vr8_",  "vr9_",  "vr10_", "vr11_", "vr12_", "vr13_", "vr14_", "vr15_",
    "vr16_", "vr17_", "vr18_", "vr19_", "vr20_", "vr21_", "vr22_", "vr23_",
    "vr24_", "vr25_", "vr26_", "vr27_", "vr28_", "vr29_", "vr30_", "vr31_",
};

const char* string_cr = "cr_";
const char* string_fpscr = "fpscr_";
const char* string_xer = "xer_";

Recompiler::Recompiler(Function* function) : frontend::IRecompiler<U32>(function)
{
}

void Recompiler::createProlog()
{
    builder.SetInsertPoint(prolog);

    // Place arguments in local variables
    auto argValue = function->function.function->arg_begin();

    auto ppuFunc = static_cast<Function*>(function);
    for (int i = 0; i < ppuFunc->type_in.size(); i++, argValue++) {
        switch (ppuFunc->type_in[i]) {
        case FUNCTION_IN_INTEGER:
            gpr[3 + i] = builder.CreateAlloca<I64>(format("r%d_", 3 + i));
            builder.CreateStore(Value<I64>{argValue}, gpr[3 + i]);
            break;
        case FUNCTION_IN_FLOAT:
            fpr[1 + i] = builder.CreateAlloca<F64>(format("r%d_", 1 + i));
            builder.CreateStore(Value<F64>{argValue}, fpr[1 + i]);
            break;
        case FUNCTION_IN_VECTOR:
            // TODO
            break;
        }
    }

    state = builder.CreateCall(static_cast<Segment*>(function->parent)->funcGetState);
    builder.CreateCall(static_cast<Segment*>(function->parent)->funcLogState, {builder.get<I64>(function->address)});

    // Branch to the real entry block
    hir::Block entryBlock = blocks.at(function->address);
    builder.createBr(entryBlock);
}

void Recompiler::createEpilog()
{
    builder.SetInsertPoint(epilog);

    auto ppuFunc = static_cast<Function*>(function);
    switch (ppuFunc->type_out) {
    case FUNCTION_OUT_INTEGER:
        builder.createRet(getGPR(3));
        break;
    case FUNCTION_OUT_FLOAT:
        builder.createRet(getFPR(1));
        break;
    case FUNCTION_OUT_FLOAT_X2:
        builder.createRet(getFPR(1)); // TODO
        break;
    case FUNCTION_OUT_FLOAT_X3:
        builder.createRet(getFPR(1)); // TODO
        break;
    case FUNCTION_OUT_FLOAT_X4:
        builder.createRet(getFPR(1)); // TODO
        break;
    case FUNCTION_OUT_VECTOR:
        builder.createRet(getVR<I32>(2));
        break;
    case FUNCTION_OUT_VOID:
        builder.createRetVoid();
        break;

    default:
        logger.error(LOG_CPU, "Recompiler::createReturn: Invalid returnType");
        builder.createRetVoid();
        break;
    }
}

/**
 * Register read
 */
Value* Recompiler::getGPR(int index, Type type)
{
    // Return+Parameter registers and nonvolatile registers
    if ((3 <= index && index <= 10) || (index >= 14)) {
        if (!gpr[index]) {
            gpr[index] = allocaVariable<hir::I64>(format("r%d_", index));
        }
        return builder.createLoad(gpr[index], format("r%d_", index));
    }

    // Other volatile registers
    else {
        Value<I64*> regPointer = builder.CreateInBoundsGEP(state, {
            builder.get<I32>(0),
            builder.get<I32>(0),
            builder.get<I32>(index)});
        return builder.CreateLoad(regPointer, format("r%d_", index));
    }
}

Value* Recompiler::getFPR(int index)
{
    // Return+Parameter registers and nonvolatile registers
    if ((1 <= index && index <= 13) || (index >= 14)) {
        if (!fpr[index].value) {
            fpr[index] = allocaVariable<F64>(format("f%d_", index));
        }
        return builder.CreateLoad(fpr[index]);
    }

    // Other volatile registers
    else {
        Value<F64*> regPointer = builder.CreateInBoundsGEP(state, {
            builder.get<I32>(0),
            builder.get<I32>(1),
            builder.get<I32>(index)});
        return builder.CreateLoad(regPointer, format("f%d_", index));
    }
}

Value* Recompiler::getCR(int index)
{
    // TODO: We are considering all CR fields nonvolatile
    if (!cr[index].value) {
        cr[index] = allocaVariable<I8>(format("cr%d_", index));
    }
    return builder.createLoad(cr[index]);
}

Value* Recompiler::getXER()
{
    // TODO: We are considering the XER field nonvolatile
    if (!ctr.value) {
        ctr = allocaVariable<I64>("xer_");
    }
    return builder.createLoad(ctr);
}

Value* Recompiler::getCTR()
{
    // TODO: We are considering the CTR field nonvolatile
    if (!ctr.value) {
        ctr = allocaVariable<I64>("ctr_");
    }
    return builder.createLoad(ctr);
}

/**
 * Register write
 */
void Recompiler::setGPR(int index, Value* value)
{
    // Return+Parameter registers and non-volatile registers
    if ((3 <= index && index <= 10) || (index >= 14)) {
        if (!gpr[index].value) {
            gpr[index] = allocaVariable<I64>(format("r%d_", index));
        }
        builder.createStore(value, gpr[index]);
    }

    // Other volatile registers
    else {
        hir::Value<hir::I64*> regPointer = builder.CreateInBoundsGEP(state, {
            builder.get<I32>(0),
            builder.get<I32>(0),
            builder.get<I32>(index)});
        return builder.CreateStore(value, regPointer);
    }
}

void Recompiler::setFPR(int index, Value* value)
{
    // Return+Parameter registers and nonvolatile registers
    if ((1 <= index && index <= 13) || (index >= 14)) {
        if (!fpr[index].value) {
            fpr[index] = allocaVariable<F64>(format("f%d_", index));
        }
        builder.createStore(value, fpr[index]);
    }

    // Other volatile registers
    else {
        Value<F64*> regPointer = builder.CreateInBoundsGEP(state, {
            builder.get<I32>(0),
            builder.get<I32>(0),
            builder.get<I32>(index)});
        return builder.CreateStore(value, regPointer);
    }
}

void Recompiler::setCR(int index, Value* value)
{
    // TODO: We are considering all CR fields nonvolatile
    if (!cr[index].value) {
        cr[index] = allocaVariable<I8>(format("cr%d_", index));
    }
    builder.createStore(value, cr[index]);
}

void Recompiler::setXER(Value* value)
{
    // TODO: We are considering the XER field nonvolatile
    if (!xer.value) {
        xer = allocaVariable<I64>("xer_");
    }
    builder.createStore(value, xer);
}

void Recompiler::setCTR(Value* value)
{
    // TODO: We are considering the CTR field nonvolatile
    if (!ctr.value) {
        ctr = allocaVariable<I64>("ctr_");
    }
    builder.createStore(value, ctr);
}

/**
 * Operation flags
 */
void Recompiler::updateCR0(Value* value)
{
    updateCR(0, value, builder.getConstantI64(0), false);
}

}  // namespace ppu
}  // namespace frontend
}  // namespace cpu
