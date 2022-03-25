#include <algorithm>

#include "../../Builder/NameMangle.hpp"
#include "../../TemplateUtils/ItemInList.hpp"
#include "GenericASTIndex.hpp"
#include "Instantiator.hpp"

struct InstiatorContext
{
    std::string& genericIdentifier;
    Token& typeRootToken;
    Node& instTypeNode;
    icode::TypeDescription& instantiationType;
    Console& console;
};

void block(const InstiatorContext& ctx, Node& root);

Token modToken(const Token& tok, const std::string& tokenString)
{
    return Token(tok.getFileName(), tokenString, tok.getType());
}

Node constructNode(const node::NodeType nodeType, const std::string& tokenString)
{
    Node node;
    node.type = nodeType;
    node.tok = Token("", tokenString, token::GENERATED);

    return node;
}

void validateTypeNode(const InstiatorContext& ctx, Node& root)
{
    size_t childNodeCounter = 1;
    int typeModifierCount = 0;

    while (root.isNthChild(node::PARAM, childNodeCounter) || root.isNthChild(node::MUT_PARAM, childNodeCounter))
        childNodeCounter++;

    while (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter++;

    if (root.isNthChild(node::IDENTIFIER, childNodeCounter))
        childNodeCounter++;

    while (root.isNthChild(node::POINTER_STAR, childNodeCounter))
    {
        typeModifierCount++;
        childNodeCounter++;
    }

    while (root.isNthChild(node::EMPTY_SUBSCRIPT, childNodeCounter))
    {
        typeModifierCount++;
        childNodeCounter++;
    }

    if (root.isNthChild(node::SUBSCRIPT, childNodeCounter))
        typeModifierCount++;

    if (typeModifierCount > 1)
        ctx.console.compileErrorOnToken("Invalid type parameters for this generic", ctx.typeRootToken);
}

void monomorphizeTypeNode(const InstiatorContext& ctx, Node& root)
{
    size_t childNodeCounter = 1;
    if (root.type == node::GENERIC_TYPE_PARAM || root.type == node::MAKE)
        childNodeCounter = 0;

    while (root.isNthChild(node::PARAM, childNodeCounter) || root.isNthChild(node::MUT_PARAM, childNodeCounter))
        childNodeCounter++;

    while (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter++;

    if (childNodeCounter >= root.children.size())
        return;

    if (root.getNthChildToken(childNodeCounter).toString() == ctx.genericIdentifier)
    {
        root.children.erase(root.children.begin() + childNodeCounter);

        root.children.insert(root.children.begin() + childNodeCounter,
                             ctx.instTypeNode.children.begin(),
                             ctx.instTypeNode.children.end());

        validateTypeNode(ctx, root);

        return;
    }

    childNodeCounter++;

    while (root.isNthChild(node::GENERIC_TYPE_PARAM, childNodeCounter))
    {
        monomorphizeTypeNode(ctx, root.children[childNodeCounter]);
        validateTypeNode(ctx, root.children[childNodeCounter]);

        childNodeCounter++;
    }
}

void sizeOf(const InstiatorContext& ctx, Node& root)
{
    if (!root.isNthChild(node::IDENTIFIER, 0))
        return;

    const Token& typeToken = root.getNthChildToken(0);

    if (typeToken.toString() != ctx.genericIdentifier)
        return;

    root.children[0].tok = modToken(typeToken, ctx.instantiationType.dtypeName);

    const std::string& alias = mangleModuleName(ctx.instantiationType.moduleName);
    root.children.insert(root.children.begin(), constructNode(node::MODULE, alias));
}

void pointerCast(const InstiatorContext& ctx, Node& root)
{
    const Token& typeToken = root.getNthChildToken(0);

    if (typeToken.toString() != ctx.genericIdentifier)
        return;

    root.children[0].tok = modToken(typeToken, ctx.instantiationType.dtypeName);

    Node newRootNode = constructNode(node::TERM, typeToken.toString());

    const std::string& alias = mangleModuleName(ctx.instantiationType.moduleName);
    newRootNode.children.push_back(constructNode(node::MODULE, alias));
    newRootNode.children.push_back(root);

    root = newRootNode;
}

void make(const InstiatorContext& ctx, Node& root)
{
    monomorphizeTypeNode(ctx, root.children[0]);
}

void expression(const InstiatorContext& ctx, Node& root)
{
    if (root.type == node::SIZEOF)
    {
        sizeOf(ctx, root);
        return;
    }

    if (root.type == node::MAKE)
    {
        make(ctx, root);
        return;
    }

    // TODO: monomorphize type in generic function call

    if (root.isNthChild(node::PTR_CAST, 0) || root.isNthChild(node::PTR_ARRAY_CAST, 0))
        pointerCast(ctx, root);

    for (Node& child : root.children)
        expression(ctx, child);
}

void forLoop(const InstiatorContext& ctx, Node& root)
{
    if (root.isNthChild(node::VAR, 0))
        monomorphizeTypeNode(ctx, root.children[0]);

    block(ctx, root.children[3]);
}

void ifStatement(const InstiatorContext& ctx, Node& root)
{
    for (Node& child : root.children)
        block(ctx, child);
}

void statement(const InstiatorContext& ctx, Node& root)
{
    switch (root.type)
    {
        case node::VAR:
        case node::CONST:
            monomorphizeTypeNode(ctx, root);
            break;
        case node::FOR:
            forLoop(ctx, root);
            break;
        case node::IF:
            ifStatement(ctx, root);
            break;
        case node::WHILE:
            block(ctx, root.children[1]);
            break;
        case node::DO_WHILE:
            block(ctx, root.children[0]);
            break;
        case node::LOOP:
            block(ctx, root.children[0]);
            break;
        default:
            break;
    }

    expression(ctx, root);
}

void block(const InstiatorContext& ctx, Node& root)
{
    for (Node& stmt : root.children)
        statement(ctx, stmt);
}

void function(const InstiatorContext& ctx, Node& root)
{
    monomorphizeTypeNode(ctx, root);

    for (size_t i = 1; root.children[i].isParamNode(); i += 1)
        monomorphizeTypeNode(ctx, root.children[i]);

    block(ctx, root.children.back());
}

void structure(const InstiatorContext& ctx, Node& root)
{
    for (Node& child : root.children[0].children)
        monomorphizeTypeNode(ctx, child);
}

void monomorphizeTypeNodes(const InstiatorContext& ctx, Node& root)
{
    switch (root.type)
    {
        case node::STRUCT:
            structure(ctx, root);
            break;
        case node::FUNCTION:
        case node::EXTERN_FUNCTION:
            function(ctx, root);
            break;
        case node::VAR:
            monomorphizeTypeNode(ctx, root);
            break;
        default:
            break;
    }
}

void instantiateASTSingle(InstiatorContext& ctx, Node& genericModuleAST)
{
    for (Node& child : genericModuleAST.children)
        monomorphizeTypeNodes(ctx, child);
}

void prependUseNode(Node& root, const std::string& moduleName, const std::string& alias)
{
    Node useNode = constructNode(node::USE, "use");
    useNode.children.push_back(constructNode(node::STR_LITERAL, '\"' + moduleName + '\"'));
    useNode.children.push_back(constructNode(node::IDENTIFIER, alias));

    root.children.insert(root.children.begin(), useNode);
}

void stripModulesFromTypeNode(Node& root)
{
    int numModuleNodes = 0;

    while (root.isNthChild(node::MODULE, numModuleNodes))
        numModuleNodes++;

    root.children.erase(root.children.begin(), root.children.begin() + numModuleNodes);
}

void prependModuleToTypeNode(Node& root, const std::string& alias)
{
    root.children.insert(root.children.begin(), constructNode(node::MODULE, alias));
}

void prependUseNodes(const std::vector<icode::TypeDescription>& instantiationTypes,
                     std::vector<Node>& instTypeNodes,
                     Node& genericModuleAST)
{
    std::vector<std::string> prependedModules;

    for (size_t i = 0; i < instantiationTypes.size(); i += 1)
    {
        const icode::TypeDescription& type = instantiationTypes[i];
        Node& typeNode = instTypeNodes[i];

        const std::string& moduleName = type.moduleName;
        const std::string& alias = mangleModuleName(moduleName);

        stripModulesFromTypeNode(typeNode);
        prependModuleToTypeNode(typeNode, alias);

        if (itemInList<std::string>(moduleName, prependedModules))
            continue;

        prependedModules.push_back(moduleName);

        prependUseNode(genericModuleAST, moduleName, alias);
    }
}

Node instantiateAST(GenericASTIndex index,
                    Token typeRootToken,
                    std::vector<icode::TypeDescription> instantiationTypes,
                    std::vector<Node> instTypeNodes,
                    Console& console)
{
    Node& genericModuleAST = index.ast;

    prependUseNodes(instantiationTypes, instTypeNodes, genericModuleAST);

    for (size_t i = 0; i < index.genericIdentifiers.size(); i += 1)
    {
        InstiatorContext ctx = InstiatorContext{ index.genericIdentifiers[i],
                                                 typeRootToken,
                                                 instTypeNodes[i],
                                                 instantiationTypes[i],
                                                 console };

        instantiateASTSingle(ctx, genericModuleAST);
    }

    return genericModuleAST;
}
