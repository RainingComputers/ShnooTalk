#ifndef BUILDER_TYPE_CHECKER
#define BUILDER_TYPE_CHECKER

#include "Unit.hpp"

#include "../IntermediateRepresentation/TypeDescription.hpp"

bool isSameType(const Unit& unit1, const Unit& unit2);
bool isSameTypeDescription(const icode::TypeDescription& type1, const icode::TypeDescription& type2);

#endif