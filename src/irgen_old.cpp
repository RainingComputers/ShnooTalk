#include <algorithm>

#include "Builder/TypeDescriptionUtil.hpp"
#include "Generator/ConditionalExpression.hpp"
#include "Generator/ControlStatement.hpp"
#include "Generator/Define.hpp"
#include "Generator/Enum.hpp"
#include "Generator/Expression.hpp"
#include "Generator/From.hpp"
#include "Generator/Function.hpp"
#include "Generator/FunctionCall.hpp"
#include "Generator/Global.hpp"
#include "Generator/Input.hpp"
#include "Generator/Module.hpp"
#include "Generator/Print.hpp"
#include "Generator/Structure.hpp"
#include "Generator/TypeDescriptionFromNode.hpp"
#include "Generator/UnitFromIdentifier.hpp"
#include "irgen_old.hpp"

namespace irgen
{
    ir_generator::ir_generator(icode::TargetDescription& targetDescription,
                               icode::StringModulesMap& modulesMap,
                               const std::string& fileName,
                               Console& console)

      : target(targetDescription)
      , modulesMap(modulesMap)
      , rootModule(modulesMap[fileName])
      , console(console)
      , moduleBuilder(modulesMap, console)
      , unitBuilder(opBuilder)
      , descriptionFinder(modulesMap[fileName], modulesMap, unitBuilder, console)
      , functionBuilder(modulesMap, opBuilder, unitBuilder, console)
      , strBuilder(modulesMap[fileName], opBuilder)
    {
        workingFunction = nullptr;
        workingModule = &rootModule;

        rootModule.name = fileName;
        rootModule.initializeTargetInfo(targetDescription);

        setWorkingModule(workingModule);
    }

    void ir_generator::resetWorkingModule()
    {
        workingModule = &rootModule;
        moduleBuilder.setWorkingModule(&rootModule);
        descriptionFinder.setWorkingModule(&rootModule);
    }

    void ir_generator::setWorkingModule(icode::ModuleDescription* moduleDescription)
    {
        workingModule = moduleDescription;
        moduleBuilder.setWorkingModule(moduleDescription);
        descriptionFinder.setWorkingModule(moduleDescription);
    }

    void ir_generator::pushWorkingModule()
    {
        moduleDescriptionStack.push_back(workingModule);
    }

    void ir_generator::popWorkingModule()
    {
        setWorkingModule(moduleDescriptionStack.back());
        moduleDescriptionStack.pop_back();
    }

    void ir_generator::var(const Node& root)
    {
        const Token& nameToken = root.getNthChildToken(0);

        icode::TypeDescription localType = typeDescriptionFromNode(*this, root);

        scope.putInCurrentScope(nameToken);

        if (root.type == node::VAR)
            localType.becomeMutable();

        Unit local = functionBuilder.createLocal(nameToken, localType);

        Node lastNode = root.children.back();

        if (lastNode.isNodeType(node::EXPRESSION) || lastNode.isNodeType(node::TERM) ||
            lastNode.isNodeType(node::STR_LITERAL) || lastNode.isNodeType(node::INITLIST))
        {
            Unit RHS = expression(*this, lastNode);

            if (!icode::isSameType(local.type, RHS.type))
                console.typeError(lastNode.tok, local.type, RHS.type);

            functionBuilder.unitCopy(local, RHS);
        }
    }

    icode::Instruction ir_generator::assignmentTokenToBinaryOperator(const Token tok)
    {
        switch (tok.getType())
        {
            case token::EQUAL:
                return icode::EQUAL;
            case token::PLUS_EQUAL:
                return icode::ADD;
            case token::MINUS_EQUAL:
                return icode::SUB;
            case token::DIVIDE_EQUAL:
                return icode::DIV;
            case token::MULTIPLY_EQUAL:
                return icode::MUL;
            case token::OR_EQUAL:
                return icode::BWO;
            case token::AND_EQUAL:
                return icode::BWA;
            case token::XOR_EQUAL:
                return icode::BWX;
            default:
                console.internalBugErrorOnToken(tok);
        }
    }

    void ir_generator::assignment(const Node& root)
    {
        Unit LHS = getUnitFromIdentifier(*this, root.children[0]);

        Unit RHS = expression(*this, root.children[2]);

        Token assignOperator = root.getNthChildToken(1);

        if (LHS.op.operandType == icode::LITERAL)
            console.compileErrorOnToken("Cannot assign to LITERAL", root.children[0].tok);

        if (!icode::isSameType(LHS.type, RHS.type))
            console.typeError(root.children[2].tok, LHS.type, RHS.type);

        if (!LHS.type.isMutable())
            console.compileErrorOnToken("Cannot modify IMMUTABLE variable or parameter", root.children[0].tok);

        if ((LHS.type.isStruct() || LHS.type.isArray()) && assignOperator.getType() != token::EQUAL)
            console.compileErrorOnToken("Only EQUAL operator allowed on STRUCT or ARRAY", assignOperator);

        if (assignOperator.isBitwiseOperation() && !icode::isInteger(LHS.type.dtype))
            console.compileErrorOnToken("Bitwise operation not allowed on FLOAT", assignOperator);

        icode::Instruction instruction = assignmentTokenToBinaryOperator(assignOperator);

        if (assignOperator.getType() == token::EQUAL)
            functionBuilder.unitCopy(LHS, RHS);
        else
            functionBuilder.unitCopy(LHS, functionBuilder.binaryOperator(instruction, LHS, RHS));
    }

