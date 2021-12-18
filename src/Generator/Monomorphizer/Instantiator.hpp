#ifndef GENERATOR_MONOMORPHIZER_INSTANTIATOR
#define GENERATOR_MONOMORPHIZER_INSTANTIATOR

#include <string>
#include <vector>

#include "../../IntermediateRepresentation/TypeDescription.hpp"
#include "../../Node/Node.hpp"
#include "../../Console/Console.hpp"
#include "GenericASTIndex.hpp"

Node instantiateAST(GenericASTIndex index,
                    const Token& typeRootToken,
                    const std::vector<icode::TypeDescription>& instantiationTypes,
                    const std::vector<Node>& instTypeNodes,
                    const std::string& instantiationSuffix,
                    Console& console);

#endif