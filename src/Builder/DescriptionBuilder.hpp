#ifndef DESCRIPTION_BUILDER
#define DESCRIPTION_BUILDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"

class DescriptionBuilder
{
    icode::ModuleDescription* workingModule;
    Console& console;

    icode::StructDescription createEmptyStructDescription();

    std::pair<int, std::string> getSizeAndModuleName(const token::Token& dataTypeToken, icode::DataType dtype);

  public:
    void setWorkingModule(icode::ModuleDescription* moduleDesc);

    icode::VariableDescription createVoidVariableDescription();

    icode::VariableDescription createVariableDescription(const token::Token& dataTypeToken);

    icode::VariableDescription createArrayVariableDescription(const icode::VariableDescription& variableDesc,
                                                              std::vector<int>& dimensions);

    void createDefine(const token::Token& nameToken, const token::Token& valueToken);

    void createEnum(const std::vector<token::Token>& enums);

    void createFunctionDescription(const token::Token& nameToken,
                                   const icode::VariableDescription& returnType,
                                   const std::vector<token::Token>& paramNames,
                                   const std::vector<icode::VariableDescription>& paramTypes);

    void createGlobal(const token::Token globalNameToken, const icode::VariableDescription& variableDesc);

    void createStructDescription(const token::Token& nameToken,
                                 const std::vector<token::Token>& fieldNames,
                                 const std::vector<icode::VariableDescription>& fieldTypes);

    DescriptionBuilder(Console& console);
};

#endif