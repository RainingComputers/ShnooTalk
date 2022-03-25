#include "../Builder/NameMangle.hpp"

#include "Generic.hpp"

using namespace icode;

void generateModule(generator::GeneratorContext& ctx, const Node& root);

std::string typeDescriptionToTypeString(const icode::TypeDescription& typeDescription)
{
    std::string typeDescString = typeDescription.dtypeName;

    if (typeDescription.isStruct())
        typeDescString = typeDescription.moduleName + "@" + typeDescString;

    if (typeDescription.isPointer())
    {
        if (typeDescription.isArray())
            return typeDescString + "[]";
        else
            return typeDescString + "~";
    }

    for (const int dim : typeDescription.dimensions)
        typeDescString += "[" + std::to_string(dim) + "]";

    return typeDescString;
}

std::string constructInstantiationSuffix(const std::vector<icode::TypeDescription>& types)
{
    std::string result = "";

    for (const icode::TypeDescription& type : types)
        result += "[" + typeDescriptionToTypeString(type) + "]";

    return result;
}

std::string generateIRUsingMonomorphizer(generator::GeneratorContext& ctx,
                                         const std::string& genericModuleName,
                                         const Token& genericNameToken,
                                         const std::vector<TypeDescription>& instantiationTypes,
                                         const std::vector<Node>& instantiationTypeNodes)
{
    const std::string& instantiationSuffix = constructInstantiationSuffix(instantiationTypes);

    const std::string& instantiatedModuleName = genericModuleName + instantiationSuffix;

    if (!ctx.moduleExists(instantiatedModuleName))
    {
        ctx.console.pushModule(genericModuleName);

        Node ast =
            ctx.mm.instantiateGeneric(genericModuleName, genericNameToken, instantiationTypes, instantiationTypeNodes);

        generator::GeneratorContext generatorContext = ctx.clone(instantiatedModuleName);
        generateModule(generatorContext, ast);

        ctx.console.popModule();
    }

    return instantiatedModuleName;
}

TypeDescription instantiateGenericAndGetType(generator::GeneratorContext& ctx,
                                             const std::string& genericModuleName,
                                             const Token& genericStructNameToken,
                                             const std::vector<TypeDescription>& instantiationTypes,
                                             const std::vector<Node>& instantiationTypeNodes)
{
    ctx.ir.pushWorkingModule();

    const std::string& moduleName = generateIRUsingMonomorphizer(ctx,
                                                                 genericModuleName,
                                                                 genericStructNameToken,
                                                                 instantiationTypes,
                                                                 instantiationTypeNodes);

    ctx.ir.setWorkingModule(&ctx.modulesMap.at(moduleName));

    TypeDescription monomorphizedType = ctx.ir.moduleBuilder.createTypeDescription(genericStructNameToken);

    ctx.ir.popWorkingModule();

    return monomorphizedType;
}

FunctionDescription intantiateGenericAndGetFunction(generator::GeneratorContext& ctx,
                                                    const std::string& genericModuleName,
                                                    const Token& genericFunctionNameToken,
                                                    const std::vector<TypeDescription>& instantiationTypes,
                                                    const std::vector<Node>& instantiationTypeNodes)
{
    ctx.ir.pushWorkingModule();

    const std::string& moduleName = generateIRUsingMonomorphizer(ctx,
                                                                 genericModuleName,
                                                                 genericFunctionNameToken,
                                                                 instantiationTypes,
                                                                 instantiationTypeNodes);

    ctx.ir.setWorkingModule(&ctx.modulesMap.at(moduleName));

    FunctionDescription monomorphizedFunction = ctx.ir.descriptionFinder.getFunction(genericFunctionNameToken);

    ctx.ir.popWorkingModule();

    return monomorphizedFunction;
}