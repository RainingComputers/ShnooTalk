#include <fstream>
#include <iostream>

#include "LLVMTranslator/LLVMTranslator.hpp"
#include "irgen.hpp"
#include "lexer.hpp"
#include "log.hpp"
#include "parser.hpp"
#include "pathchk.hpp"
#include "token.hpp"

void print_usage()
{
    miklog::println("USAGE: uhllc MODULE [OPTION]");
    miklog::println("\nAvailable options:");
    miklog::println("\t-ast\tPrint parse tree");
    miklog::println("\t-ir\tPrint intermediate code representation");
    miklog::println("\t-llvm\tPrint llvm ir");
    miklog::println("\t-c\tCompile program (default)");
}

void ir_gen(const std::string& file_name, icode::TargetDescription& target, icode::StringModulesMap& modules)
{
    /* Open file */
    std::ifstream ifile;
    ifile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    ifile.open(file_name + ".uhll");

    lexer::lexical_analyser lex(file_name, ifile);
    parser::rd_parser parse(lex, file_name, ifile);
    irgen::ir_generator gen(target, modules, file_name, ifile);

    /* Intermediate code generation */
    gen.initgen(parse.ast);

    /* Compile external modules that the module uses/imports */
    for (std::string use : modules[file_name].uses)
        if (modules.find(use) == modules.end())
            ir_gen(use, target, modules);

    /* Generate icode */
    gen.program(parse.ast);
}

std::string strip_file_ext(const std::string& file_name)
{
    /* Get module name, if .uhll is present, strip it */

    std::string stripped_file_name;
    std::string ext = ".uhll";

    if (file_name.size() > ext.size() && file_name.substr(file_name.size() - ext.size()) == ext)
        stripped_file_name = file_name.substr(0, file_name.size() - ext.size());
    else
        stripped_file_name = file_name;

    return stripped_file_name;
}

int main(int argc, char* argv[])
{
    /* Check for correct usage */
    if (argc < 2 || argc > 3)
    {
        print_usage();
        return EXIT_FAILURE;
    }

    /* Get module name */
    std::string file_name = strip_file_ext(argv[1]);

    /* Get option passed by user, (if present) */
    std::string option;
    if (argc == 3)
    {
        option = argv[2];

        if (option != "-ir" && option != "-llvm" && option != "-c" && option != "-ast")
        {
            print_usage();
            return EXIT_FAILURE;
        }
    }

    /* Map for holding all the compiled module in intermediate representation */
    icode::StringModulesMap modules;

    /* Compile program */
    try
    {
        if (option == "-ast")
        {
            /* Open file */
            std::ifstream ifile;
            ifile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            ifile.open(file_name + ".uhll");

            lexer::lexical_analyser lex(file_name, ifile);
            parser::rd_parser parse(lex, file_name, ifile);

            miklog::print_node(parse.ast);

            return 0;
        }

        icode::TargetDescription target = llvmgen::getTargetDescription();

        ir_gen(file_name, target, modules);

        if (option == "-ir")
        {
            for (auto pair : modules)
            {
                miklog::print_module_desc(pair.second);
                miklog::println("");
            }

            return 0;
        }

        if (option == "-llvm")
        {
            std::string llvm_module = llvmgen::generateLLVMModule(modules[file_name], modules, false);
            miklog::println(llvm_module);
            return 0;
        }

        for (auto pair : modules)
            llvmgen::generateLLVMModule(pair.second, modules, true);
    }
    catch (const miklog::compile_error& e)
    {
        return EXIT_FAILURE;
    }
    catch (const miklog::internal_bug_error& e)
    {
        return EXIT_FAILURE;
    }

    return 0;
}