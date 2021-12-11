#ifndef GENERATOR_GENERATOR_CONTEXT
#define GENERATOR_GENERATOR_CONTEXT

#include "../Builder/IRBuilder.hpp"
#include "ScopeTracker.hpp"

namespace generator
{
    struct GeneratorContext
    {
        IRBuilder ir;
        ScopeTracker scope;
        Console& console;

        icode::StringModulesMap& modulesMap;
        icode::TargetEnums& target;

        GeneratorContext(icode::TargetEnums& target,
                         icode::StringModulesMap& modulesMap,
                         const std::string& fileName,
                         Console& console);
    };
}

#endif