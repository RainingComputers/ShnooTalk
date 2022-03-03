#include "JSON/FlatJSONPrinter.hpp"
#include "JSON/JSONMapUtil.hpp"

#include "EntryPrinter.hpp"
#include "IRPrinter.hpp"
#include "PrettyPrintError.hpp"

namespace pp
{
    void printOperand(const icode::Operand& op, FlatJSONPrinter& jsonp)
    {
        jsonp.begin();

        jsonp.printString("type", operandTypeToString[op.operandType]);

        if (op.operandType != icode::NONE)
            jsonp.printNumber("id", op.operandId);

        switch (op.operandType)
        {
            case icode::TEMP:
            case icode::TEMP_PTR:
            case icode::TEMP_PTR_PTR:
            case icode::RET_VALUE:
            case icode::RET_PTR:
            case icode::CALLEE_RET_VAL:
            case icode::CALLEE_RET_PTR:
                jsonp.printString("dtype", icode::dataTypeToString(op.dtype));
                break;
            case icode::VAR:
            case icode::GBL_VAR:
            case icode::PTR:
                jsonp.printString("name", op.name);
                jsonp.printString("dtype", icode::dataTypeToString(op.dtype));
                break;
            case icode::STR_DATA:
                jsonp.printString("name", op.name);
                jsonp.printNumber("size", op.val.size);
                break;
            case icode::BYTES:
                jsonp.printNumber("bytes", op.val.bytes);
                break;
            case icode::LITERAL:
            {
                jsonp.printString("dtype", icode::dataTypeToString(op.dtype));

                if (icode::isInteger(op.dtype))
                    jsonp.printNumber("value", op.val.integer);
                else
                    jsonp.printNumber("value", op.val.floating);

                break;
            }
            case icode::LABEL:
            case icode::MODULE:
                jsonp.printString("name", op.name);
                break;
            case icode::NONE:
                break;
            default:
                throw PrettyPrintError();
        }

        jsonp.end();
    }

    void printEntry(const icode::Entry& entry, FlatJSONPrinter& jsonp)
    {
        jsonp.begin();

        jsonp.printString("opcode", instructionToString[entry.opcode]);

        FlatJSONPrinter op1Printer = jsonp.beginNested("op1");
        printOperand(entry.op1, op1Printer);

        FlatJSONPrinter op2Printer = jsonp.beginNested("op2");
        printOperand(entry.op2, op2Printer);

        FlatJSONPrinter op3Printer = jsonp.beginNested("op3");
        printOperand(entry.op3, op3Printer);

        jsonp.end();
    }

    std::vector<std::string> getPropertiesStringArrayFromType(const icode::TypeDescription& typeDescription)
    {
        std::vector<std::string> propertiesStringArray;

        for (unsigned int i = 0; i <= icode::IS_INCOMPLETE_TYPE; i += 1)
            if (typeDescription.checkProperty((icode::TypeProperties)i))
                propertiesStringArray.push_back(typePropertiesString[i]);

        return propertiesStringArray;
    }

    std::vector<std::string> getDimTypesStringArrayFromType(const icode::TypeDescription& typeDescription)
    {
        std::vector<std::string> dimTypesStringArray;

        for (icode::DimensionType dimType : typeDescription.dimTypes)
            dimTypesStringArray.push_back(dimensionTypesToString[dimType]);

        return dimTypesStringArray;
    }

    void printTypeDescription(const icode::TypeDescription& typeDescription, FlatJSONPrinter& jsonp)
    {
        jsonp.begin();
        jsonp.printString("dtype", icode::dataTypeToString(typeDescription.dtype));
        jsonp.printNumber("dtypeSize", typeDescription.dtypeSize);
        jsonp.printNumber("size", typeDescription.size);
        jsonp.printString("moduleName", typeDescription.moduleName);
        jsonp.printNumber("offset", typeDescription.offset);
        jsonp.printIntArray("dimensions", typeDescription.dimensions);
        jsonp.printStringArray("dimTypes", getDimTypesStringArrayFromType(typeDescription));
        jsonp.printStringArray("properties", getPropertiesStringArrayFromType(typeDescription));
        jsonp.end();
    }

