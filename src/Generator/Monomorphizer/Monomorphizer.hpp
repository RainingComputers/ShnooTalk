#ifndef GENERATOR_MONOMORPHIZER_MONOMORPHIZER
#define GENERATOR_MONOMORPHIZER_MONOMORPHIZER

#include <map>
#include <string>
#include <vector>

#include "../../Console/Console.hpp"
#include "../../IntermediateRepresentation/TypeDescription.hpp"
#include "../../Node/Node.hpp"
#include "GenericASTIndex.hpp"

namespace monomorphizer
{
    typedef std::map<std::string, GenericASTIndex> StringGenericASTMap;

    class Monomorphizer
    {
        StringGenericASTMap& genericsMap;
        std::map<std::string, std::string> aliases;
        std::map<std::string, std::string> genericUses;

        Console& console;

        std::vector<std::string> getGenericStructs(const Node& root,
                                                   const std::vector<std::string>& genericIdentifiers);

    public:
        Monomorphizer(StringGenericASTMap& genericsMap, Console& console);

        void indexAST(const std::string& genericModuleName, const Node& ast);

        void createUse(const Token& genericModuleNameToken, const Token& aliasToken);

        std::string getGenericModuleNameFromAlias(const Token& aliasToken);

        std::string getGenericModuleNameFromStruct(const Token& nameToken);

        Node instantiateGeneric(const std::string& genericModuleName,
                                const Token& typeRootToken,
                                const std::vector<icode::TypeDescription>& instantiationTypes,
                                const std::vector<Node>& instantiationTypeNodes);
    };
};

#endif