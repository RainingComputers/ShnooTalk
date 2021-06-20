#include <algorithm>

#include "Builder/TypeDescriptionUtil.hpp"
#include "IRGenerator/ConditionalExpression.hpp"
#include "IRGenerator/ControlStatement.hpp"
#include "IRGenerator/Define.hpp"
#include "IRGenerator/Enum.hpp"
#include "IRGenerator/Expression.hpp"
#include "IRGenerator/From.hpp"
#include "IRGenerator/Function.hpp"
#include "IRGenerator/FunctionCall.hpp"
#include "IRGenerator/Global.hpp"
#include "IRGenerator/Input.hpp"
#include "IRGenerator/Module.hpp"
#include "IRGenerator/Structure.hpp"
#include "IRGenerator/TypeDescriptionFromNode.hpp"
#include "IRGenerator/UnitFromIdentifier.hpp"
#include "IRGenerator/Print.hpp"
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
      , moduleBuilder(console)
      , unitBuilder(opBuilder)
      , descriptionFinder(modulesMap[fileName], modulesMap, console, unitBuilder)
      , builder(opBuilder)
      , functionBuilder(modulesMap, console, opBuilder, builder)
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
        icode::Operand curr_offset = builder.createPointer(var.first, var.second.dtypeName, workingModule);

        /* Loop through int and initialize string */
        for (size_t i = 0; i < char_count; i++)
        {
            char character = root.tok.toUnescapedString()[i];

            /* Write to current offset */
            builder.copy(curr_offset, opBuilder.createIntLiteralOperand(icode::UI8, character));

            curr_offset =
              builder.addressAddOperator(curr_offset, opBuilder.createLiteralAddressOperand(var.second.dtypeSize));
        }

        /* Copy null character */
        builder.copy(curr_offset, opBuilder.createIntLiteralOperand(icode::UI8, 0));
    }

    void ir_generator::copy_array(icode::Operand& left, Unit right)
    {
        icode::Operand curr_offset_left = builder.createPointer(left, right.second.dtypeName, workingModule);
        icode::Operand curr_offset_right = builder.createPointer(right.first, right.second.dtypeName, workingModule);

        unsigned int size = right.second.size;
        unsigned int dtype_size = right.second.dtypeSize;

        /* Loop through array and copy each element */
        for (size_t i = 0; i < size; i += dtype_size)
        {
            if (right.second.dtype == icode::STRUCT)
            {
                /* Copy struct from right to left */
                copy_struct(curr_offset_left, Unit(curr_offset_right, right.second));
            }
            else
            {
                /* Copy element from right to left */
                builder.copy(curr_offset_left, curr_offset_right);
            }

            /* Update offset */
            if (i != size - dtype_size)
            {
                icode::Operand update = opBuilder.createLiteralAddressOperand(dtype_size);
                curr_offset_left = builder.addressAddOperator(curr_offset_left, update);
                curr_offset_right = builder.addressAddOperator(curr_offset_right, update);
            }
        }
    }

    void ir_generator::copy_struct(icode::Operand& left, Unit right)
    {
        icode::Operand curr_offset_left = builder.createPointer(left, right.second.dtypeName, workingModule);
        icode::Operand curr_offset_right = builder.createPointer(right.first, right.second.dtypeName, workingModule);

        /* Loop through each field and copy them */
        unsigned int count = 0;
        icode::Operand update;
        for (auto field : rootModule.structures[right.second.dtypeName].structFields)
        {
            icode::TypeDescription field_info = field.second;

            if (count != 0)
            {
                curr_offset_left.dtype = field.second.dtype;
                curr_offset_right.dtype = field.second.dtype;

                curr_offset_left = builder.addressAddOperator(curr_offset_left, update);
                curr_offset_right = builder.addressAddOperator(curr_offset_right, update);
            }

            /* Copy field */
            if (field_info.dimensions.size() != 0)
            {
                copy_array(curr_offset_left, Unit(curr_offset_right, field_info));
            }
            else if (field_info.dtype != icode::STRUCT)
            {
                /* Copy field from right into left */
                builder.copy(curr_offset_left, curr_offset_right);
            }
            else
            {
                copy_struct(curr_offset_left, Unit(curr_offset_right, field_info));
            }

            update = opBuilder.createLiteralAddressOperand(field_info.size);

            count++;
        }
    }

    void ir_generator::assign_init_list_tovar(Unit var, Node& root)
    {
        /* Cannot use initializer list to assign to var */
        if (var.second.dimensions.size() == 0)
            console.compileErrorOnToken("Cannot initialize a NON-ARRAY with initializer list", root.tok);

        icode::Operand curr_offset = builder.createPointer(var.first, var.second.dtypeName, workingModule);

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
                    builder.copy(curr_offset, element_expr.first);
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
                  builder.addressAddOperator(curr_offset, opBuilder.createLiteralAddressOperand(element_var.size));
            }
        }

        if (dim_count < var.second.dimensions[0])
            console.compileErrorOnToken("Dimension size too small", root.tok);
    }

    void ir_generator::var(const Node& root)
    {
        TokenTypePair var = var_from_node(root);
        var.second.setProperty(icode::IS_LOCAL);
        scope.putInCurrentScope(var.first);

        /* Set mutable for var */
        if (root.type == node::VAR)
            var.second.setProperty(icode::IS_MUT);

        /* Check if symbol already exists */
        if (rootModule.symbolExists(var.first.toString()) || (*workingFunction).symbolExists(var.first.toString()))
            console.compileErrorOnToken("Symbol already defined", var.first);

        /* Check for initialization expression or initializer list */
        Node last_node = root.children.back();

        if (last_node.type == node::EXPRESSION || last_node.type == node::TERM)
        {
            /* If an array requires initializer list */
            if (var.second.dimensions.size() != 0)
                console.compileErrorOnToken("Initializer list required to initialize array", last_node.tok);

            /* Create icode operands, one for variable other for temp
                to hold result of initialization expression */
            icode::Operand left = opBuilder.createVarOperand(var.second.dtype, var.first.toString());

            Unit init_exp = expression(*this, last_node);

            /* Check if the type match */
            if (!icode::isSameType(var.second, init_exp.second))
                console.typeError(last_node.tok, var.second, init_exp.second);

            if (var.second.dtype != icode::STRUCT)
            {
                /* Create EQUAL icode entry is not a STRUCT */
                builder.copy(left, init_exp.first);
            }
            else
            {
                copy_struct(left, init_exp);
            }
        }
        else if (last_node.type == node::STR_LITERAL)
        {
            Unit var_pair = Unit(opBuilder.createVarOperand(var.second.dtype, var.first.toString()), var.second);
            assign_str_literal_tovar(var_pair, last_node);
        }
        else if (last_node.type == node::INITLIST)
        {
            Unit var_pair = Unit(opBuilder.createVarOperand(var.second.dtype, var.first.toString()), var.second);
            assign_init_list_tovar(var_pair, last_node);
        }

        /* Add to symbol table */
        (*workingFunction).symbols[var.first.toString()] = var.second;
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

        if (LHS.second.isArray())
            console.compileErrorOnToken("Assignment operators not allowed on ARRAY", assignOperator);

        if (LHS.second.isStruct() && assignOperator.getType() != token::EQUAL)
            console.compileErrorOnToken("Only EQUAL operator allowed on STRUCT", assignOperator);

        if (assignOperator.isBitwiseOperation() && !icode::isInteger(LHS.second.dtype))
            console.compileErrorOnToken("Bitwise operation not allowed on FLOAT", assignOperator);

        icode::Instruction opcode = assignmentTokenToBinaryOperator(assignOperator);

        if (LHS.second.isStruct())
        {
            copy_struct(LHS.first, RHS);
            return;
        }

        if (assignOperator.getType() == token::EQUAL)
        {
            builder.copy(LHS.first, RHS.first);
        }
        else
        {
            icode::Operand temp = opBuilder.createTempOperand(LHS.second.dtype);
            builder.copy(temp, LHS.first);
            builder.binaryOperator(opcode, LHS.first, temp, RHS.first);
        }
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
                    assignment(stmt);
                    break;
                case node::ASSIGNMENT_STR:
                {
                    Unit var = getUnitFromIdentifier(*this, stmt.children[0]);
                    assign_str_literal_tovar(var, stmt.children[1]);
                    break;
                }
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
                    icode::TypeDescription ret_info = (*workingFunction).functionReturnType;

                    /* Get return value */
                    if (stmt.children.size() != 0)
                    {
                        Unit ret_val = expression(*this, stmt.children[0]);

                        /* Type check */
                        if (!icode::isSameType(ret_info, ret_val.second))
                            console.typeError(stmt.children[0].tok, ret_info, ret_val.second);

                        /* Assign return value to return pointer */
                        icode::Operand ret_ptr = opBuilder.createRetPointerOperand(ret_info.dtype);

                        if (ret_val.second.dtype == icode::STRUCT)
                            copy_struct(ret_ptr, ret_val);
                        else
                        {
                            builder.copy(ret_ptr, ret_val.first);
                        }
                    }
                    else if (ret_info.dtype != icode::VOID)
                        console.compileErrorOnToken("Ret type is not VOID", stmt.tok);

                    /* Add return to icode */
                    builder.noArgumentEntry(icode::RET);

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
                    builder.noArgumentEntry(icode::EXIT);
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
                builder.setWorkingFunction(workingFunction);
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
