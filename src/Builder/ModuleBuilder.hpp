#ifndef BUILDER_MODULE_BUILDER
#define BUILDER_MODULE_BUILDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"

class ModuleBuilder
{
    icode::ModuleDescription& rootModule;
    icode::StringModulesMap& modulesMap;
    Console& console;

    icode::ModuleDescription* workingModule;

    icode::StructDescription createEmptyStructDescription();

    std::pair<int, std::string> getSizeAndModuleName(const Token& dataTypeToken, icode::DataType dtype);

    icode::FunctionDescription createFunctionDescription(const icode::TypeDescription& returnType,
                                                         const std::vector<Token>& paramNames,
                                                         std::vector<icode::TypeDescription>& paramTypes, 
                                                         const std::string& moduleName);

public:
    ModuleBuilder(icode::ModuleDescription& rootModule, icode::StringModulesMap& modulesMap, Console& console);

    void setWorkingModule(icode::ModuleDescription* moduleDescription);

    icode::TypeDescription createVoidTypeDescription();

    icode::TypeDescription createTypeDescription(const Token& dataTypeToken);

    icode::TypeDescription createTypeDescriptionFromStructName(const std::string& dtype);

    void createIntDefine(const Token& nameToken, int value);

    void createFloatDefine(const Token& nameToken, float value);

    std::string createMultilineStringData(const std::vector<Token>& tokens);

    std::string createStringData(const Token& stringToken);

    void createStringDefine(const Token& nameToken, const Token& valueToken);

    void createEnum(const std::vector<Token>& enums);

    void createFunction(const Token& nameToken,
                        const icode::TypeDescription& returnType,
                        const std::vector<Token>& paramNames,
                        std::vector<icode::TypeDescription>& paramTypes);

    void createExternFunction(const Token& nameToken,
                              const icode::TypeDescription& returnType,
                              const std::vector<Token>& paramNames,
                              std::vector<icode::TypeDescription>& paramTypes);


    void createExternFunctionModule(const Token& nameToken,
                                    const icode::TypeDescription& returnType,
                                    const std::vector<Token>& paramNames,
                                    std::vector<icode::TypeDescription>& paramTypes,
                                    const Token& moduleNameToken);


    void createGlobal(const Token globalNameToken, icode::TypeDescription& typeDescription);

    void createStruct(const Token& nameToken,
                      const std::vector<Token>& fieldNames,
                      const std::vector<icode::TypeDescription>& fieldTypes);

    void createUse(const Token& pathToken, const Token& aliasToken);

    void createFrom(const Token& aliasToken, const Token& symbolNameToken);
};

#endif