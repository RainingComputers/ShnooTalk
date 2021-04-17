#ifndef LOG_HPP
#define LOG_HPP

#include <fstream>
#include <iostream>
#include <string>

#include "icode.hpp"
#include "node.hpp"
#include "token.hpp"

namespace miklog
{
    struct compile_error
    {
    };

    struct internal_bug_error
    {
    };

    void print_token(const token::token& symbol);

    void print_node(const node::node& node, int depth = 1);

    void println(const std::string& msg);

    void error_line(const std::string& error_msg, const std::string& line, int lineno, int col);

    void error(const std::string& mod_name, const std::string& error_msg);

    void error_tok(const std::string& mod_name,
                   const std::string& error_msg,
                   std::ifstream& file,
                   const token::token& tok);

    void parse_error(const std::string& mod_name, token::token_type expected, token::token& found, std::ifstream& file);

    void parse_error_mult(const std::string& mod_name,
                          const token::token_type* expected,
                          int ntoks,
                          const token::token& found,
                          std::ifstream& file);

    void type_error(const std::string& mod_name,
                    std::ifstream& file,
                    const token::token& tok,
                    icode::var_info& expected,
                    icode::var_info& found);

    void internal_error_tok(const std::string& mod_name, std::ifstream& file, const token::token& tok);

    void internal_error(const std::string& mod_name);

    void print_operand(const icode::operand& op);

    void print_entry(const icode::entry& entry);

    void print_var_info(const icode::var_info& var_info);

    void print_struct_desc(const icode::struct_desc& struct_desc, int ilvl = 0);

    void print_def(const icode::def& definition);

    void print_func_desc(const icode::func_desc& func_desc, int ilvl = 0);

    void print_module_desc(const icode::module_desc& module_desc, int ilvl = 0);
}

#endif