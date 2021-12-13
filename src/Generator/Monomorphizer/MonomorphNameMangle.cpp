#include "../../Builder/NameMangle.hpp"

#include "MonomorphNameMangle.hpp"

std::string dimensionToString(const std::vector<int>& dimensions)
{
    std::string result = "";

    for (const int dim : dimensions)
        result += dim;

    return result;
}

std::string instantiationStringFromType(const icode::TypeDescription& typeDescription)
{
    std::string result = mangleModuleName(typeDescription.moduleName) + typeDescription.dtypeName;

    if (typeDescription.isArray())
    {
        result += "_array";
        result += dimensionToString(typeDescription.dimensions);
    }    

    if (typeDescription.isPointer())
        result += "_ptr";

    return result;
}

std::string constructInstantiationSuffix(const std::vector<icode::TypeDescription>& instantiationTypes) 
{
    std::string result = "";

    for (const icode::TypeDescription& type : instantiationTypes)
        result += "_" + instantiationStringFromType(type);

    return result;
}