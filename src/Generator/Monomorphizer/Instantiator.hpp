#ifndef GENERATOR_MONOMORPHIZER_INSTANTIATOR
#define GENERATOR_MONOMORPHIZER_INSTANTIATOR

#include <string>
#include <vector>

#include "../../IntermediateRepresentation/TypeDescription.hpp"
#include "../../Node/Node.hpp"

void instantiateAST(Node& genericModuleAST,
                    const std::vector<std::string> genericIdentifiers,
                    std::vector<icode::TypeDescription> instantiationTypes,
                    std::vector<Node> typeDescriptionNodes,
                    const std::string& instantiationSuffix);

#endif