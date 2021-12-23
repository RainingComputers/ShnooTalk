#ifndef GENERATOR_MONOMORPHIZER_INSTANTIATOR
#define GENERATOR_MONOMORPHIZER_INSTANTIATOR

#include <string>
#include <vector>

#include "../../Console/Console.hpp"
#include "../../IntermediateRepresentation/TypeDescription.hpp"
#include "../../Node/Node.hpp"
#include "GenericASTIndex.hpp"

Node instantiateAST(GenericASTIndex index,
                    Token typeRootToken,
                    std::vector<icode::TypeDescription> instantiationTypes,
                    std::vector<Node> instTypeNodes,
                    std::string instantiationSuffix,
                    Console& console);

#endif