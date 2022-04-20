#ifndef BUILDER_OPTIMIZERS_CONST_STRUCT_OR_ARRAY_TO_POINTER
#define BUILDER_OPTIMIZERS_CONST_STRUCT_OR_ARRAY_TO_POINTER

#include "../OperandBuilder.hpp"

void constStructsToPointer(OperandBuilder& opBuilder, icode::FunctionDescription& functionDesc);

#endif