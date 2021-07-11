#include <fstream>
#include <iostream>

#include "Console/Console.hpp"
#include "Generator/IRGenerator.hpp"
#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include "PrettyPrint/IRPrinter.hpp"
#include "Token/Token.hpp"
#include "Translator/LLVMTranslator.hpp"

void printCLIUsage()
{
    pp::println("USAGE: uhllc FILE OPTION");
    pp::println("\nAvailable options:");
    pp::println("\t-c\tCompile program");
    pp::println("\t-ast\tPrint parse tree");
    pp::println("\t-ir\tPrint intermediate code representation");
    pp::println("\t-llvm\tPrint llvm ir");
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
            std::string useWithExt = use + ".uhll";
            Console console = getStreamAndConsole(useWithExt, fileStream);
            generateIR(console, use, target, modulesMap);
        }

    generator::generateModule(generatorContext, ast);
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

    /* Compile program */
    try
    {
        std::ifstream fileStream;
        Console console = getStreamAndConsole(fileName, fileStream);

        if (option == "-ast")
        {
            pp::printNode(generateAST(console));
            return 0;
        }

        icode::StringModulesMap modulesMap;
        icode::TargetEnums target = translator::getTarget();
        generateIR(console, moduleName, target, modulesMap);

        if (option == "-ir")
        {
            pp::printModuleDescription(modulesMap[moduleName]);
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
        pp::println("File I/O error");
        return EXIT_FAILURE;
    }

    return 0;
}
