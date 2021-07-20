#ifndef VALIDATOR_OPERAND_UTIL
#define VALIDATOR_OPERAND_UTIL

#include "../../IntermediateRepresentation/Operand.hpp"

bool validDataType(const icode::Operand& op);
bool nonVoidBaseDataType(const icode::Operand& op);
bool voidDataType(const icode::Operand& op);

bool temp(const icode::Operand& op);
bool none(const icode::Operand& op);
bool notNone(const icode::Operand& op);
bool mod(const icode::Operand& op);
bool var(const icode::Operand& op);
bool label(const icode::Operand& op);
bool retValue(const icode::Operand& op);
bool notPointer(const icode::Operand& op);



#endif
