#include <fstream>
#include <iostream>

#include "Console/Console.hpp"
#include "Generator/IRGenerator.hpp"
#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
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
    pp::println("    -c         Compile program");
    pp::println("    -ast       Print parse tree");
    pp::println("    -json-ast  Print parse tree in JSON");
    pp::println("    -ir        intermediate code representation");
    pp::println("    -json-ir   Print intermediate code representation completely in JSON");
    pp::println("    -llvm      Print llvm ir");
    pp::println("");
    pp::println("Use shtkc -version for compiler version");
}

Node generateAST(Console& console)
{
    lexer::Lexer lex(*console.getStream(), console);
    return parser::generateAST(lex, console);
}

void generateIR(Console& console,
                const std::string& moduleName,
                icode::TargetEnums& target,
                icode::StringModulesMap& modulesMap)
{

    Node ast = generateAST(console);

    generator::GeneratorContext generatorContext(target, modulesMap, moduleName, console);

    generator::getUses(generatorContext, ast);

    for (std::string use : modulesMap[moduleName].uses)
        if (modulesMap.find(use) == modulesMap.end())
        {
            console.pushModule(use);
            generateIR(console, use, target, modulesMap);
            console.popModule();
        }

    generator::generateModule(generatorContext, ast);
}

int phaseDriver(const std::string& moduleName, const std::string& option, Console& console)
{
    console.pushModule(moduleName);

    if (option == "-ast")
    {
        pp::printNode(generateAST(console));
        return 0;
    }

    if (option == "-json-ast")
    {
        pp::printJSONAST(generateAST(console));
        return 0;
    }

    icode::StringModulesMap modulesMap;
    icode::TargetEnums target = translator::getTarget();
    generateIR(console, moduleName, target, modulesMap);

    if (option == "-ir")
    {
        pp::printModuleDescription(modulesMap[moduleName], false);
        return 0;
    }

    if (option == "-json-ir")
    {
        pp::printModuleDescription(modulesMap[moduleName], true);
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
