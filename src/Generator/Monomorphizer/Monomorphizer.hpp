#ifndef GENERATOR_MONOMORPHIZER_MONOMORPHIZER
#define GENERATOR_MONOMORPHIZER_MONOMORPHIZER

#include <map>
#include <string>
#include <vector>

#include "../../Console/Console.hpp"
#include "../../IntermediateRepresentation/TypeDescription.hpp"
#include "../../Node/Node.hpp"
#include "GenericASTIndex.hpp"
#include "MonomorphNameMangle.hpp"

namespace monomorphizer
{
    typedef std::map<std::string, GenericASTIndex> StringGenericASTMap;

    class Monomorphizer
    {
        StringGenericASTMap& genericsMap;
        std::map<std::string, std::string> aliases;
        std::vector<std::string> uses;
        std::map<std::string, std::string> genericUses;
        std::string workingModule;

        Console& console;

        std::vector<std::string> getGenericStructs(const Node& root,
                                                   const std::vector<std::string>& genericIdentifiers);

        void createFrom(const std::string& genericModuleName, const Token& symbolToken);

        std::string getGenericModuleNameFromAlias(const Token& aliasToken);

        std::string getGenericModuleNameFromUse(const Token& nameToken);

    public:
        void resetWorkingModule();

        Monomorphizer(StringGenericASTMap& genericsMap, Console& console);

        void indexAST(const std::string& genericModuleName, const Node& ast);

        void createUse(const Token& pathToken, const Token& aliasToken);

        bool useExists(const Token& pathToken);

        bool aliasExists(const Token& aliasToken);

        void createDirectFrom(const Token& pathToken, const Token& symbolToken);

        void createAliasFrom(const Token& aliasToken, const Token& symbolToken);

        void setWorkingModuleFromAlias(const Token& aliasToken);

        std::string getGenericModuleFromToken(const Token& token);

        Node instantiateGeneric(const std::string& genericModuleName,
                                const std::string& instantiationSuffix,
                                const Token& genericStructNameToken,
                                const std::vector<icode::TypeDescription>& instantiationTypes,
                                const std::vector<Node>& instantiationTypeNodes);
    };
};

#endif