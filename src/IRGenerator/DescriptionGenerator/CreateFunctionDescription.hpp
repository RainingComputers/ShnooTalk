#include "../../irgen.hpp"

void createFunctionDescription(irgen::ir_generator& ctx,
                               const token::Token& nameToken,
                               const icode::VariableDescription& returnType,
                               const std::vector<token::Token>& paramNames,
                               const std::vector<icode::VariableDescription>& paramTypes);