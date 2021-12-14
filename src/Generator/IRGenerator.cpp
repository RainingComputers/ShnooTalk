#include "../Builder/TypeDescriptionUtil.hpp"
#include "Assignment.hpp"
#include "Define.hpp"
#include "Enum.hpp"
#include "Expression.hpp"
#include "Input.hpp"
#include "Module.hpp"
#include "Print.hpp"
#include "Return.hpp"

#include "../Lexer/Lexer.hpp"
#include "../Parser/Parser.hpp"

#include "IRGenerator.hpp"

// TODO: DELETE THIS
#include "../PrettyPrint/ASTPrinter.hpp"

using namespace icode;

void block(generator::GeneratorContext& ctx,
           const Node& root,
           bool isLoopBlock,
           const Operand& loopLabel,
           const Operand& breakLabel,
           const Operand& continueLabel);

void generateModule(generator::GeneratorContext& ctx, const Node& root);

TypeDescription typeDescriptionFromNode(generator::GeneratorContext& ctx, const Node& root);

bool isGenericModule(const Node& ast)
{
    return ast.children[0].type == node::GENERIC;
}

bool generateIRFromName(generator::GeneratorContext& ctx, const std::string& moduleName)
{
    if (ctx.modulesMap.find(moduleName) != ctx.modulesMap.end())
        return false;

    ctx.console.pushModule(moduleName);

    Node ast = generator::generateAST(ctx.console);

    const bool isGeneric = isGenericModule(ast);

    if (isGeneric)
        ctx.mm.indexAST(moduleName, ast);
    else
    {
        generator::GeneratorContext generatorContext = ctx.clone(moduleName);
        generateModule(generatorContext, ast);
    }

    ctx.console.popModule();

    return isGeneric;
}

void createUse(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& moduleNameToken = root.getNthChildToken(0);
    const Token& aliastoken = root.getNthChildToken(1);

    ctx.ir.moduleBuilder.createUse(moduleNameToken, aliastoken);

    bool isGeneric = generateIRFromName(ctx, moduleNameToken.toUnescapedString());

    if (isGeneric)
        ctx.mm.createUse(moduleNameToken, aliastoken);
}

void createFrom(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& moduleNameToken = root.children[0].tok;

    for (Node child : root.children[1].children)
        ctx.ir.moduleBuilder.createFrom(moduleNameToken, child.tok);
}

void generateIRFromInstatiatedAST(generator::GeneratorContext& ctx, const std::string& moduleName, const Node& ast)
{
    ctx.console.pushModule(moduleName);

    generator::GeneratorContext generatorContext = ctx.clone(moduleName);
    generateModule(generatorContext, ast);

    ctx.console.popModule();
}

TypeDescription getMonomorphizedTypeDescriptionFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    size_t childNodeCounter = 1;

    std::string genericModuleName = "";

    while (root.isNthChild(node::MODULE, childNodeCounter))
    {
        const Token& aliasToken = root.getNthChildToken(childNodeCounter);

        if (childNodeCounter > 1)
            ctx.console.compileErrorOnToken("Invalid MODULE ACCESS from GENERIC", aliasToken);

        genericModuleName = ctx.mm.getGenericModuleNameFromAlias(aliasToken);

        childNodeCounter++;
    }

    const Token& genericStructNameToken = root.getNthChildToken(childNodeCounter);
    childNodeCounter++;

    if (genericModuleName == "")
        genericModuleName = ctx.mm.getGenericModuleNameFromStruct(genericStructNameToken);

    std::vector<TypeDescription> instantiationTypes;
    std::vector<Node> instantiationTypeNodes;

    while (root.isNthChild(node::GENERIC, childNodeCounter))
    {
        instantiationTypes.push_back(typeDescriptionFromNode(ctx, root.children[childNodeCounter]));
        instantiationTypeNodes.push_back(root.children[childNodeCounter]);

        childNodeCounter++;
    }

    pp::printNode(ctx.mm.instantiateGeneric(genericModuleName, root.tok, instantiationTypes, instantiationTypeNodes));

    return ctx.ir.moduleBuilder.createVoidTypeDescription();
}

