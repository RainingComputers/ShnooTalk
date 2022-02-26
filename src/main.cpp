#include <fstream>
#include <iostream>

#include "Console/Console.hpp"
#include "Generator/IRGenerator.hpp"
#include "PrettyPrint/ASTPrinter.hpp"
#include "PrettyPrint/IRPrinter.hpp"
#include "PrettyPrint/PrettyPrintError.hpp"
#include "Translator/LLVMTranslator.hpp"

#include "version.hpp"

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
    pp::println("    -json-ast        Print parse tree in JSON");
    pp::println("    -json-ir         Print ShnooTalk IR in JSON");
    pp::println("    -json-ir-all     Print ShnooTalk IR recursively for all modules in json");
    pp::println("    -json-icode      Print ShnooTalk IR in JSON, but only the icode");
    pp::println("");
    pp::println("Use shtkc -version for compiler version");
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

    if (option == "-llvm")
    {
        pp::println(translator::generateLLVMModuleString(modulesMap[moduleName], modulesMap, console));
        return 0;
    }

    if (option == "-c")
    {
        for (auto stringModulePair : modulesMap)
            translator::generateLLVMModuleObject(stringModulePair.second, modulesMap, false, console);

        return 0;
    }

    if (option == "-release")
    {
        for (auto stringModulePair : modulesMap)
            translator::generateLLVMModuleObject(stringModulePair.second, modulesMap, true, console);

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
