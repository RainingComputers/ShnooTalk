#include "../../irgen.hpp"

icode::VariableDescription createVoidVariableDescription(const irgen::ir_generator& ctx);
icode::VariableDescription createVariableDescription(irgen::ir_generator& ctx, const token::Token& dataTypeToken);
icode::VariableDescription createArrayVariableDescription(const icode::VariableDescription& variableDesc,
                                                     std::vector<int>& dimensions);