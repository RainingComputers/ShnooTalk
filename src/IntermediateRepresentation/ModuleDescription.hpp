#ifndef IR_MODULE_DESCRIPTION
#define IR_MODULE_DESCRIPTION

#include <map>
#include <string>
#include <vector>

#include "EnumDescription.hpp"
#include "FunctionDescription.hpp"
#include "StructDescription.hpp"

namespace icode
{
    struct ModuleDescription
    {
        std::string name;
        std::vector<std::string> uses;
        std::map<std::string, std::string> aliases;

        std::map<std::string, std::string> incompleteTypes;
        std::map<std::string, StructDescription> structures;

        std::vector<std::string> definedFunctions;
        std::map<std::string, std::string> incompleteFunctions;
        std::map<std::string, FunctionDescription> functions;
        std::map<std::string, FunctionDescription> externFunctions;

        std::vector<std::string> definedEnumsTypes;
        std::map<std::string, EnumDescription> enums;
        std::map<std::string, long> intDefines;
        std::map<std::string, double> floatDefines;
        std::map<std::string, TypeDescription> globals;

        std::map<std::string, std::string> stringDefines;
        std::map<std::string, std::string> stringsData;
        std::map<std::string, int> stringsDataCharCounts;

        ModuleDescription();

        bool getModuleNameFromAlias(const std::string& name, std::string& returnValue);
        bool getStruct(const std::string& name, StructDescription& returnValue);
        bool getFunction(const std::string& name, FunctionDescription& returnValue);
        bool getExternFunction(const std::string& name, FunctionDescription& returnValue);
        bool getEnum(const std::string& name, EnumDescription& returnValue);
        bool getIntDefine(const std::string& name, long& returnValue);
        bool getFloatDefine(const std::string& name, double& returnValue);
        bool getStringDefine(const std::string& name, std::string& returnValue);
        bool getGlobal(const std::string& name, TypeDescription& returnValue);
        bool getIncompleteTypeModule(const std::string& typeName, std::string& returnValue);
        bool getIncompleteFunctionModule(const std::string& functionName, std::string& returnValue);
        bool enumTypeExists(const std::string&);
        bool useExists(const std::string&);
        bool aliasExists(const std::string&);
        bool symbolExists(const std::string&);
        void setDeconstructor(const TypeDescription& type, const std::string& mangledFunctionName);
    };

    typedef std::map<std::string, ModuleDescription> StringModulesMap;
}

#endif