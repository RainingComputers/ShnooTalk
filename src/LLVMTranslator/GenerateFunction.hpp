#ifndef GENERATE_FUNCTION_HPP
#define GENERATE_FUNCTION_HPP

#include "BranchContext.hpp"
#include "ModuleContext.hpp"

void generateFunction(ModuleContext& ctx,
                      BranchContext& branchContext,
                      const FormatStringsContext& formatStringsContext,
                      const icode::FunctionDescription& functionDesc,
                      const std::string& name);

#endif