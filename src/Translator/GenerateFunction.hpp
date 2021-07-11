#ifndef TRANSLATOR_GENERATE_FUNCTION
#define TRANSLATOR_GENERATE_FUNCTION

#include "BranchContext.hpp"
#include "ModuleContext.hpp"

void generateFunction(ModuleContext& ctx,
                      BranchContext& branchContext,
                      const FormatStringsContext& formatStringsContext,
                      const icode::FunctionDescription& functionDesc,
                      const std::string& name);

#endif