    void printTypeDescriptionMap(const std::map<std::string, icode::TypeDescription>& typeMap, FlatJSONPrinter& jsonp)
    {
        jsonp.begin();

        for (auto pair : typeMap)
        {
            FlatJSONPrinter typePrinter = jsonp.beginNested(pair.first, true);
            printTypeDescription(pair.second, typePrinter);
        }

        jsonp.end();
    }

    void printStructDescription(const icode::StructDescription& structDesc, FlatJSONPrinter& jsonp)
    {
        jsonp.begin();

        FlatJSONPrinter fieldsPrinter = jsonp.beginNested("structFields");
        printTypeDescriptionMap(structDesc.structFields, fieldsPrinter);

        jsonp.printNumber("size", structDesc.size);
        jsonp.printString("moduleName", structDesc.moduleName);

        jsonp.end();
    }

    void printStructDescriptionMap(const std::map<std::string, icode::StructDescription>& structsMap,
                                   FlatJSONPrinter& jsonp)
    {
        jsonp.begin();

        for (auto pair : structsMap)
        {
            FlatJSONPrinter structPrinter = jsonp.beginNested(pair.first);
            printStructDescription(pair.second, structPrinter);
        }

        jsonp.end();
    }

    void printFunctionDescription(const icode::FunctionDescription& functionDesc, FlatJSONPrinter& jsonp, bool jsonIR)
    {
        jsonp.begin();

        jsonp.printString("moduleName", functionDesc.moduleName);

        jsonp.printStringArray("parameters", functionDesc.parameters);

        FlatJSONPrinter typePrinter = jsonp.beginNested("functionReturnType", true);
        printTypeDescription(functionDesc.functionReturnType, typePrinter);

        FlatJSONPrinter symbolsPrinter = jsonp.beginNested("symbols");
        printTypeDescriptionMap(functionDesc.symbols, symbolsPrinter);

        if (jsonIR)
        {
            FlatJSONPrinter entryPrinter = jsonp.beginArray("icode", true);
            for (icode::Entry e : functionDesc.icodeTable)
                printEntry(e, entryPrinter);
            jsonp.endArray();
        }
        else
        {
            prettyPrintIcodeTable(functionDesc.icodeTable, jsonp.getBaseIndentLevel(), jsonp.getIndentWidth());
        }

        jsonp.end();
    }

    void printFunctionDescriptionMap(const std::map<std::string, icode::FunctionDescription>& functionsMap,
                                     FlatJSONPrinter& jsonp,
                                     bool jsonIR)
    {
        jsonp.begin();

        for (auto pair : functionsMap)
        {
            FlatJSONPrinter funcPrinter = jsonp.beginNested(pair.first);
            printFunctionDescription(pair.second, funcPrinter, jsonIR);
        }

        jsonp.end();
    }

    void printModuleDescription(const icode::ModuleDescription& moduleDescription, FlatJSONPrinter& jsonp, bool jsonIR)
    {
        jsonp.begin();

        jsonp.printString("moduleName", moduleDescription.name);

        jsonp.printStringArray("uses", moduleDescription.uses);

        FlatJSONPrinter aliasesPrinter = jsonp.beginNested("aliases");
        printStringMap(moduleDescription.aliases, aliasesPrinter);

        FlatJSONPrinter enumPrinter = jsonp.beginNested("enumerations");
        printNumberMap<int>(moduleDescription.enumerations, enumPrinter);

        FlatJSONPrinter intDefinePrinter = jsonp.beginNested("intDefines");
        printNumberMap<long>(moduleDescription.intDefines, intDefinePrinter);

        FlatJSONPrinter floatDefinePrinter = jsonp.beginNested("floatDefines");
        printNumberMap<double>(moduleDescription.floatDefines, floatDefinePrinter);

        FlatJSONPrinter stringDefinePrinter = jsonp.beginNested("stringDefines");
        printStringMap(moduleDescription.stringDefines, stringDefinePrinter);

        FlatJSONPrinter stringsDataCharCountPrinter = jsonp.beginNested("stringsDataCharCounts");
        printNumberMap<int>(moduleDescription.stringsDataCharCounts, stringsDataCharCountPrinter);

        FlatJSONPrinter globalsPrinter = jsonp.beginNested("globals");
        printTypeDescriptionMap(moduleDescription.globals, globalsPrinter);

        FlatJSONPrinter structsPrinter = jsonp.beginNested("structures");
        printStructDescriptionMap(moduleDescription.structures, structsPrinter);

        FlatJSONPrinter externFunctionsPrinter = jsonp.beginNested("externFunctions");
        printFunctionDescriptionMap(moduleDescription.externFunctions, externFunctionsPrinter, jsonIR);

        FlatJSONPrinter functionsPrinter = jsonp.beginNested("functions");
        printFunctionDescriptionMap(moduleDescription.functions, functionsPrinter, jsonIR);

        jsonp.end();
    }

