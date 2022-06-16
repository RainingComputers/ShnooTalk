#ifndef GENERATOR_MONOMORPHIZER_GENERIC_AST_INDEX
#define GENERATOR_MONOMORPHIZER_GENERIC_AST_INDEX

#include <string>
#include <vector>

#include "../../Node/Node.hpp"

struct GenericASTIndex
{
    Node ast;
    std::vector<std::string> genericIdentifiers;
    std::vector<std::string> genericStructs;
    std::vector<std::string> genericFunctions;

    bool isGenericStructOrFunction(const Token& name) const;
};

#endif
