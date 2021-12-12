#include "GeneratorContext.hpp"

namespace generator
{
    GeneratorContext::GeneratorContext(icode::TargetEnums& target,
                                       icode::StringModulesMap& modulesMap,
                                       monomorphizer::StringGenericASTMap& genericsMap,
                                       const std::string& fileName,
                                       Console& console)

        : ir(target, modulesMap, fileName, console)
        , mm(genericsMap, console)
        , console(console)
        , modulesMap(modulesMap)
        , genericsMap(genericsMap)
        , target(target)
    {
    }
}
