#ifndef IRGEN_HPP
#define IRGEN_HPP

#include "Builder/DescriptionBuilder.hpp"
#include "Builder/UnitBuilder.hpp"
#include "Builder/DescriptionFinder.hpp"
#include "Builder/Unit.hpp"
#include "Console/Console.hpp"
#include "IRBuilder_old/IRBuilder.hpp"
#include "IRGenerator/ScopeTracker.hpp"
#include "IntermediateRepresentation/All.hpp"
#include "Node/Node.hpp"
#include "Token/Token.hpp"
#include "pathchk.hpp"


typedef std::pair<token::Token, icode::TypeDescription> TokenTypePair;
typedef std::pair<std::vector<int>, int> LiteralDimensionsIndexPair;

namespace irgen
{
    class ir_generator
    {
      public:
        icode::TargetDescription& target;
        icode::StringModulesMap& ext_modules_map;
        icode::ModuleDescription& rootModule;
        Console& console;

        OperandBuilder opBuilder;
        ibuild::IRBuilder builder;
        UnitBuilder unitBuilder;
        DescriptionFinder descriptionFinder;

        icode::FunctionDescription* workingFunction;
        icode::ModuleDescription* workingModule;

        DescriptionBuilder descriptionBuilder;

        ScopeTracker scope;

        void resetWorkingModule();
        void setWorkingModule(icode::ModuleDescription* moduleDescription);

        bool get_func(const std::string& name, icode::FunctionDescription& func);

        std::pair<token::Token, icode::TypeDescription> var_from_node(const node::Node& root);

        void use(const node::Node& root);

        void from(const node::Node& root);

        icode::Operand gen_str_dat(const token::Token& str_token, size_t char_count, icode::DataType dtype);

        Unit var_info_to_str_dat(const token::Token& str_token, icode::TypeDescription var);

        void assign_str_literal_tovar(Unit var, node::Node& root);

        void assign_init_list_tovar(Unit var, node::Node& root);

        void copy_array(icode::Operand& left, Unit right);

        void copy_struct(icode::Operand& left, Unit right);

        void var(const node::Node& root);

        Unit getTypeFromToken(const node::Node& root);

        Unit var_access(const node::Node& root);

        Unit funccall(const node::Node& root);

        Unit size_of(const node::Node& root);

        Unit term(const node::Node& root);

        icode::Instruction tokenToBinaryOperator(const token::Token tok);

        Unit expression(const node::Node& root);

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
                     Console& console);
    };
}

#endif