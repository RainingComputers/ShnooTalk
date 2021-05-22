#include "../irgen.hpp"

icode::VariableDescription constructVoidVariableDesc(const irgen::ir_generator& ctx);
icode::VariableDescription variableDescFromDataTypeToken(irgen::ir_generator& ctx, const token::Token& dataTypeToken);
icode::VariableDescription addDimensionToDescription(const icode::VariableDescription& variableDesc,
                                                     std::vector<int>& dimensions);