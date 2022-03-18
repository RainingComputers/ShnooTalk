#include "Generic.hpp"

using namespace icode;

void generateModule(generator::GeneratorContext& ctx, const Node& root);

std::pair<std::string, std::string> generateIRUsingMonomorphizer(generator::GeneratorContext& ctx,
                                                                 const std::string& genericModuleName,
                                                                 const Token& genericNameToken,
                                                                 const std::vector<TypeDescription>& instantiationTypes,
                                                                 const std::vector<Node>& instantiationTypeNodes)
{
    const std::string& instantiationSuffix = constructInstantiationSuffix(instantiationTypes);

    const std::string& instantiatedModuleName = getInstantiatedModuleName(instantiationSuffix, genericModuleName);

    if (!ctx.moduleExists(instantiatedModuleName))
    {
        ctx.console.pushModule(genericModuleName);

        Node ast = ctx.mm.instantiateGeneric(genericModuleName,
                                             instantiationSuffix,
                                             genericNameToken,
                                             instantiationTypes,
                                             instantiationTypeNodes);

        generator::GeneratorContext generatorContext = ctx.clone(instantiatedModuleName);
        generateModule(generatorContext, ast);

        ctx.console.popModule();
    }

    std::string instantiatedStructName = getInstantiatedStructName(instantiationSuffix, genericNameToken);

    return std::pair<std::string, std::string>(instantiatedModuleName, instantiatedStructName);
}
