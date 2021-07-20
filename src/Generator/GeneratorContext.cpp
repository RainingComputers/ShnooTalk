#include "../Builder/NameMangle.hpp"

#include "GeneratorContext.hpp"

namespace generator
{
    GeneratorContext::GeneratorContext(icode::TargetEnums& target,
                                       icode::StringModulesMap& modulesMap,
                                       const std::string& fileName,
                                       Console& console)

      : ir(target, modulesMap, fileName, console)
      , console(console)
    {
    }
}