    void printModule(const icode::ModuleDescription& moduleDescription, bool jsonIR)
    {
        FlatJSONPrinter jsonp(0);
        printModuleDescription(moduleDescription, jsonp, jsonIR);
    }

    void printModulesMap(const icode::StringModulesMap& modulesMap, bool jsonIR)
    {
        FlatJSONPrinter jsonp(0);
        jsonp.begin();

        for (auto pair : modulesMap)
        {
            FlatJSONPrinter moduleDescriptionPrinter = jsonp.beginNested(pair.first);
            printModuleDescription(pair.second, moduleDescriptionPrinter, jsonIR);
        }

        jsonp.end();
    }

    void printFunctionDescriptionIcodeOnly(const icode::FunctionDescription& functionDesc,
                                           FlatJSONPrinter& jsonp,
                                           bool jsonIR)
    {
        jsonp.begin();

        if (jsonIR)
        {
            FlatJSONPrinter entryPrinter = jsonp.beginArray("icode", true);
            for (icode::Entry e : functionDesc.icodeTable)
                printEntry(e, entryPrinter);
            jsonp.endArray();
        }
        else
        {
            prettyPrintIcodeTable(functionDesc.icodeTable, jsonp.getBaseIndentLevel(), jsonp.getIndentWidth());
        }

        jsonp.end();
    }

    void printFunctionDescriptionMapIcodeOnly(const std::map<std::string, icode::FunctionDescription>& functionsMap,
                                              FlatJSONPrinter& jsonp,
                                              bool jsonIR)
    {
        jsonp.begin();

        for (auto pair : functionsMap)
        {
            FlatJSONPrinter funcPrinter = jsonp.beginNested(pair.first);
            printFunctionDescriptionIcodeOnly(pair.second, funcPrinter, jsonIR);
        }

        jsonp.end();
    }

    void printModuleDescriptionIcodeOnly(const icode::ModuleDescription& moduleDescription, FlatJSONPrinter& jsonp, bool jsonIR) 
    {
        jsonp.begin();

        jsonp.printString("moduleName", moduleDescription.name);

        FlatJSONPrinter functionsPrinter = jsonp.beginNested("functions");
        printFunctionDescriptionMapIcodeOnly(moduleDescription.functions, functionsPrinter, jsonIR);

        jsonp.end();
    }

    void printModuleIcodeOnly(const icode::ModuleDescription& moduleDescription, bool jsonIR)
    {
        FlatJSONPrinter jsonp(0);
        printModuleDescriptionIcodeOnly(moduleDescription, jsonp, jsonIR);
    }

    void printModulesMapIcodeOnly(const icode::StringModulesMap& modulesMap, bool jsonIR)
    {
        FlatJSONPrinter jsonp(0);
        jsonp.begin();

        for (auto pair : modulesMap)
        {
            FlatJSONPrinter moduleDescriptionPrinter = jsonp.beginNested(pair.first);
            printModuleDescriptionIcodeOnly(pair.second, moduleDescriptionPrinter, jsonIR);
        }

        jsonp.end();
    }

} // namespace mikpp