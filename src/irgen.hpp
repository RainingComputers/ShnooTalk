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
    typedef std::pair<icode::Operand, icode::VariableDescription> OperandDescriptionPair;

    class ir_generator
    {
        icode::TargetDescription& target;
        icode::StringModulesMap& ext_modules_map;
        icode::ModuleDescription& module;
        std::ifstream& file;

        ibuild::ir_builder builder;

        icode::FunctionDescription* current_func_desc;
        icode::ModuleDescription* current_ext_module;

        unsigned int id_counter;
        unsigned int scope_id_counter;
        std::vector<unsigned int> scope_id_stack;

        unsigned int id();

        unsigned int get_scope_id();
        void enter_scope();
        void exit_scope();
        void clear_scope();
        bool in_scope(unsigned int scope_id);

        bool get_def(const std::string& name, icode::Define& def);

        bool get_func(const std::string& name, icode::FunctionDescription& func);

        bool get_enum(const std::string& name, int& val);

        std::pair<token::token, icode::VariableDescription> var_from_node(const node::node& root);

        void use(const node::node& root);

        void from(const node::node& root);

        void enumeration(const node::node& root);

        void def(const node::node& root);

        void structure(const node::node& root);

        void fn(const node::node& root);

        void global_var(const node::node& root);

        icode::Operand gen_str_dat(const token::token& str_token, size_t char_count, icode::DataType dtype);

        OperandDescriptionPair var_info_to_str_dat(const token::token& str_token, icode::VariableDescription var);

        void assign_str_literal_tovar(OperandDescriptionPair var, node::node& root);

        void assign_init_list_tovar(OperandDescriptionPair var, node::node& root);

        void copy_array(icode::Operand& left, OperandDescriptionPair right);

        void copy_struct(icode::Operand& left, OperandDescriptionPair right);

        void var(const node::node& root);

        OperandDescriptionPair var_access(const node::node& root);

        OperandDescriptionPair funccall(const node::node& root);

        OperandDescriptionPair size_of(const node::node& root);

        OperandDescriptionPair term(const node::node& root);

        OperandDescriptionPair expression(const node::node& root);

        void assignment(const node::node& root);

        icode::Operand gen_label(token::token tok, bool true_label, std::string prefix = "");

        void condn_expression(const node::node& root,
                              const icode::Operand& t_label,
                              const icode::Operand& f_label,
                              bool t_fall,
                              bool f_fall);

        void ifstmt(const node::node& root,
                    bool loop,
                    const icode::Operand& start_label,
                    const icode::Operand& break_label,
                    const icode::Operand& cont_label);

        void whileloop(const node::node& root);

        void forloop(const node::node& root);

        void print(const node::node& root);

        void input(const node::node& root);

        void block(const node::node& root,
                   bool loop,
                   const icode::Operand& start_label,
                   const icode::Operand& break_label,
                   const icode::Operand& cont_label);

      public:
        void initgen(const node::node& ast);

        bool current_function_terminates();

        void program(const node::node& root);

        ir_generator(icode::TargetDescription& target_desc,
                     icode::StringModulesMap& modules_map,
                     const std::string& file_name,
                     std::ifstream& ifile);
    };
}

#endif