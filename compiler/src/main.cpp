#include <fstream>
#include <iostream>

#include "Console/Console.hpp"
#include "Generator/IRGenerator.hpp"
#include "PrettyPrint/ASTPrinter.hpp"
#include "PrettyPrint/IRPrinter.hpp"
#include "PrettyPrint/PrettyPrintError.hpp"
#include "Translator/LLVMTranslator.hpp"
#include "Utils/KeyExistsInMap.hpp"

#include "config.hpp"

static std::map<std::string, translator::Platform> platformMap = {
    { "-linux-x86_64", translator::LINUX_x86_64 },
    { "-linux-arm64", translator::LINUX_ARM64 },
    { "-macos-x86_64", translator::MACOS_x86_64 },
    { "-macos-arm64", translator::MACOS_ARM64 },
    { "-wasm32", translator::WASM32 },
    { "-wasm64", translator::WASM64 },
    { "-arm-cortex_m4-hardfloat", translator::ARM_CORTEX_M4_HARD_FLOAT },
};

void printCLIUsage()
{
    pp::println("USAGE: shtkc FILE OPTION");
    pp::println("");
    pp::println("Available options:");
    pp::println("    -c               Create debug executable");
    pp::println("    -release         Create release executable");
    pp::println("    -ast             Print parse tree");
    pp::println("    -ir              Print ShnooTalk IR");
    pp::println("    -icode           Print ShnooTalk IR, but only the icode");
    pp::println("    -ir-all          Print ShnooTalk IR recursively for all modules");
    pp::println("    -llvm            Print LLVM IR");
    pp::println("    -llvm-release    Print LLVM IR after optimization");
    pp::println("    -json-ast        Print parse tree in JSON");
    pp::println("    -json-ir         Print ShnooTalk IR in JSON");
    pp::println("    -json-ir-all     Print ShnooTalk IR recursively for all modules in json");
    pp::println("    -json-icode      Print ShnooTalk IR in JSON, but only the icode");
    pp::println("");
    pp::println("Cross compilation options for release executable:");
    for (const auto& pair : platformMap)
        pp::println("    " + pair.first);
    pp::println("");
    pp::println("Use shtkc -version for compiler version");
}

bool isValidPlatformString(const std::string& platformString)
{
    return keyExistsInMap(platformMap, platformString);
}

translator::Platform getPlatformFromString(const std::string& platformString)
{
    return platformMap.at(platformString);
}

int phaseDriver(const std::string& moduleName, const std::string& option, Console& console)
{
    console.pushRootModule(moduleName);

    if (option == "-ast")
    {
        pp::printNode(generator::generateAST(console));
        return 0;
    }

    if (option == "-json-ast")
    {
        pp::printJSONAST(generator::generateAST(console));
        return 0;
    }

    icode::StringModulesMap modulesMap;
    monomorphizer::StringGenericASTMap genericsMap;
    generator::generateIR(console, moduleName, modulesMap, genericsMap);

    if (option == "-ir")
        pp::printModule(modulesMap[moduleName], false);
    else if (option == "-icode")
        pp::printModuleIcodeOnly(modulesMap[moduleName], false);
    else if (option == "-ir-all")
        pp::printModulesMap(modulesMap, false);
    else if (option == "-icode-all")
        pp::printModulesMapIcodeOnly(modulesMap, false);
    else if (option == "-json-ir")
        pp::printModule(modulesMap[moduleName], true);
    else if (option == "-json-icode")
        pp::printModuleIcodeOnly(modulesMap[moduleName], true);
    else if (option == "-json-ir-all")
        pp::printModulesMap(modulesMap, true);
    else if (option == "-json-icode-all")
        pp::printModulesMapIcodeOnly(modulesMap, true);
    else if (option == "-llvm")
        pp::println(translator::generateLLVMModuleString(modulesMap[moduleName], modulesMap, false, console));
    else if (option == "-llvm-release")
        pp::println(translator::generateLLVMModuleString(modulesMap[moduleName], modulesMap, true, console));
    else if (option == "-c")
        for (auto stringModulePair : modulesMap)
            translator::generateObject(stringModulePair.second, modulesMap, translator::DEFAULT, false, console);
    else if (option == "-release")
        for (auto stringModulePair : modulesMap)
            translator::generateObject(stringModulePair.second, modulesMap, translator::DEFAULT, true, console);
    else if (isValidPlatformString(option))
    {
        for (auto stringModulePair : modulesMap)
        {
            translator::generateObject(stringModulePair.second,
                                       modulesMap,
                                       getPlatformFromString(option),
                                       true,
                                       console);
        }
    }
    else
    {
        printCLIUsage();
        return EXIT_FAILURE;
    }

    console.popModule();
    return 0;
}

int main(int argc, char* argv[])
{
    Console console;

    if (argc < 2)
    {
        printCLIUsage();
        return EXIT_FAILURE;
    }

    std::string fileName = argv[1];

    if (fileName == "-version" && argc == 2)
    {
        pp::println(VERSION);
        return EXIT_SUCCESS;
    }

    if (argc != 3)
    {
        printCLIUsage();
        return EXIT_FAILURE;
    }

    std::string option = argv[2];

    try
    {
        return phaseDriver(fileName, option, console);
    }
    catch (const CompileError)
    {
        return EXIT_FAILURE;
    }
    catch (const InternalBugError)
    {
        return EXIT_FAILURE;
    }
    catch (const PrettyPrintError&)
    {
        pp::println("Pretty print error");
        pp::println("REPORT THIS BUG");
        return EXIT_FAILURE;
    }
    catch (const std::ifstream::failure& error)
    {
        pp::println("File I/O error");
        return EXIT_FAILURE;
    }
    // catch (...)
    // {
    //     pp::println("Unknown error or an internal compiler error,");
    //     pp::println("REPORT THIS BUG");
    //     return EXIT_FAILURE;
    // }

    return 0;
}
