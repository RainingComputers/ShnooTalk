#ifndef IRGEN_HPP
#define IRGEN_HPP

#include "Builder/ModuleBuilder.hpp"
#include "Builder/DescriptionFinder.hpp"
#include "Builder/EntryBuilder.hpp"
#include "Builder/Unit.hpp"
#include "Builder/UnitBuilder.hpp"
#include "Builder/FunctionBuilder.hpp"
#include "Builder/StringBuilder.hpp"
#include "Console/Console.hpp"
#include "IRGenerator/ScopeTracker.hpp"
#include "IntermediateRepresentation/All.hpp"
#include "Node/Node.hpp"
#include "Token/Token.hpp"
#include "pathchk.hpp"

typedef std::pair<Token, icode::TypeDescription> TokenTypePair;
typedef std::pair<std::vector<int>, int> LiteralDimensionsIndexPair;

namespace irgen
{
    class ir_generator
    {
      public:
        icode::TargetDescription& target;
        icode::StringModulesMap& modulesMap;
        icode::ModuleDescription& rootModule;
        Console& console;

        ModuleBuilder moduleBuilder;
        OperandBuilder opBuilder;
        UnitBuilder unitBuilder;
        DescriptionFinder descriptionFinder;
        EntryBuilder builder;
        FunctionBuilder functionBuilder;
        StringBuilder strBuilder;
        

        icode::FunctionDescription* workingFunction;
        icode::ModuleDescription* workingModule;

        ScopeTracker scope;

        std::vector<icode::ModuleDescription*> moduleDescriptionStack;

        void resetWorkingModule();
        void setWorkingModule(icode::ModuleDescription* moduleDescription);
        void pushWorkingModule();
        void popWorkingModule();

        bool get_func(const std::string& name, icode::FunctionDescription& func);

        std::pair<Token, icode::TypeDescription> var_from_node(const Node& root);

        void assign_str_literal_tovar(Unit var, Node& root);

        void assign_init_list_tovar(Unit var, Node& root);

        void copy_array(icode::Operand& left, Unit right);

        void copy_struct(icode::Operand& left, Unit right);

        void var(const Node& root);

        icode::Instruction assignmentTokenToBinaryOperator(const Token tok);

        void assignment(const Node& root);

        void print(const Node& root);

        void input(const Node& root);

        void block(const Node& root,
                   bool loop,
                   const icode::Operand& start_label,
                   const icode::Operand& break_label,
                   const icode::Operand& cont_label);

        void initgen(const Node& ast);

        void program(const Node& root);

        ir_generator(icode::TargetDescription& target_desc,
                     icode::StringModulesMap& modules_map,
                     const std::string& file_name,
                     Console& console);
    };
}

#endif