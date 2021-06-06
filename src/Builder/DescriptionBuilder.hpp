#ifndef DESCRIPTION_BUILDER
#define DESCRIPTION_BUILDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"

class ModuleBuilder
{
    icode::ModuleDescription* workingModule;
    Console& console;

    icode::StructDescription createEmptyStructDescription();

    std::pair<int, std::string> getSizeAndModuleName(const Token& dataTypeToken, icode::DataType dtype);

  public:
    void setWorkingModule(icode::ModuleDescription* moduleDescription);



    icode::TypeDescription createVoidTypeDescription();

    icode::TypeDescription createTypeDescription(const Token& dataTypeToken);

    icode::TypeDescription createArrayTypeDescription(const icode::TypeDescription& typeDescription,
                                                      std::vector<int>& dimensions);

    void createDefine(const Token& nameToken, const Token& valueToken);

    void createEnum(const std::vector<Token>& enums);

    void createFunctionDescription(const Token& nameToken,
                                   const icode::TypeDescription& returnType,
                                   const std::vector<Token>& paramNames,
                                   std::vector<icode::TypeDescription>& paramTypes);

    void createGlobal(const Token globalNameToken, icode::TypeDescription& typeDescription);

    void createStructDescription(const Token& nameToken,
                                 const std::vector<Token>& fieldNames,
                                 const std::vector<icode::TypeDescription>& fieldTypes);

    ModuleBuilder(Console& console);
};

#endif