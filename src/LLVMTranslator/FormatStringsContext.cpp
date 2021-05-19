#include "../log.hpp"

#include "FormatStringsContext.hpp"

using namespace llvm;

void setupFormatStringsContext(const ModuleContext& ctx, FormatStringsContext& formatStringsContext)
{
    // TODO: Change this to a map

    /* Setup global format strings */
    formatStringsContext.byteFormatString = ctx.builder->CreateGlobalString("%c", "byte", 0U, ctx.LLVMModule.get());

    formatStringsContext.ubyteFormatString = ctx.builder->CreateGlobalString("%c", "ubyte", 0U, ctx.LLVMModule.get());

    formatStringsContext.shortFormatString = ctx.builder->CreateGlobalString("%hd", "shortfmt", 0U, ctx.LLVMModule.get());

    formatStringsContext.ushortFormatString = ctx.builder->CreateGlobalString("%hu", "ushortfmt", 0U, ctx.LLVMModule.get());

    formatStringsContext.intFormatString = ctx.builder->CreateGlobalString("%d", "intfmt", 0U, ctx.LLVMModule.get());

    formatStringsContext.uintFormatString = ctx.builder->CreateGlobalString("%u", "uintfmt", 0U, ctx.LLVMModule.get());

    formatStringsContext.longFormatString = ctx.builder->CreateGlobalString("%li", "longfmt", 0U, ctx.LLVMModule.get());

    formatStringsContext.ulongFormatString = ctx.builder->CreateGlobalString("%lu", "ulongfmt", 0U, ctx.LLVMModule.get());

    formatStringsContext.floatFormatString = ctx.builder->CreateGlobalString("%f", "floatfmt", 0U, ctx.LLVMModule.get());

    formatStringsContext.doubleFormatString = ctx.builder->CreateGlobalString("%lf", "doublefmt", 0U, ctx.LLVMModule.get());

    formatStringsContext.newLineString = ctx.builder->CreateGlobalString("\n", "newln", 0U, ctx.LLVMModule.get());

    formatStringsContext.spaceString = ctx.builder->CreateGlobalString(" ", "space", 0U, ctx.LLVMModule.get());
}

Value* getFromatString(const ModuleContext& ctx,
                       const FormatStringsContext& formatStringsContext,
                       icode::DataType dtype)
{
    if (dtype == icode::I8)
        return formatStringsContext.byteFormatString;

    if (dtype == icode::UI8)
        return formatStringsContext.ubyteFormatString;

    if (dtype == icode::I16)
        return formatStringsContext.shortFormatString;

    if (dtype == icode::UI16)
        return formatStringsContext.ushortFormatString;

    if (dtype == icode::I32 || dtype == icode::INT)
        return formatStringsContext.intFormatString;

    if (dtype == icode::UI32)
        return formatStringsContext.uintFormatString;

    if (dtype == icode::I64)
        return formatStringsContext.longFormatString;

    if (dtype == icode::UI64)
        return formatStringsContext.ulongFormatString;

    if (dtype == icode::F32 || dtype == icode::FLOAT)
        return formatStringsContext.floatFormatString;

    if (dtype == icode::F64)
        return formatStringsContext.doubleFormatString;

    miklog::internal_error(ctx.moduleDescription.name);
    throw miklog::internal_bug_error();
}

Value* getFormatStringForStringInput(const ModuleContext& ctx, int charCount)
{
    std::string formatString = "%" + std::to_string(charCount-1) + "s";
    return ctx.builder->CreateGlobalString(formatString, "space", 0U, ctx.LLVMModule.get());
}