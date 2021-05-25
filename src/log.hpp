#ifndef LOG_HPP
#define LOG_HPP

#include <fstream>
#include <iostream>
#include <string>

#include "IntermediateRepresentation/All.hpp"
#include "Node/Node.hpp"
#include "Token/Token.hpp"

namespace miklog
{
    struct compile_error
    {
    };

    struct internal_bug_error
    {
    };

    void printToken(const token::Token& symbol);

    void printNode(const node::Node& node, int depth = 1);

    void println(const std::string& msg);

    void errorOnLine(const std::string& error_msg, const std::string& line, int lineno, int col);

    void error(const std::string& mod_name, const std::string& error_msg);

    void errorOnToken(const std::string& mod_name,
                   const std::string& error_msg,
                   std::ifstream& file,
                   const token::Token& tok);

    void parserError(const std::string& mod_name, token::tokenType expected, token::Token& found, std::ifstream& file);

    void parserErrorMultiple(const std::string& mod_name,
                          const token::tokenType* expected,
                          int ntoks,
                          const token::Token& found,
                          std::ifstream& file);

    void typeError(const std::string& mod_name,
                    std::ifstream& file,
                    const token::Token& tok,
                    icode::VariableDescription& expected,
                    icode::VariableDescription& found);

    void internalCompilerErrorToken(const std::string& mod_name, std::ifstream& file, const token::Token& tok);

    void internalCompileError(const std::string& mod_name);

    void print_operand(const icode::Operand& op);

    void print_entry(const icode::Entry& entry);

    void print_var_info(const icode::VariableDescription& var_info);

    void print_struct_desc(const icode::StructDescription& struct_desc, int ilvl = 0);

    void print_def(const icode::Define& definition);

    void print_func_desc(const icode::FunctionDescription& func_desc, int ilvl = 0);

    void print_module_desc(const icode::ModuleDescription& module_desc, int ilvl = 0);
}

#endif