#ifndef IRGEN_HPP
#define IRGEN_HPP

#include <fstream>
#include <map>
#include <numeric>
#include <string>

#include "IRBuilder/IRBuilder.hpp"
#include "IntermediateRepresentation/All.hpp"
#include "Node/Node.hpp"
#include "log.hpp"
#include "pathchk.hpp"
#include "Token/Token.hpp"

typedef std::pair<icode::Operand, icode::VariableDescription> OperandDescriptionPair;
typedef std::pair<token::Token, icode::VariableDescription> TokenDescriptionPair;
typedef std::pair<icode::ModuleDescription*, size_t> ModuleIndexPair;
typedef std::pair<icode::DataType, int> DataTypeSizePair;
typedef std::pair<std::vector<int>, int> LiteralDimensionsIndexPair;

namespace irgen
{
    class ir_generator
    {
      public:
        icode::TargetDescription& target;
        icode::StringModulesMap& ext_modules_map;
        icode::ModuleDescription& module;
        std::ifstream& file;

        ibuild::IRBuilder builder;

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
        void resetCurrentExternalModule();

        bool get_def(const std::string& name, icode::Define& def);

        bool get_func(const std::string& name, icode::FunctionDescription& func);

        bool get_enum(const std::string& name, int& val);

        std::pair<token::Token, icode::VariableDescription> var_from_node(const node::Node& root);

        void use(const node::Node& root);

        void from(const node::Node& root);

        void enumeration(const node::Node& root);

        void def(const node::Node& root);

        void structure(const node::Node& root);

        void fn(const node::Node& root);

        void global_var(const node::Node& root);

        icode::Operand gen_str_dat(const token::Token& str_token, size_t char_count, icode::DataType dtype);

        OperandDescriptionPair var_info_to_str_dat(const token::Token& str_token, icode::VariableDescription var);

        void assign_str_literal_tovar(OperandDescriptionPair var, node::Node& root);

        void assign_init_list_tovar(OperandDescriptionPair var, node::Node& root);

        void copy_array(icode::Operand& left, OperandDescriptionPair right);

        void copy_struct(icode::Operand& left, OperandDescriptionPair right);

        void var(const node::Node& root);

        OperandDescriptionPair var_access(const node::Node& root);

        OperandDescriptionPair funccall(const node::Node& root);

        OperandDescriptionPair size_of(const node::Node& root);

        OperandDescriptionPair term(const node::Node& root);

        icode::Instruction tokenToBinaryOperator(const token::Token tok);

        OperandDescriptionPair expression(const node::Node& root);

        icode::Instruction assignmentTokenToBinaryOperator(const token::Token tok);

        void assignment(const node::Node& root);

        icode::Operand gen_label(token::Token tok, bool true_label, std::string prefix = "");

        icode::Instruction tokenToCompareOperator(const token::Token tok);

        void condn_expression(const node::Node& root,
                              const icode::Operand& t_label,
                              const icode::Operand& f_label,
                              bool t_fall,
                              bool f_fall);

        void ifstmt(const node::Node& root,
                    bool loop,
                    const icode::Operand& start_label,
                    const icode::Operand& break_label,
                    const icode::Operand& cont_label);

        void whileloop(const node::Node& root);

        void forloop(const node::Node& root);

        void print(const node::Node& root);

        void input(const node::Node& root);

        void block(const node::Node& root,
                   bool loop,
                   const icode::Operand& start_label,
                   const icode::Operand& break_label,
                   const icode::Operand& cont_label);

        void initgen(const node::Node& ast);

        bool current_function_terminates();

        void program(const node::Node& root);

        ir_generator(icode::TargetDescription& target_desc,
                     icode::StringModulesMap& modules_map,
                     const std::string& file_name,
                     std::ifstream& ifile);
    };
}

#endif