    void ir_generator::block(const Node& root,
                             bool isLoopBlock,
                             const icode::Operand& loopLabel,
                             const icode::Operand& breakLabel,
                             const icode::Operand& continueLabel)
    {
        /* Setup scope */
        scope.createScope();

        for (Node stmt : root.children)
        {
            switch (stmt.type)
            {
                case node::VAR:
                case node::CONST:
                    var(stmt);
                    break;
                case node::ASSIGNMENT:
                case node::ASSIGNMENT_STR:
                case node::ASSIGNMENT_INITLIST:
                    assignment(stmt);
                    break;
                case node::FUNCCALL:
                    functionCall(*this, stmt);
                    break;
                case node::TERM:
                    functionCall(*this, stmt.children[0]);
                    break;
                case node::MODULE:
                {
                    pushWorkingModule();

                    int nodeCounter = setWorkingModuleFromNode(*this, stmt, 0);

                    functionCall(*this, stmt.children[nodeCounter]);

                    popWorkingModule();

                    break;
                }
                case node::IF:
                    ifStatement(*this, stmt, isLoopBlock, loopLabel, breakLabel, continueLabel);
                    break;
                case node::WHILE:
                    whileLoop(*this, stmt);
                    break;
                case node::FOR:
                    forLoop(*this, stmt);
                    break;
                case node::BREAK:
                    breakStatement(*this, isLoopBlock, breakLabel, stmt.tok);
                    break;
                case node::CONTINUE:
                    continueStatement(*this, isLoopBlock, continueLabel, stmt.tok);
                    break;
                case node::RETURN:
                {
                    icode::TypeDescription returnType = (*workingFunction).functionReturnType;

                    if (stmt.children.size() != 0)
                    {
                        Unit returnValue = expression(*this, stmt.children[0]);

                        if (!icode::isSameType(returnType, returnValue.type))
                            console.typeError(stmt.children[0].tok, returnType, returnValue.type);

                        Unit returnPointer = functionBuilder.getReturnPointerUnit();

                        functionBuilder.unitCopy(returnPointer, returnValue);
                    }
                    else if (returnType.dtype != icode::VOID)
                        console.compileErrorOnToken("Ret type is not VOID", stmt.tok);

                    functionBuilder.noArgumentEntry(icode::RET);

                    break;
                }
                case node::PRINT:
                case node::PRINTLN:
                    print(*this, stmt);
                    break;
                case node::INPUT:
                    input(*this, stmt);
                    break;
                case node::EXIT:
                {
                    functionBuilder.noArgumentEntry(icode::EXIT);
                    break;
                }
                default:
                    console.internalBugErrorOnToken(stmt.tok);
            }
        }

        scope.exitScope();
    }

    void ir_generator::initgen(const Node& root)
    {
        /* Get uses */
        for (Node child : root.children)
        {
            if (child.type == node::USE)
                createUse(*this, child);
            else
                break;
        }
    }

    void ir_generator::program(const Node& root)
    {
        /* Setup scope */
        scope.resetScope();

        /* Build symbol table */
        for (Node child : root.children)
        {
            switch (child.type)
            {
                case node::USE:
                    break;
                case node::FROM:
                    createFrom(*this, child);
                    break;
                case node::STRUCT:
                    createStructFromNode(*this, child);
                    break;
                case node::FUNCTION:
                    createFunctionFromNode(*this, child);
                    break;
                case node::ENUM:
                    createEnumFromNode(*this, child);
                    break;
                case node::DEF:
                    createDefineFromNode(*this, child);
                    break;
                case node::VAR:
                    createGlobalFromNode(*this, child);
                    break;
                default:
                    console.internalBugErrorOnToken(child.tok);
            }
        }

        /* Process function blocks */
        for (Node child : root.children)
        {
            if (child.type == node::FUNCTION)
            {
                /* Get function name */
                std::string func_name = child.children[0].tok.toString();

                /* Switch symbol and icode table */
                workingFunction = &rootModule.functions[func_name];
                functionBuilder.setWorkingFunction(workingFunction);
                descriptionFinder.setWorkingFunction(workingFunction);

                /* Clear scope */
                scope.resetScope();

                /* Process block */
                block(child.children.back(),
                      false,
                      opBuilder.createLabelOperand(""),
                      opBuilder.createLabelOperand(""),
                      opBuilder.createLabelOperand(""));

                /* Last instruction must be return */
                if (!functionBuilder.terminateFunction())
                    console.compileErrorOnToken("Missing RETURN for this FUNCTION", child.tok);
            }
        }
    }
} // namespace irgen
