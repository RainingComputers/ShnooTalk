#ifndef GENERATOR_GENERIC
#define GENERATOR_GENERIC

#include "GeneratorContext.hpp"

std::pair<std::string, std::string> generateIRUsingMonomorphizer(
    generator::GeneratorContext& ctx,
    const std::string& genericModuleName,
    const Token& genericNameToken,
    const std::vector<icode::TypeDescription>& instantiationTypes,
    const std::vector<Node>& instantiationTypeNodes);

#endif