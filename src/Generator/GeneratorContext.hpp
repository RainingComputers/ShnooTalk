#ifndef GENERATOR_GENERATOR_CONTEXT
#define GENERATOR_GENERATOR_CONTEXT

#include "../Builder/IRBuilder.hpp"
#include "Monomorphizer/Monomorphizer.hpp"
#include "ScopeTracker.hpp"

namespace generator
{
    struct GeneratorContext
    {
        IRBuilder ir;
        monomorphizer::Monomorphizer mm;
        ScopeTracker scope;
        Console& console;

        icode::StringModulesMap& modulesMap;
        monomorphizer::StringGenericASTMap& genericsMap;
        icode::TargetEnums& target;

        GeneratorContext(icode::TargetEnums& target,
                         icode::StringModulesMap& modulesMap,
                         monomorphizer::StringGenericASTMap& genericsMap,
                         const std::string& fileName,
                         Console& console);
    };
}

#endif