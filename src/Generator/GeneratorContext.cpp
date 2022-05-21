#include "../Utils/KeyExistsInMap.hpp"

#include "GeneratorContext.hpp"

namespace generator
{
    GeneratorContext::GeneratorContext(icode::StringModulesMap& modulesMap,
                                       monomorphizer::StringGenericASTMap& genericsMap,
                                       const std::string& fileName,
                                       Console& console)

        : ir(modulesMap, fileName, console)
        , mm(genericsMap, console)
        , console(console)
        , modulesMap(modulesMap)
        , genericsMap(genericsMap)
    {
    }

    GeneratorContext GeneratorContext::clone(const std::string& fileName)
    {
        return GeneratorContext(modulesMap, genericsMap, fileName, console);
    }

    bool GeneratorContext::moduleExists(const std::string& name)
    {
        return keyExistsInMap(modulesMap, name);
    }

    bool GeneratorContext::genericModuleExists(const std::string& name)
    {
        return keyExistsInMap(genericsMap, name);
    }
}
