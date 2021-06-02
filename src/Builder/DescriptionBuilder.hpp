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
    void setWorkingModule(icode::ModuleDescription* moduleDescription);

    icode::ModuleDescription* getModuleFromToken(const token::Token& moduleNameToken,
                                                 icode::StringModulesMap& modulesMap);

    icode::TypeDescription createVoidTypeDescription();

    icode::TypeDescription createTypeDescription(const token::Token& dataTypeToken);

    icode::TypeDescription createArrayTypeDescription(const icode::TypeDescription& typeDescription,
                                                      std::vector<int>& dimensions);

    void createDefine(const token::Token& nameToken, const token::Token& valueToken);

    void createEnum(const std::vector<token::Token>& enums);

    void createFunctionDescription(const token::Token& nameToken,
                                   const icode::TypeDescription& returnType,
                                   const std::vector<token::Token>& paramNames,
                                   std::vector<icode::TypeDescription>& paramTypes);

    void createGlobal(const token::Token globalNameToken, icode::TypeDescription& typeDescription);

    void createStructDescription(const token::Token& nameToken,
                                 const std::vector<token::Token>& fieldNames,
                                 const std::vector<icode::TypeDescription>& fieldTypes);

    DescriptionBuilder(Console& console);
};

#endif