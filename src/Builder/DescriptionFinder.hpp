#ifndef DESCRIPTION_FINDER
#define DESCRIPTION_FINDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"
#include "../Token/Token.hpp"
#include "OperandBuilder.hpp"
#include "OperandDescriptionPair.hpp"
#include "ValueBuilder.hpp"

class DescriptionFinder
{
    icode::ModuleDescription& rootModule;
    icode::ModuleDescription* workingModule;
    icode::FunctionDescription* workingFunction;

    Console& console;

    ValueBuilder& valueBuilder;

  public:
    DescriptionFinder(icode::ModuleDescription& rootModule, Console& console, ValueBuilder& valueBuilder);

    void setWorkingModule(icode::ModuleDescription* module);
    void setWorkingFunction(icode::FunctionDescription* function);

    bool getLocal(const token::Token& nameToken, OperandDescriptionPair& returnValue);
    bool getGlobal(const token::Token& nameToken, OperandDescriptionPair& returnValue);
    bool getDefine(const token::Token& nameToken, OperandDescriptionPair& returnValue);
    bool getEnum(const token::Token& nameToken, OperandDescriptionPair& returnValue);

    OperandDescriptionPair getValueFromToken(const token::Token& nameToken);
};

#endif