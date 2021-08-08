#include <fstream>
#include <iostream>

#include "Console/Console.hpp"
#include "Generator/IRGenerator.hpp"
#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include "PrettyPrint/ASTPrinter.hpp"
#include "PrettyPrint/IRPrinter.hpp"
#include "Token/Token.hpp"
#include "Translator/LLVMTranslator.hpp"

void printCLIUsage()
{
    pp::println("USAGE: uhllc FILE OPTION");
    pp::println("");
    pp::println("Available options:");
    pp::println("    -c         Compile program");
    pp::println("    -ast       Print parse tree");
    pp::println("    -json-ast  Print parse tree in JSON");
    pp::println("    -ir        intermediate code representation");
    pp::println("    -json-ir   Print intermediate code representation completely in JSON");
    pp::println("    -llvm      Print llvm ir");
}

Console getStreamAndConsole(const std::string& fileName, std::ifstream& fileStream)
{
    fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fileStream.open(fileName);

    Console console(fileName, &fileStream);

    return console;
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
            std::ifstream fileStream;
            std::string useWithExt = use;
            Console console = getStreamAndConsole(useWithExt, fileStream);
            generateIR(console, use, target, modulesMap);
        }

    generator::generateModule(generatorContext, ast);
}

int phaseDriver(const std::string& moduleName, const std::string& option)
{
    std::ifstream fileStream;
    Console console = getStreamAndConsole(moduleName, fileStream);

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
    if (argc != 3)
    {
        printCLIUsage();
        return EXIT_FAILURE;
    }

    try
    {
        std::string fileName = argv[1];
        std::string option = argv[2];
        return phaseDriver(fileName, option);
    }
    catch (const CompileError)
    {
        return EXIT_FAILURE;
    }
    catch (const InternalBugError)
    {
        return EXIT_FAILURE;
    }
    catch (const std::ifstream::failure&)
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
