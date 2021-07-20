#ifndef GENERATOR_GENERATOR_CONTEXT
#define GENERATOR_GENERATOR_CONTEXT

#include "../Builder/IRBuilder.hpp"
#include "ScopeTracker.hpp"

namespace generator
{
    struct GeneratorContext
    {
        ShnooTalkIRBuilder ir;
        ScopeTracker scope;
        Console& console;

        GeneratorContext(icode::TargetEnums& target,
                         icode::StringModulesMap& modules_map,
                         const std::string& file_name,
                         Console& console);
    };
}

#endif