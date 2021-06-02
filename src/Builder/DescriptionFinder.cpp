#include "DescriptionFinder.hpp"

using namespace icode;

DescriptionFinder::DescriptionFinder(ModuleDescription& rootModule, Console& console, ValueBuilder& valueBuilder)
  : rootModule(rootModule)
  , console(console)
  , valueBuilder(valueBuilder)
{
}

void DescriptionFinder::setWorkingModule(ModuleDescription* moduleDescription)
{
    workingModule = moduleDescription;
}

void DescriptionFinder::setWorkingFunction(FunctionDescription* functionDescription)
{
    workingFunction = functionDescription;
}

bool DescriptionFinder::getLocal(const token::Token& nameToken, OperandDescriptionPair& returnValue)
{
    TypeDescription typeDescription;

    if (!workingFunction->getSymbol(nameToken.toString(), typeDescription))
        return false;

    returnValue = valueBuilder.operandDescPairFromTypeDesc(typeDescription, nameToken);
    return true;
}

bool DescriptionFinder::getGlobal(const token::Token& nameToken, OperandDescriptionPair& returnValue)
{
    TypeDescription typeDescription;

    if (!rootModule.getGlobal(nameToken.toString(), typeDescription))
        return false;

    returnValue = valueBuilder.operandDescPairFromTypeDesc(typeDescription, nameToken);
    return true;
}

bool DescriptionFinder::getEnum(const token::Token& nameToken, OperandDescriptionPair& returnValue)
{
    int enumValue;

    if (!workingModule->getEnum(nameToken.toString(), enumValue))
        if (!rootModule.getEnum(nameToken.toString(), enumValue))
            return false;

    returnValue = valueBuilder.operandDescPairFromEnum(enumValue);
    return true;
}

bool DescriptionFinder::getDefine(const token::Token& nameToken, OperandDescriptionPair& returnValue)
{
    DefineDescription defineDescription;

    if (!workingModule->getDefineDescription(nameToken.toString(), defineDescription))
        if (!rootModule.getDefineDescription(nameToken.toString(), defineDescription))
            return false;

    returnValue = valueBuilder.operandDescPairFromDefine(defineDescription);
    return true;
}

OperandDescriptionPair DescriptionFinder::getValueFromToken(const token::Token& nameToken)
{
    OperandDescriptionPair operandDescriptionPair;

    if (getLocal(nameToken, operandDescriptionPair))
        return operandDescriptionPair;

    if (getGlobal(nameToken, operandDescriptionPair))
        return operandDescriptionPair;

    if (getEnum(nameToken, operandDescriptionPair))
        return operandDescriptionPair;

    if (getDefine(nameToken, operandDescriptionPair))
        return operandDescriptionPair;

    console.compileErrorOnToken("Symbol does not exist", nameToken);
}