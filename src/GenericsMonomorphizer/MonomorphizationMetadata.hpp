#ifndef GENERICS_MONOMORPHIZER_MONOMORPHIZER_CONTEXT
#define GENERICS_MONOMORPHIZER_MONOMORPHIZER_CONTEXT

#include <map>

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/ModuleDescription.hpp"
#include "../Node/Node.hpp"

class MonomorphizationMetadata
{
    std::vector<std::string, std::vector<std::string>> genericUses;
    std::map<std::string, std::string> aliases;
    std::vector<std::string> genericStructs;
};

class MonomorphizationContext
{   
    std::map<std::string, MonomorphizationMetadata>& monomorphizationMetadataMap;
    std::map<std::string, Node> nodeMap;
    const std::string& fileName;
    Console& console;
};

/* Fill metadata for the file name */
void getGenericUses(MonomorphizationContext& ctx, const Node& root);

/* Copy and insert nodeMap(s) with generated module name, metadata should have zero generic structs */
void instantiateGenerics(MonomorphizationContext& ctx, std::vector<std::string> types, const Node& root);

/* Should have zero generic uses, expands generic uses and generic froms, replace types with monomorphized types */
void monomorphizeAST(MonomorphizationContext& ctx, const Node& root);

#endif
