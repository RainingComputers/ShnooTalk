#include "../../irgen.hpp"

void createStructDescription(irgen::ir_generator& ctx,
                             const token::Token& nameToken,
                             const std::vector<token::Token>& fieldNames,
                             const std::vector<icode::VariableDescription>& fieldTypes);