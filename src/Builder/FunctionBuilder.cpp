#include "FunctionBuilder.hpp"

using namespace icode;

FunctionBuilder::FunctionBuilder(StringModulesMap& modulesMap,
                                 Console& console,
                                 OperandBuilder& opBuilder,
                                 EntryBuilder& entryBuilder)

  : modulesMap(modulesMap)
  , console(console)
  , opBuilder(opBuilder)
  , entryBuilder(entryBuilder)
{
}

icode::Operand FunctionBuilder::ensurePointerOperand(const icode::Operand& op)
{
    if (!op.isPointer())
        return entryBuilder.createPointer(op);

    return op;
}

Unit FunctionBuilder::getStructField(const Token& fieldName, const Unit& unit)
{
    StructDescription structDescription = modulesMap[unit.second.moduleName].structures[unit.second.dtypeName];

    TypeDescription fieldType;

    if (!structDescription.getField(fieldName.toString(), fieldType))
        console.compileErrorOnToken("Undefined STRUCT field", fieldName);

    if (unit.second.isMutable())
        fieldType.becomeMutable();

    Operand fieldOperand = ensurePointerOperand(unit.first);
    fieldOperand.updateDataType(fieldType);

    fieldOperand =
      entryBuilder.addressAddOperator(fieldOperand, opBuilder.createLiteralAddressOperand(fieldType.offset));

    return Unit(fieldOperand, fieldType);
}

Unit FunctionBuilder::getIndexedElement(const Unit& unit, const std::vector<Unit>& indices)
{
    unsigned int dimensionCount = 0;
    unsigned int elementWidth = unit.second.size / unit.second.dimensions[0];

    Operand elementOperand = ensurePointerOperand(unit.first);

    TypeDescription elementType = unit.second;

    for (const Unit indexUnit : indices)
    {
        Operand subscriptOperand =
          entryBuilder.addressMultiplyOperator(indexUnit.first, opBuilder.createLiteralAddressOperand(elementWidth));

        if (dimensionCount + 1 != elementType.dimensions.size())
            elementWidth /= elementType.dimensions[dimensionCount + 1];

        elementOperand = entryBuilder.addressAddOperator(elementOperand, subscriptOperand);

        dimensionCount++;
    }

    unsigned int remainingDimensionCount = elementType.dimensions.size() - dimensionCount;

    elementType.dimensions.erase(elementType.dimensions.begin(),
                                 elementType.dimensions.end() - remainingDimensionCount);

    return Unit(elementOperand, elementType);
}