TypeDescription arrayTypeFromSubscript(const Node& root, const TypeDescription& typeDescription, size_t startIndex)
{
    std::vector<int> dimensions;

    size_t nodeCounter;

    for (nodeCounter = startIndex; root.isNthChild(node::SUBSCRIPT, nodeCounter); nodeCounter++)
    {
        const int subscriptInt = root.children[nodeCounter].children[0].tok.toInt();
        dimensions.push_back(subscriptInt);
    }

    return createArrayTypeDescription(typeDescription, dimensions, FIXED_DIM);
}

TypeDescription typeDescriptionFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    if (root.isNthChildFromLast(node::GENERIC, 1))
        return getMonomorphizedTypeDescriptionFromNode(ctx, root);

    size_t childNodeCounter = 1;

    while (root.isNthChild(node::PARAM, childNodeCounter) || root.isNthChild(node::MUT_PARAM, childNodeCounter))
        childNodeCounter++;

    if (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter = setWorkingModuleFromNode(ctx, root, childNodeCounter);

    if (!root.isNthChild(node::IDENTIFIER, childNodeCounter))
        return ctx.ir.moduleBuilder.createVoidTypeDescription();

    const Token& dataTypeToken = root.getNthChildToken(childNodeCounter);
    TypeDescription typeDescription = ctx.ir.moduleBuilder.createTypeDescription(dataTypeToken);

    childNodeCounter++;

    if (root.isNthChild(node::POINTER_STAR, childNodeCounter))
        typeDescription.becomePointer();

    if (root.isNthChild(node::EMPTY_SUBSCRIPT, childNodeCounter))
        typeDescription.becomeArrayPointer();

    if (root.isNthChild(node::SUBSCRIPT, childNodeCounter))
        typeDescription = arrayTypeFromSubscript(root, typeDescription, childNodeCounter);

    ctx.ir.resetWorkingModule();

    return typeDescription;
}

TypeDescription getParamType(generator::GeneratorContext& ctx, const Node& paramNode)
{
    TypeDescription paramType = typeDescriptionFromNode(ctx, paramNode);

    if (paramNode.type == node::MUT_PARAM)
        paramType.becomeMutable();

    return paramType;
}

bool isParamNode(const Node& nodeToCheck)
{
    return nodeToCheck.type == node::PARAM || nodeToCheck.type == node::MUT_PARAM;
}

void createStructFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);

    std::vector<Token> fieldNames;
    std::vector<icode::TypeDescription> fieldTypes;

    for (size_t i = 0; i < root.children[0].children.size(); i++)
    {
        const Token& fieldName = root.children[0].children[i].getNthChildToken(0);

        TypeDescription fieldType = typeDescriptionFromNode(ctx, root.children[0].children[i]);

        fieldNames.push_back(fieldName);
        fieldTypes.push_back(fieldType);
    }

    ctx.ir.moduleBuilder.createStruct(nameToken, fieldNames, fieldTypes);
}

void createFunctionFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);
    TypeDescription returnType = typeDescriptionFromNode(ctx, root);

    std::vector<Token> paramNames;
    std::vector<icode::TypeDescription> paramTypes;

    for (size_t i = 1; isParamNode(root.children[i]); i += 1)
    {
        const Token& paramName = root.children[i].getNthChildToken(0);
        TypeDescription paramType = getParamType(ctx, root.children[i]);

        paramNames.push_back(paramName);
        paramTypes.push_back(paramType);

        ctx.scope.putInCurrentScope(paramName);
    }

    if (root.type == node::FUNCTION)
        ctx.ir.moduleBuilder.createFunction(nameToken, returnType, paramNames, paramTypes);
    else
        ctx.ir.moduleBuilder.createExternFunction(nameToken, returnType, paramNames, paramTypes);
}

void createGlobalFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& globalName = root.getNthChildToken(0);

    TypeDescription globalType = typeDescriptionFromNode(ctx, root);

    globalType.becomeMutable();

    ctx.ir.moduleBuilder.createGlobal(globalName, globalType);
}

