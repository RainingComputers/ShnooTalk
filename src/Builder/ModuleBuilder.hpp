#ifndef BUILDER_MODULE_BUILDER
#define BUILDER_MODULE_BUILDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"

struct TypeInformation
{
    unsigned int dtypeSize;
    std::string moduleName;
    bool incompleteType;
};

class ModuleBuilder
{
    icode::ModuleDescription& rootModule;
    icode::StringModulesMap& modulesMap;
    Console& console;

    icode::ModuleDescription* workingModule;

    icode::StructDescription createEmptyStructDescription();

    TypeInformation getTypeInformation(const Token& dataTypeToken, icode::DataType dtype);

    icode::FunctionDescription createFunctionDescription(const icode::TypeDescription& returnType,
                                                         const std::vector<Token>& paramNames,
                                                         std::vector<icode::TypeDescription>& paramTypes,
                                                         const std::string& moduleName,
                                                         const std::string& absoluteName);

    void createFrom(const std::string& moduleName, const Token& aliasToken);

    bool isValidDeconstructor(const icode::FunctionDescription& function);

    void setDeconstructor(const icode::TypeDescription& type, const std::string& mangledFunctionName);

    void createUseNoAlias(const Token& pathToken);

    void createEnumType(const Token& nameToken);

public:
    ModuleBuilder(icode::ModuleDescription& rootModule, icode::StringModulesMap& modulesMap, Console& console);

    void setWorkingModule(icode::ModuleDescription* moduleDescription);

    void registerIncompleteType(const Token& typeToken);

    icode::DataType tokenToDataType(const Token& token);

    icode::TypeDescription createVoidTypeDescription();

    icode::TypeDescription createTypeDescription(const Token& dataTypeToken);

    void createIntDefine(const Token& nameToken, int value);

    void createFloatDefine(const Token& nameToken, float value);

    std::string createMultilineStringData(const std::vector<Token>& tokens);

    std::string createStringData(const Token& stringToken);

    void createStringDefine(const Token& nameToken, const Token& valueToken);

    void createEnum(const Token& nameToken, const std::vector<Token>& enums);

    void createFunction(const Token& nameToken,
                        const icode::TypeDescription& returnType,
                        const std::vector<Token>& paramNames,
                        std::vector<icode::TypeDescription>& paramTypes);

    void createFunctionExternC(const Token& nameToken,
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
                      const std::vector<Token>& fieldNameTokens,
                      const std::vector<icode::TypeDescription>& fieldTypes);

    void createUse(const Token& pathToken, const Token& aliasToken);

    void createDirectFrom(const Token& pathToken, const Token& symbolNameToken);

    void createAliasFrom(const Token& aliasToken, const Token& symbolNameToken);
};

#endif