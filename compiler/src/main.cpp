#include <fstream>
#include <iostream>

#include "Console/Console.hpp"
#include "Generator/IRGenerator.hpp"
#include "PrettyPrint/ASTPrinter.hpp"
#include "PrettyPrint/IRPrinter.hpp"
#include "PrettyPrint/PrettyPrintError.hpp"
#include "Translator/LLVMTranslator.hpp"
#include "Utils/KeyExistsInMap.hpp"

#include "version.hpp"

static std::map<std::string, translator::Platform> platformMap = {
    { "-linux-x86_64", translator::LINUX_x86_64 },
    { "-linux-arm64", translator::LINUX_ARM64 },
    { "-macos-x86_64", translator::MACOS_x86_64 },
    { "-macos-arm64", translator::MACOS_ARM64 },
    { "-wasm32", translator::WASM32 },
    { "-wasm64", translator::WASM64 },
    { "-linux-x86_64-dyn", translator::LINUX_x86_64_DYN },
    { "-linux-arm64-dyn", translator::LINUX_ARM64_DYN },
    { "-macos-x86_64-dyn", translator::MACOS_x86_64_DYN },
    { "-macos-arm64-dyn", translator::MACOS_ARM64_DYN },
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
    console.pushModule(moduleName);

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
    {
        pp::printModule(modulesMap[moduleName], false);
        return 0;
    }

    if (option == "-icode")
    {
        pp::printModuleIcodeOnly(modulesMap[moduleName], false);
        return 0;
    }

    if (option == "-ir-all")
    {
        pp::printModulesMap(modulesMap, false);
        return 0;
    }

    if (option == "-icode-all")
    {
        pp::printModulesMapIcodeOnly(modulesMap, false);
        return 0;
    }

    if (option == "-json-ir")
    {
        pp::printModule(modulesMap[moduleName], true);
        return 0;
    }

    if (option == "-json-icode")
    {
        pp::printModuleIcodeOnly(modulesMap[moduleName], true);
        return 0;
    }

    if (option == "-json-ir-all")
    {
        pp::printModulesMap(modulesMap, true);
        return 0;
    }

    if (option == "-json-icode-all")
    {
        pp::printModulesMapIcodeOnly(modulesMap, true);
        return 0;
    }

    if (option == "-llvm")
    {
        pp::println(translator::generateLLVMModuleString(modulesMap[moduleName], modulesMap, false, console));
        return 0;
    }

    if (option == "-llvm-release")
    {
        pp::println(translator::generateLLVMModuleString(modulesMap[moduleName], modulesMap, true, console));
        return 0;
    }

    if (option == "-c")
    {
        for (auto stringModulePair : modulesMap)
            translator::generateObject(stringModulePair.second, modulesMap, translator::DEFAULT, false, console);

        return 0;
    }

    if (option == "-release")
    {
        for (auto stringModulePair : modulesMap)
            translator::generateObject(stringModulePair.second, modulesMap, translator::DEFAULT, true, console);

        return 0;
    }

    if (isValidPlatformString(option))
    {
        for (auto stringModulePair : modulesMap)
            translator::generateObject(stringModulePair.second,
                                       modulesMap,
                                       getPlatformFromString(option),
                                       true,
                                       console);

        return 0;
    }

    printCLIUsage();
    return EXIT_FAILURE;
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
