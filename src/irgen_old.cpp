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
      , descriptionFinder(modulesMap[fileName], modulesMap, console, unitBuilder)
      , functionBuilder(modulesMap, console, opBuilder, unitBuilder)
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

    TokenTypePair ir_generator::var_from_node(const Node& root)
    {
        return TokenTypePair(root.getNthChildToken(0), typeDescriptionFromNode(*this, root));
    }

    void ir_generator::assign_str_literal_tovar(Unit var, Node& root)
    {
        if (var.second.dimensions.size() != 1 || var.second.dtype != icode::UI8)
            console.compileErrorOnToken("String assignment only allowed on 1D INT ARRAY", root.tok);

        /* Check size */
        size_t char_count = root.tok.toUnescapedString().length();

        if (char_count > var.second.dimensions[0])
            console.compileErrorOnToken("String too big", root.tok);

        /* Create Addr Temp */
        icode::Operand curr_offset = functionBuilder.createPointer(var.first, var.second.dtypeName, workingModule);

        /* Loop through int and initialize string */
        for (size_t i = 0; i < char_count; i++)
        {
            char character = root.tok.toUnescapedString()[i];

            /* Write to current offset */
            functionBuilder.operandCopy(curr_offset, opBuilder.createIntLiteralOperand(icode::UI8, character));

            curr_offset =
              functionBuilder.addressAddOperator(curr_offset,
                                                 opBuilder.createLiteralAddressOperand(var.second.dtypeSize));
        }

        /* Copy null character */
        functionBuilder.operandCopy(curr_offset, opBuilder.createIntLiteralOperand(icode::UI8, 0));
    }

    void ir_generator::copy_struct(icode::Operand& left, Unit right)
    {
        icode::Operand curr_offset_left = functionBuilder.createPointer(left, right.second.dtypeName, workingModule);
        icode::Operand curr_offset_right =
          functionBuilder.createPointer(right.first, right.second.dtypeName, workingModule);

        functionBuilder.memCopy(curr_offset_left, curr_offset_right, right.second.size);
    }

    void ir_generator::assign_init_list_tovar(Unit var, Node& root)
    {
        /* Cannot use initializer list to assign to var */
        if (var.second.dimensions.size() == 0)
            console.compileErrorOnToken("Cannot initialize a NON-ARRAY with initializer list", root.tok);

        icode::Operand curr_offset = functionBuilder.createPointer(var.first, var.second.dtypeName, workingModule);

        /* Create var info for the elements inside the list */
        icode::TypeDescription element_var = var.second;
        element_var.size /= element_var.dimensions[0];
        element_var.dimensions.erase(element_var.dimensions.begin());

        /* Keep track of number of dimensions written */
        unsigned int dim_count = 0;

        for (size_t i = 0; i < root.children.size(); i++)
        {
            Node child = root.children[i];

            if (dim_count >= var.second.dimensions[0])
                console.compileErrorOnToken("Dimension size too big", child.tok);

            if (element_var.dimensions.size() == 0)
            {
                /* Check if expression */
                if (child.type != node::TERM && child.type != node::EXPRESSION)
                    console.compileErrorOnToken("Incorrect dimensions", child.tok);

                Unit element_expr = expression(*this, child);

                /* Type check */
                if (!icode::isSameType(element_var, element_expr.second))
                    console.typeError(child.tok, element_var, element_expr.second);

                if (element_expr.second.dtype != icode::STRUCT)
                {
                    /* Write to current offset if not a struct */
                    functionBuilder.operandCopy(curr_offset, element_expr.first);
                }
                else
                {
                    copy_struct(curr_offset, element_expr);
                }
            }
            else if (child.type == node::STR_LITERAL)
            {
                assign_str_literal_tovar(Unit(curr_offset, element_var), child);
            }
            else
            {
                assign_init_list_tovar(Unit(curr_offset, element_var), child);
            }

            dim_count++;

            /* Update offset */
            if (i != root.children.size() - 1)
            {
                curr_offset =
                  functionBuilder.addressAddOperator(curr_offset,
                                                     opBuilder.createLiteralAddressOperand(element_var.size));
            }
        }

        if (dim_count < var.second.dimensions[0])
            console.compileErrorOnToken("Dimension size too small", root.tok);
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

        if (lastNode.type == node::EXPRESSION || lastNode.type == node::TERM || lastNode.type == node::STR_LITERAL)
        {
            Unit RHS = expression(*this, lastNode);

            if (!icode::isSameType(local.second, RHS.second))
                console.typeError(lastNode.tok, local.second, RHS.second);

           functionBuilder.unitCopy(local, RHS);
        }
        else if (lastNode.type == node::INITLIST)
            assign_init_list_tovar(local, lastNode);
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

        if (LHS.first.operandType == icode::LITERAL)
            console.compileErrorOnToken("Cannot assign to LITERAL", root.children[0].tok);

        if (!icode::isSameType(LHS.second, RHS.second))
            console.typeError(root.children[2].tok, LHS.second, RHS.second);

        if (!LHS.second.isMutable())
            console.compileErrorOnToken("Cannot modify IMMUTABLE variable or parameter", root.children[0].tok);

        if ((LHS.second.isStruct() || LHS.second.isArray()) && assignOperator.getType() != token::EQUAL)
            console.compileErrorOnToken("Only EQUAL operator allowed on STRUCT or ARRAY", assignOperator);

        if (assignOperator.isBitwiseOperation() && !icode::isInteger(LHS.second.dtype))
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
                    assignment(stmt);
                    break;
                case node::ASSIGNMENT_INITLIST:
                {
                    Unit var = getUnitFromIdentifier(*this, stmt.children[0]);
                    assign_init_list_tovar(var, stmt.children[1]);
                    break;
                }
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

                    /* Get return value */
                    if (stmt.children.size() != 0)
                    {
                        Unit returnValue = expression(*this, stmt.children[0]);

                        if (!icode::isSameType(returnType, returnValue.second))
                            console.typeError(stmt.children[0].tok, returnType, returnValue.second);

                        /* Assign return value to return pointer */
                        icode::Operand returnPointer = opBuilder.createRetPointerOperand(returnType.dtype);

                        if (returnValue.second.dtype == icode::STRUCT)
                            copy_struct(returnPointer, returnValue);
                        else
                        {
                            functionBuilder.operandCopy(returnPointer, returnValue.first);
                        }
                    }
                    else if (returnType.dtype != icode::VOID)
                        console.compileErrorOnToken("Ret type is not VOID", stmt.tok);

                    /* Add return to icode */
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
