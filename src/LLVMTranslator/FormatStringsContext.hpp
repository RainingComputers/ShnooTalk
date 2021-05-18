#ifndef FORMAT_STRINGS_CONTEXT
#define FORMAT_STRINGS_CONTEXT

#include "llvm/IR/Value.h"

#include "ModuleContext.hpp"

struct FormatStringsContext
{
    llvm::Value* byteFormatString;
    llvm::Value* ubyteFormatString;
    llvm::Value* shortFormatString;
    llvm::Value* ushortFormatString;
    llvm::Value* intFormatString;
    llvm::Value* uintFormatString;
    llvm::Value* longFormatString;
    llvm::Value* ulongFormatString;
    llvm::Value* floatFormatString;
    llvm::Value* doubleFormatString;
    
    llvm::Value* newLineString;
    llvm::Value* spaceString;
};

void setupFormatStringsContext(const ModuleContext& ctx, FormatStringsContext& formatStringsContext);
llvm::Value* getFromatString(const ModuleContext& ctx,
                       const FormatStringsContext& formatStringsContext,
                       icode::DataType dtype);
llvm::Value* getFormatStringForStringInput(const ModuleContext& ctx, int charCount);

#endif