void generateSymbol(generator::GeneratorContext& ctx, const Node& child)
{
    ctx.scope.resetScope();

    switch (child.type)
    {
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

void local(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);

    icode::TypeDescription localType = typeDescriptionFromNode(ctx, root);

    ctx.scope.putInCurrentScope(nameToken);

    if (root.type == node::VAR)
        localType.becomeMutable();

    Unit local = ctx.ir.functionBuilder.createLocal(nameToken, localType);

    Node lastNode = root.children.back();

    if (lastNode.type == node::EXPRESSION || lastNode.type == node::TERM ||
        lastNode.type == node::MULTILINE_STR_LITERAL || lastNode.type == node::INITLIST)
    {
        Unit RHS = expression(ctx, lastNode);
        assignmentFromTree(ctx, root, nameToken, lastNode.tok, local, RHS);
    }
}

void ifStatement(generator::GeneratorContext& ctx,
                 const Node& root,
                 bool isLoopBlock,
                 const Operand& loopLabel,
                 const Operand& breakLabel,
                 const Operand& continueLabel)
{
    Operand elseIfChainEndLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "ifend");

    for (size_t i = 0; i < root.children.size(); i++)
    {
        Node child = root.children[i];

        Operand ifTrueLabel = ctx.ir.functionBuilder.createLabel(child.tok, true, "if");
        Operand ifFalseLabel = ctx.ir.functionBuilder.createLabel(child.tok, false, "if");

        if (child.type != node::ELSE)
        {
            conditionalExpression(ctx, child.children[0], ifTrueLabel, ifFalseLabel, true);

            block(ctx, child.children[1], isLoopBlock, loopLabel, breakLabel, continueLabel);

            if (i != root.children.size() - 1)
                ctx.ir.functionBuilder.createBranch(GOTO, elseIfChainEndLabel);

            ctx.ir.functionBuilder.insertLabel(ifFalseLabel);
        }
        else
        {
            block(ctx, child.children[0], isLoopBlock, loopLabel, breakLabel, continueLabel);
        }
    }

    if (root.children.size() != 1)
        ctx.ir.functionBuilder.insertLabel(elseIfChainEndLabel);
}

void whileLoop(generator::GeneratorContext& ctx, const Node& root)
{
    Operand loopLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "while");
    Operand breakLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "while");

    ctx.ir.functionBuilder.insertLabel(loopLabel);

    conditionalExpression(ctx, root.children[0], loopLabel, breakLabel, true);

    block(ctx, root.children[1], true, loopLabel, breakLabel, loopLabel);

    ctx.ir.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.ir.functionBuilder.insertLabel(breakLabel);
}

void doWhileLoop(generator::GeneratorContext& ctx, const Node& root)
{
    Operand loopLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "do");
    Operand breakLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "do");

    ctx.ir.functionBuilder.insertLabel(loopLabel);

    block(ctx, root.children[0], true, loopLabel, breakLabel, loopLabel);

    conditionalExpression(ctx, root.children[1], loopLabel, breakLabel, true);

    ctx.ir.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.ir.functionBuilder.insertLabel(breakLabel);
}

void forLoopInitOrUpdateNode(generator::GeneratorContext& ctx, const Node& root)
{
    if (root.type == node::VAR)
        local(ctx, root);
    else if (root.type == node::TERM)
        term(ctx, root);
    else
        assignment(ctx, root);
}

void forLoop(generator::GeneratorContext& ctx, const Node& root)
{

    forLoopInitOrUpdateNode(ctx, root.children[0]);

    Operand loopLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "for");
    Operand breakLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "for");
    Operand continueLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "for_cont");

    ctx.ir.functionBuilder.insertLabel(loopLabel);

    conditionalExpression(ctx, root.children[1], loopLabel, breakLabel, true);

    block(ctx, root.children[3], true, loopLabel, breakLabel, continueLabel);

    ctx.ir.functionBuilder.insertLabel(continueLabel);

    forLoopInitOrUpdateNode(ctx, root.children[2]);

    ctx.ir.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.ir.functionBuilder.insertLabel(breakLabel);
}

