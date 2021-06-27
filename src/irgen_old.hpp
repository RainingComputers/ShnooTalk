#ifndef DEPRICATED_IRGEN
#define DEPRICATED_IRGEN

#include "Builder/DescriptionFinder.hpp"
#include "Builder/FunctionBuilder.hpp"
#include "Builder/ModuleBuilder.hpp"
#include "Builder/StringBuilder.hpp"
#include "Builder/Unit.hpp"
#include "Builder/UnitBuilder.hpp"
#include "Console/Console.hpp"
#include "Generator/ScopeTracker.hpp"
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

        void setWorkingFunction(icode::FunctionDescription* functionDescription);

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