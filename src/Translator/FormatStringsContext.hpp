#ifndef TRANSLATOR_FORMAT_STRINGS_CONTEXT
#define TRANSLATOR_FORMAT_STRINGS_CONTEXT

#include <map>

#include "llvm/IR/Value.h"

#include "ModuleContext.hpp"

struct FormatStringsContext
{
    std::map<icode::DataType, llvm::Value*> dataTypeFormatStringsMap;

    llvm::Value* charInputFormatString;
    llvm::Value* newLineString;
    llvm::Value* spaceString;
};

void setupFormatStringsContext(const ModuleContext& ctx, FormatStringsContext& formatStringsContext);
llvm::Value* getFromatStringFromDataTypePrintf(const FormatStringsContext& formatStringsContext, icode::DataType dtype);
llvm::Value* getFromatStringFromDataTypeScanf(const FormatStringsContext& formatStringsContext, icode::DataType dtype);
llvm::Value* getFormatStringForStringInput(const ModuleContext& ctx, int charCount);

#endif