void infniteLoop(generator::GeneratorContext& ctx, const Node& root)
{
    Operand loopLabel = ctx.ir.functionBuilder.createLabel(root.tok, true, "loop");
    Operand breakLabel = ctx.ir.functionBuilder.createLabel(root.tok, false, "loop");

    ctx.ir.functionBuilder.insertLabel(loopLabel);

    block(ctx, root.children[0], true, loopLabel, breakLabel, loopLabel);

    ctx.ir.functionBuilder.createBranch(GOTO, loopLabel);

    ctx.ir.functionBuilder.insertLabel(breakLabel);
}

void continueStatement(generator::GeneratorContext& ctx,
                       bool isLoopBlock,
                       const Operand& continueLabel,
                       const Token& token)
{
    if (!isLoopBlock)
        ctx.console.compileErrorOnToken("CONTINUE outside loop", token);

    ctx.ir.functionBuilder.createBranch(GOTO, continueLabel);
}

void breakStatement(generator::GeneratorContext& ctx, bool isLoopBlock, const Operand& breakLabel, const Token& token)
{
    if (!isLoopBlock)
        ctx.console.compileErrorOnToken("BREAK outside loop", token);

    ctx.ir.functionBuilder.createBranch(GOTO, breakLabel);
}

void statement(generator::GeneratorContext& ctx,
               const Node& root,
               bool isLoopBlock,
               const Operand& loopLabel,
               const Operand& breakLabel,
               const Operand& continueLabel)
{
    switch (root.type)
    {
        case node::VAR:
        case node::CONST:
            local(ctx, root);
            break;
        case node::ASSIGNMENT:
            assignment(ctx, root);
            break;
        case node::FUNCCALL:
            functionCall(ctx, root);
            break;
        case node::TERM:
            functionCall(ctx, root.children[0]);
            break;
        case node::IF:
            ifStatement(ctx, root, isLoopBlock, loopLabel, breakLabel, continueLabel);
            break;
        case node::WHILE:
            whileLoop(ctx, root);
            break;
        case node::DO_WHILE:
            doWhileLoop(ctx, root);
            break;
        case node::FOR:
            forLoop(ctx, root);
            break;
        case node::LOOP:
            infniteLoop(ctx, root);
            break;
        case node::BREAK:
            breakStatement(ctx, isLoopBlock, breakLabel, root.tok);
            break;
        case node::CONTINUE:
            continueStatement(ctx, isLoopBlock, continueLabel, root.tok);
            break;
        case node::RETURN:
            functionReturn(ctx, root);
            break;
        case node::PRINT:
        case node::PRINTLN:
            print(ctx, root);
            break;
        case node::INPUT:
            input(ctx, root);
            break;
        case node::MODULE:
        {
            ctx.ir.pushWorkingModule();
            int nodeCounter = setWorkingModuleFromNode(ctx, root, 0);
            functionCall(ctx, root.children[nodeCounter]);
            ctx.ir.popWorkingModule();
            break;
        }
        default:
            ctx.console.internalBugErrorOnToken(root.tok);
    }
}

void block(generator::GeneratorContext& ctx,
           const Node& root,
           bool isLoopBlock,
           const Operand& loopLabel,
           const Operand& breakLabel,
           const Operand& continueLabel)
{
    ctx.scope.createScope();

    for (Node stmt : root.children)
        statement(ctx, stmt, isLoopBlock, loopLabel, breakLabel, continueLabel);

    ctx.scope.exitScope();
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
        generateSymbol(ctx, child);

    for (const Node& child : root.children)
        if (child.type == node::FUNCTION)
            generateFunction(ctx, child);
}

Node generator::generateAST(Console& console)
{
    lexer::Lexer lex(*console.getStream(), console);
    return parser::generateAST(lex, console);
}

void generator::generateIR(Console& console,
                           const std::string& moduleName,
                           icode::TargetEnums& target,
                           icode::StringModulesMap& modulesMap,
                           monomorphizer::StringGenericASTMap& genericsMap)
{

    Node ast = generateAST(console);

    if (isGenericModule(ast))
        console.compileErrorOnToken("Connot compile a GENERIC", ast.children[0].tok);

    generator::GeneratorContext generatorContext(target, modulesMap, genericsMap, moduleName, console);
    generateModule(generatorContext, ast);
}
