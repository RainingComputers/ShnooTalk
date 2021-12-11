#include <fstream>
#include <iostream>

#include "Console/Console.hpp"
#include "Generator/IRGenerator.hpp"
#include "PrettyPrint/ASTPrinter.hpp"
#include "PrettyPrint/IRPrinter.hpp"
#include "PrettyPrint/PrettyPrintError.hpp"
#include "Token/Token.hpp"
#include "Translator/LLVMTranslator.hpp"

#include "version.hpp"

void printCLIUsage()
{
    pp::println("USAGE: shtkc FILE OPTION");
    pp::println("");
    pp::println("Available options:");
    pp::println("    -c               Compile program");
    pp::println("    -ast             Print parse tree");
    pp::println("    -ir              Print ShnooTalk IR");
    pp::println("    -icode           Print ShnooTalk IR, but only the icode");
    pp::println("    -llvm            Print LLVM IR");
    pp::println("    -json-ast        Print parse tree in JSON");
    pp::println("    -json-ir         Print ShnooTalk IR in JSON");
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
    icode::TargetEnums target = translator::getTarget();
    generator::generateIR(console, moduleName, target, modulesMap);

    if (option == "-ir")
    {
        pp::printModuleDescription(modulesMap[moduleName], false);
        return 0;
    }

    if (option == "-icode")
    {
        pp::printModuleDescriptionIcodeOnly(modulesMap[moduleName], false);
        return 0;
    }

    if (option == "-json-ir")
    {
        pp::printModuleDescription(modulesMap[moduleName], true);
        return 0;
    }

    if (option == "-json-icode")
    {
        pp::printModuleDescriptionIcodeOnly(modulesMap[moduleName], true);
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
            translator::generateLLVMModuleObject(stringModulePair.second, modulesMap, console);

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
    catch (...)
    {
        pp::println("Unknown error or an internal compiler error,");
        pp::println("REPORT THIS BUG");
        return EXIT_FAILURE;
    }

    return 0;
}
