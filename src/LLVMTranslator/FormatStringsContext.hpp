#ifndef FORMAT_STRINGS_CONTEXT
#define FORMAT_STRINGS_CONTEXT

#include "llvm/IR/Value.h"

struct FormatStringsContext
{
    llvm::Value* uintFormatString;
    llvm::Value* intFormatString;
    llvm::Value* floatFormatString;
    llvm::Value* newLineString;
    llvm::Value* spaceString;
};

#endif