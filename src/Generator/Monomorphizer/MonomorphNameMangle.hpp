#ifndef GENERATOR_MONOMORPHIZER_NAME_MANGLE
#define GENERATOR_MONOMORPHIZER_NAME_MANGLE

#include <string>

#include "../../IntermediateRepresentation/TypeDescription.hpp"
#include "../../Token/Token.hpp"

std::string constructInstantiationSuffix(const std::vector<icode::TypeDescription>& types);
std::string getInstantiatedStructName(const std::string& instantiationSuffix, const Token& nameToken);
std::string getInstantiatedModuleName(const std::string& instantiationSuffix, const std::string& genericModuleName);

#endif