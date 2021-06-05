#ifndef FUNCTION_BUILDER
#define FUNCTION_BUILDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"
#include "EntryBuilder.hpp"
#include "UnitBuilder.hpp"

class FunctionBuilder
{
    icode::StringModulesMap& modulesMap;

    Console& console;
    OperandBuilder& opBuilder;
    EntryBuilder& entryBuilder;

    icode::Operand ensurePointerOperand(const icode::Operand& op);

  public:
    FunctionBuilder(icode::StringModulesMap& modulesMap,
                    Console& console,
                    OperandBuilder& opBuilder,
                    EntryBuilder& entryBuilder);

    Unit getStructField(const Token& fieldName, const Unit& unit);

    Unit getIndexedElement(const Unit& unit, const std::vector<Unit>& indices);
};

#endif