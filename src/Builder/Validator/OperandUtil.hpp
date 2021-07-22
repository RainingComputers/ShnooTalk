#ifndef VALIDATOR_OPERAND_UTIL
#define VALIDATOR_OPERAND_UTIL

#include "../../IntermediateRepresentation/Operand.hpp"

bool validDataType(const icode::Operand& op);
bool nonVoidBaseDataType(const icode::Operand& op);
bool nonVoidDataType(const icode::Operand& op);


bool none(const icode::Operand& op);
bool notNone(const icode::Operand& op);

#endif
