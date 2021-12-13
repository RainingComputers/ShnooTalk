#ifndef GENERATOR_MONOMORPHIZER_NAME_MANGLE
#define GENERATOR_MONOMORPHIZER_NAME_MANGLE

#include <string>

#include "../../IntermediateRepresentation/TypeDescription.hpp"

std::string constructInstantiationSuffix(const std::vector<icode::TypeDescription>& instantiationTypes);

#endif