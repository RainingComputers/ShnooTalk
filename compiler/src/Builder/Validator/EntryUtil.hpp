#ifndef VALIDATOR_ENTRY_UTIL
#define VALIDATOR_ENTRY_UTIL

#include "OperandUtil.hpp"
#include "../../IntermediateRepresentation/Entry.hpp"

bool allOperandEqualDataType(const icode::Entry& e);
bool allOperandNonVoidBaseDataType(const icode::Entry& e);
bool allOperandNonVoidDataType(const icode::Entry& e);
bool allOperandNonPointer(const icode::Entry& e);
bool noOperand(const icode::Entry& e);
bool oneOperand(const icode::Entry& e);
bool twoOperand(const icode::Entry& e);
bool threeOperand(const icode::Entry& e);

#endif