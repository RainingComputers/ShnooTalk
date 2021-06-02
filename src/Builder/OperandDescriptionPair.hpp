#ifndef OPERAND_DESCRIPTION_PAIR
#define OPERAND_DESCRIPTION_PAIR

#include <utility>

#include "../IntermediateRepresentation/Operand.hpp"
#include "../IntermediateRepresentation/TypeDescription.hpp"

typedef std::pair<icode::Operand, icode::TypeDescription> OperandDescriptionPair;

#endif