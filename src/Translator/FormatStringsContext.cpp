#include "FormatStringsContext.hpp"

using namespace llvm;

void setupFormatStringsContext(const ModuleContext& ctx, FormatStringsContext& formatStringsContext)
{
    /* Setup global format strings */

    formatStringsContext.dataTypeFormatStringsMap = {
        { icode::I8, ctx.builder->CreateGlobalString("%c", "byte", 0U, ctx.LLVMModule.get()) },
        { icode::UI8, ctx.builder->CreateGlobalString("%c", "ubyte", 0U, ctx.LLVMModule.get()) },
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

    formatStringsContext.newLineString = ctx.builder->CreateGlobalString("\n", "newln", 0U, ctx.LLVMModule.get());

    formatStringsContext.spaceString = ctx.builder->CreateGlobalString(" ", "space", 0U, ctx.LLVMModule.get());
}

Value* getFromatStringFromDataType(const FormatStringsContext& formatStringsContext, icode::DataType dtype)
{
    return formatStringsContext.dataTypeFormatStringsMap.at(dtype);
}

Value* getFormatStringForStringInput(const ModuleContext& ctx, int charCount)
{
    std::string formatString = "%" + std::to_string(charCount - 1) + "s";
    return ctx.builder->CreateGlobalString(formatString, "space", 0U, ctx.LLVMModule.get());
}