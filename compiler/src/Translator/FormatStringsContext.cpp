#include "FormatStringsContext.hpp"

using namespace llvm;

void setupFormatStringsContext(const ModuleContext& ctx, FormatStringsContext& formatStringsContext)
{
    /* Setup global format strings */

    formatStringsContext.dataTypeFormatStringsMap = {
        { icode::I8, ctx.builder->CreateGlobalString("%d", "bytefmt", 0U, ctx.LLVMModule.get()) },
        { icode::UI8, ctx.builder->CreateGlobalString("%c", "ubytefmt", 0U, ctx.LLVMModule.get()) },
        { icode::I16, ctx.builder->CreateGlobalString("%hd", "shortfmt", 0U, ctx.LLVMModule.get()) },
        { icode::UI16, ctx.builder->CreateGlobalString("%hu", "ushortfmt", 0U, ctx.LLVMModule.get()) },
        { icode::I32, ctx.builder->CreateGlobalString("%d", "intfmt", 0U, ctx.LLVMModule.get()) },
        { icode::UI32, ctx.builder->CreateGlobalString("%u", "uintfmt", 0U, ctx.LLVMModule.get()) },
        { icode::I64, ctx.builder->CreateGlobalString("%li", "longfmt", 0U, ctx.LLVMModule.get()) },
        { icode::UI64, ctx.builder->CreateGlobalString("%lu", "ulongfmt", 0U, ctx.LLVMModule.get()) },
        { icode::F32, ctx.builder->CreateGlobalString("%f", "floatfmt", 0U, ctx.LLVMModule.get()) },
        { icode::F64, ctx.builder->CreateGlobalString("%lf", "doublefmt", 0U, ctx.LLVMModule.get()) }
    };

    formatStringsContext.dataTypeFormatStringsMap[icode::AUTO_INT] =
        formatStringsContext.dataTypeFormatStringsMap[icode::I64];

    formatStringsContext.dataTypeFormatStringsMap[icode::AUTO_FLOAT] =
        formatStringsContext.dataTypeFormatStringsMap[icode::F64];

    formatStringsContext.dataTypeFormatStringsMap[icode::ENUM] =
        formatStringsContext.dataTypeFormatStringsMap[icode::UI32];

    formatStringsContext.charInputFormatString =
        ctx.builder->CreateGlobalString(" %c", "charinpfmt", 0U, ctx.LLVMModule.get());

    formatStringsContext.newLineString = ctx.builder->CreateGlobalString("\n", "newlnfmt", 0U, ctx.LLVMModule.get());

    formatStringsContext.spaceString = ctx.builder->CreateGlobalString(" ", "spacefmt", 0U, ctx.LLVMModule.get());
}

Value* getFormatStringFromDataTypePrintf(const FormatStringsContext& formatStringsContext, icode::DataType dtype)
{
    return formatStringsContext.dataTypeFormatStringsMap.at(dtype);
}
