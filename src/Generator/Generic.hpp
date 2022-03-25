#ifndef GENERATOR_GENERIC
#define GENERATOR_GENERIC

#include "GeneratorContext.hpp"

std::string generateIRUsingMonomorphizer(generator::GeneratorContext& ctx,
                                         const std::string& genericModuleName,
                                         const Token& genericNameToken,
                                         const std::vector<icode::TypeDescription>& instantiationTypes,
                                         const std::vector<Node>& instantiationTypeNodes);

icode::TypeDescription instantiateGenericAndGetType(generator::GeneratorContext& ctx,
                                                    const std::string& genericModuleName,
                                                    const Token& genericStructNameToken,
                                                    const std::vector<icode::TypeDescription>& instantiationTypes,
                                                    const std::vector<Node>& instantiationTypeNodes);

icode::FunctionDescription intantiateGenericAndGetFunction(
    generator::GeneratorContext& ctx,
    const std::string& genericModuleName,
    const Token& genericFunctionNameToken,
    const std::vector<icode::TypeDescription>& instantiationTypes,
    const std::vector<Node>& instantiationTypeNodes);

#endif