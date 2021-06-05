#include <fstream>
#include <iostream>

#include "Console/Console.hpp"
#include "LLVMTranslator/LLVMTranslator.hpp"
#include "PrettyPrint/IRPrinter.hpp"
#include "Token/Token.hpp"
#include "irgen_old.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "pathchk.hpp"

void printCLIUsage()
{
    mikpp::println("USAGE: uhllc FILE OPTION");
    mikpp::println("\nAvailable options:");
    mikpp::println("\t-c\tCompile program");
    mikpp::println("\t-ast\tPrint parse tree");
    mikpp::println("\t-ir\tPrint intermediate code representation");
    mikpp::println("\t-llvm\tPrint llvm ir");
}

std::string removeFileExtension(const std::string& fileName)
{
    return fileName.substr(0, fileName.find_last_of("."));
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
    lexer::lexical_analyser lex(*console.getStream(), console);
    parser::rd_parser parse(lex, console);

    return parse.ast;
}

void generateIR(Console& console,
                const std::string& moduleName,
                icode::TargetDescription& target,
                icode::StringModulesMap& modulesMap)
{

    Node ast = generateAST(console);

    irgen::ir_generator gen(target, modulesMap, moduleName, console);

    gen.initgen(ast);

    for (std::string use : modulesMap[moduleName].uses)
        if (modulesMap.find(use) == modulesMap.end())
        {
            std::ifstream fileStream;
            std::string useWithExt = use + ".uhll";
            Console console = getStreamAndConsole(useWithExt, fileStream);
            generateIR(console, use, target, modulesMap);
        }

    /* Generate icode */
    gen.program(ast);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printCLIUsage();
        return EXIT_FAILURE;
    }

    std::string fileName = argv[1];
    std::string moduleName = removeFileExtension(fileName);
    std::string option = argv[2];

    std::ifstream fileStream;
    Console console = getStreamAndConsole(fileName, fileStream);

    /* Compile program */
    try
    {
        if (option == "-ast")
        {
            mikpp::printNode(generateAST(console));
            return 0;
        }

        icode::StringModulesMap modulesMap;
        icode::TargetDescription target = llvmgen::getTargetDescription();
        generateIR(console, moduleName, target, modulesMap);

        if (option == "-ir")
        {
            mikpp::printModuleDescription(modulesMap[moduleName]);
            return 0;
        }

        if (option == "-llvm")
        {
            mikpp::println(llvmgen::generateLLVMModuleString(modulesMap[moduleName], modulesMap, console));
            return 0;
        }

        if (option == "-c")
        {
            for (auto stringModulePair : modulesMap)
                llvmgen::generateLLVMModuleObject(stringModulePair.second, modulesMap, console);

            return 0;
        }

        printCLIUsage();
        return EXIT_FAILURE;
    }
    catch (const CompileError)
    {
        return EXIT_FAILURE;
    }
    catch (const InternalBugError)
    {
        return EXIT_FAILURE;
    }
    catch (const std::ifstream::failure)
    {
        mikpp::println("File I/O error");
        return EXIT_FAILURE;
    }

    return 0;
}