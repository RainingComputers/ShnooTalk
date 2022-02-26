#include "Define.hpp"
#include "Enum.hpp"
#include "Function.hpp"
#include "Global.hpp"
#include "Statement.hpp"
#include "Structure.hpp"
#include "Use.hpp"

#include "IRGenerator.hpp"

#include "../Lexer/Lexer.hpp"
#include "../Parser/Parser.hpp"

#include "IRGenerator.hpp"

using namespace icode;

Node generator::generateAST(Console& console)
{
    lexer::Lexer lex(*console.getStream(), console);
    return parser::generateAST(lex, console);
}

void generateIncompleteType(generator::GeneratorContext& ctx, const Node& child)
{
    ctx.scope.resetScope();
 
    switch (child.type)
    {
        case node::STRUCT:
            registerIncompleteTypeFromNode(ctx, child);
            break;
        default:
            break;
    }
}

void generateSymbol(generator::GeneratorContext& ctx, const Node& child)
{
    ctx.scope.resetScope();

    switch (child.type)
    {
        case node::GENERIC:
            break;
        case node::USE:
            createUse(ctx, child);
            break;
        case node::FROM:
            createFrom(ctx, child);
            break;
        case node::STRUCT:
            createStructFromNode(ctx, child);
            break;
        case node::FUNCTION:
        case node::EXTERN_FUNCTION:
            createFunctionFromNode(ctx, child);
            break;
        case node::ENUM:
            createEnumFromNode(ctx, child);
            break;
        case node::DEF:
            createDefineFromNode(ctx, child);
            break;
        case node::VAR:
            createGlobalFromNode(ctx, child);
            break;
        default:
            ctx.console.internalBugErrorOnToken(child.tok);
    }
}

void generateFunction(generator::GeneratorContext& ctx, const Node& child)
{
    const Token& functionNameToken = child.children[0].tok;

    ctx.ir.setWorkingFunction(functionNameToken);

    ctx.scope.resetScope();

    block(ctx,
          child.children.back(),
          false,
          ctx.ir.opBuilder.createLabelOperand(""),
          ctx.ir.opBuilder.createLabelOperand(""),
          ctx.ir.opBuilder.createLabelOperand(""));

    ctx.ir.functionBuilder.terminateFunction(functionNameToken);
}

void generateModule(generator::GeneratorContext& ctx, const Node& root)
{
    for (const Node& child : root.children)
        generateIncompleteType(ctx, child);

    for (const Node& child : root.children)
        generateSymbol(ctx, child);

    for (const Node& child : root.children)
        if (child.type == node::FUNCTION)
            generateFunction(ctx, child);
}

void generator::generateIR(Console& console,
                           const std::string& moduleName,
                           icode::TargetEnums& target,
                           icode::StringModulesMap& modulesMap,
                           monomorphizer::StringGenericASTMap& genericsMap)
{

    Node ast = generateAST(console);

    if (ast.isGenericModule())
        console.compileErrorOnToken("Connot compile a GENERIC", ast.children[0].tok);

    generator::GeneratorContext generatorContext(target, modulesMap, genericsMap, moduleName, console);
    generateModule(generatorContext, ast);
}
