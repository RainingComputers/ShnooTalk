#ifndef IRGEN_HPP
#define IRGEN_HPP

#include <fstream>
#include <map>
#include <numeric>
#include <string>

#include "ibuild.hpp"
#include "icode.hpp"
#include "log.hpp"
#include "node.hpp"
#include "pathchk.hpp"
#include "token.hpp"

namespace irgen
{
    typedef std::pair<icode::operand, icode::var_info> op_var_pair;

    class ir_generator
    {
        icode::target_desc& target;
        icode::module_desc_map& ext_modules_map;
        icode::module_desc& module;
        std::ifstream& file;

        ibuild::ir_builder builder;

        icode::func_desc* current_func_desc;
        icode::module_desc* current_ext_module;

        unsigned int id_counter;
        unsigned int scope_id_counter;
        std::vector<unsigned int> scope_id_stack;

        unsigned int id();

        unsigned int get_scope_id();
        void enter_scope();
        void exit_scope();
        void clear_scope();
        bool in_scope(unsigned int scope_id);

        bool get_def(const std::string& name, icode::def& def);

        bool get_func(const std::string& name, icode::func_desc& func);

        bool get_enum(const std::string& name, int& val);

        std::pair<token::token, icode::var_info> var_from_node(const node::node& root);

        void use(const node::node& root);

        void from(const node::node& root);

        void enumeration(const node::node& root);

        void def(const node::node& root);

        void structure(const node::node& root);

        void fn(const node::node& root);

        void global_var(const node::node& root);

        std::vector<int> str_literal_toint(const token::token& str_token);

        op_var_pair gen_str_dat(const token::token& str_token, icode::var_info var);

        void assign_str_literal_tovar(op_var_pair var, node::node& root);

        void assign_init_list_tovar(op_var_pair var, node::node& root);

        void copy_array(icode::operand& left, op_var_pair right);

        void copy_struct(icode::operand& left, op_var_pair right);

        void var(const node::node& root);

        op_var_pair var_access(const node::node& root);

        op_var_pair funccall(const node::node& root);

        op_var_pair size_of(const node::node& root);

        op_var_pair term(const node::node& root);

        op_var_pair expression(const node::node& root);

        void assignment(const node::node& root);

        icode::operand gen_label(token::token tok, bool true_label, std::string prefix = "");

        void condn_expression(const node::node& root,
                              const icode::operand& t_label,
                              const icode::operand& f_label,
                              bool t_fall,
                              bool f_fall);

        void ifstmt(const node::node& root,
                    bool loop,
                    const icode::operand& start_label,
                    const icode::operand& break_label,
                    const icode::operand& cont_label);

        void whileloop(const node::node& root);

        void forloop(const node::node& root);

        void print(const node::node& root);

        void input(const node::node& root);

        void block(const node::node& root,
                   bool loop,
                   const icode::operand& start_label,
                   const icode::operand& break_label,
                   const icode::operand& cont_label);

      public:
        void initgen(const node::node& ast);

        void program(const node::node& root);

        ir_generator(icode::target_desc& target_desc,
                     icode::module_desc_map& modules_map,
                     const std::string& file_name,
                     std::ifstream& ifile);
    };
}

#endif