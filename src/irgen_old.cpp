#include <algorithm>

#include "IRGenerator/Define.hpp"
#include "IRGenerator/Enum.hpp"
#include "IRGenerator/Expression.hpp"
#include "IRGenerator/From.hpp"
#include "IRGenerator/Function.hpp"
#include "IRGenerator/Global.hpp"
#include "IRGenerator/Module.hpp"
#include "IRGenerator/Structure.hpp"
#include "IRGenerator/TypeDescriptionFromNode.hpp"
#include "IRGenerator/UnitFromIdentifier.hpp"
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
      , builder(modulesMap[fileName], modulesMap, opBuilder)
      , functionBuilder(modulesMap, console, opBuilder, builder)
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

    bool ir_generator::get_func(const std::string& name, icode::FunctionDescription& func)
    {
        if ((*workingModule).getFunction(name, func))
            return true;

        if (rootModule.getFunction(name, func))
            return true;

        return false;
    }

    TokenTypePair ir_generator::var_from_node(const Node& root)
    {
        return TokenTypePair(root.getNthChildToken(0), typeDescriptionFromNode(*this, root));
    }

    icode::Operand ir_generator::gen_str_dat(const Token& str_token, size_t char_count, icode::DataType dtype)
    {
        /* Append string data */
        std::string name = "_str" + str_token.getLineColString();
        rootModule.stringsData[name] = str_token.toUnescapedString();

        /* Create icode::operand */
        size_t size = char_count * icode::getDataTypeSize(dtype);
        icode::Operand opr = opBuilder.createStringDataOperand(name, size);

        return opr;
    }

    Unit ir_generator::var_info_to_str_dat(const Token& str_token, icode::TypeDescription var)
    {
        if (var.dimensions.size() != 1 || var.dtype != icode::UI8)
            console.compileErrorOnToken("String assignment only allowed on 1D CHAR ARRAY", str_token);

        /* Check dimensions */
        size_t char_count = str_token.toUnescapedString().length();

        if (char_count > var.dimensions[0])
            console.compileErrorOnToken("String too big", str_token);

        /* Create STR_DAT operand */
        icode::Operand opr = gen_str_dat(str_token, char_count, var.dtype);

        return Unit(opr, var);
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
        icode::Operand curr_offset = builder.createPointer(var.first);

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
        icode::Operand curr_offset_left = builder.createPointer(left);
        icode::Operand curr_offset_right = builder.createPointer(right.first);

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
        icode::Operand curr_offset_left = builder.createPointer(left);
        icode::Operand curr_offset_right = builder.createPointer(right.first);

        /* Loop through each field and copy them */
        unsigned int count = 0;
        icode::Operand update;
        for (auto field : rootModule.structures[right.second.dtypeName].structFields)
        {
            icode::TypeDescription field_info = field.second;

            if (count != 0)
            {
                curr_offset_left.updateDataType(field.second);
                curr_offset_right.updateDataType(field.second);

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

        icode::Operand curr_offset = builder.createPointer(var.first);

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
            icode::Operand left =
              opBuilder.createVarOperand(var.second.dtype, var.second.dtypeName, var.first.toString());

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
            Unit var_pair =
              Unit(opBuilder.createVarOperand(var.second.dtype, var.second.dtypeName, var.first.toString()),
                   var.second);
            assign_str_literal_tovar(var_pair, last_node);
        }
        else if (last_node.type == node::INITLIST)
        {
            Unit var_pair =
              Unit(opBuilder.createVarOperand(var.second.dtype, var.second.dtypeName, var.first.toString()),
                   var.second);
            assign_init_list_tovar(var_pair, last_node);
        }

        /* Add to symbol table */
        (*workingFunction).symbols[var.first.toString()] = var.second;
    }

    Unit ir_generator::funccall(const Node& root)
    {
        icode::ModuleDescription* temp = workingModule;

        /* Get the first argument (if not a string literal) */
        Unit first_arg;
        if (root.children.size() != 0)
        {
            if (root.children[0].type != node::STR_LITERAL)
                first_arg = expression(*this, root.children[0]);

            /* If struct funccall, switch to struct's (first arg's) module */
            if (root.type == node::STRUCT_FUNCCALL)
                workingModule = &modulesMap[first_arg.second.moduleName];
        }

        /* Check if function exits */
        icode::FunctionDescription func_desc;
        std::string func_name = root.tok.toString();

        if (!get_func(func_name, func_desc))
            console.compileErrorOnToken("Function does not exist", root.tok);

        /* Check number of parameters */
        if (root.children.size() != func_desc.parameters.size())
            console.compileErrorOnToken("Number of parameters don't match", root.tok);

        /* Go through each parameter */
        for (size_t i = 0; i < root.children.size(); i++)
        {
            /* Get parameter information */
            icode::TypeDescription param = func_desc.symbols[func_desc.parameters[i]];
            bool mut = param.checkProperty(icode::IS_MUT);

            /* Get argument passed to function */
            Unit arg;
            if (root.children[i].type == node::STR_LITERAL)
            {
                arg = var_info_to_str_dat(root.children[i].tok, param);
            }
            else if (i == 0 && root.children[0].type != node::STR_LITERAL)
                arg = first_arg;
            else
                arg = expression(*this, root.children[i]);

            /* Type check */
            if (!icode::isSameType(param, arg.second))
                console.typeError(root.children[i].tok, param, arg.second);

            /* If mutable, it has to be a variable or address */
            if (mut && (arg.first.operandType == icode::TEMP || arg.first.operandType == icode::STR_DATA ||
                        arg.first.operandType == icode::LITERAL))
                console.compileErrorOnToken("Cannot pass an EXPRESSION or STRING LITERAL as MUTABLE",
                                            root.children[i].tok);

            /* If param is mut, arg has to be mutable to */
            if (mut && !arg.second.checkProperty(icode::IS_MUT))
                console.compileErrorOnToken("Cannot pass IMMUTABLE as MUTABLE", root.children[i].tok);

            /* Pass arguments */

            /* If mut or struct or array, pass by ref */
            if (mut || param.dtype == icode::STRUCT || param.dimensions.size() != 0)
                builder.pass(icode::PASS_ADDR, arg.first, func_name, func_desc);
            /* Else pass by value */
            else
                builder.pass(icode::PASS, arg.first, func_name, func_desc);
        }

        /* Call function */
        icode::Operand ret_temp = builder.call(func_name, func_desc);

        /* Switch back to current module if ext_mod was modified */
        if (root.type == node::STRUCT_FUNCCALL)
            workingModule = temp;

        return Unit(ret_temp, func_desc.functionReturnDescription);
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

        Token assignOperator = root.children[1].tok;

        if (LHS.first.operandType == icode::LITERAL)
            console.compileErrorOnToken("Cannot assign to LITERAL", root.children[0].tok);

        if (!icode::isSameType(LHS.second, RHS.second))
            console.typeError(root.children[2].tok, LHS.second, RHS.second);

        if (!LHS.second.checkProperty(icode::IS_MUT))
            console.compileErrorOnToken("Cannot modify IMMUTABLE variable or parameter", root.children[0].tok);

        if (LHS.second.dimensions.size() != 0)
            console.compileErrorOnToken("Assignment operators not allowed on ARRAY", assignOperator);

        if (LHS.second.dtype == icode::STRUCT && assignOperator.getType() != token::EQUAL)
            console.compileErrorOnToken("Only EQUAL operator allowed on STRUCT", assignOperator);

        if (assignOperator.isBitwiseOperation() && !icode::isInteger(LHS.second.dtype))
            console.compileErrorOnToken("Bitwise operation not allowed on FLOAT", assignOperator);

        icode::Instruction opcode = assignmentTokenToBinaryOperator(assignOperator);

        if (LHS.second.dtype == icode::STRUCT)
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
            icode::Operand temp = opBuilder.createTempOperand(LHS.second.dtype, LHS.second.dtypeName);
            builder.copy(temp, LHS.first);
            builder.binaryOperator(opcode, LHS.first, temp, RHS.first);
        }
    }

    icode::Operand ir_generator::gen_label(Token tok, bool true_label, std::string prefix)
    {
        /* Generate label using token's line and col number */

        std::string label_name = tok.getLineColString();

        if (true_label)
            return opBuilder.createLabelOperand("_" + prefix + "_true" + label_name);

        return opBuilder.createLabelOperand("_" + prefix + "_false" + label_name);
    }

    icode::Instruction ir_generator::tokenToCompareOperator(const Token tok)
    {
        switch (tok.getType())
        {
            case token::LESS_THAN:
                return icode::LT;
            case token::LESS_THAN_EQUAL:
                return icode::LTE;
            case token::GREATER_THAN:
                return icode::GT;
            case token::GREATER_THAN_EQUAL:
                return icode::GTE;
            case token::CONDN_EQUAL:
                return icode::EQ;
            case token::CONDN_NOT_EQUAL:
                return icode::NEQ;
            default:
                console.compileErrorOnToken("Invalid conditional expression", tok);
        }
    }

    void ir_generator::condn_expression(const Node& root,
                                        const icode::Operand& t_label,
                                        const icode::Operand& f_label,
                                        bool t_fall,
                                        bool f_fall)
    {
        if (root.type == node::TERM)
        {
            /* For unary operator NOT */
            if (root.children[0].type == node::EXPRESSION)
            {
                condn_expression(root.children[0], t_label, f_label, t_fall, f_fall);
                return;
            }

            if (root.children[0].tok.getType() != token::CONDN_NOT)
                console.compileErrorOnToken("Invalid conditional expression", root.tok);

            condn_expression(root.children[0].children[0].children[0], f_label, t_label, f_fall, t_fall);
        }
        else if (root.children.size() == 1)
        {
            condn_expression(root.children[0], t_label, f_label, t_fall, f_fall);
        }
        else
        {
            Token expr_opr = root.children[1].tok;

            /* See the dragon book, Figure 6.39 and Figure 6.40 */

            switch (expr_opr.getType())
            {
                case token::CONDN_AND:
                {
                    icode::Operand new_t_label = gen_label(expr_opr, true);
                    icode::Operand new_f_label = gen_label(expr_opr, false);

                    if (!f_fall)
                        condn_expression(root.children[0], new_t_label, f_label, true, false);
                    else
                        condn_expression(root.children[0], new_t_label, new_f_label, true, false);

                    condn_expression(root.children[2], t_label, f_label, t_fall, f_fall);

                    if (f_fall)
                    {
                        builder.label(new_f_label);
                    }

                    break;
                }
                case token::CONDN_OR:
                {
                    icode::Operand new_t_label = gen_label(expr_opr, true);
                    icode::Operand new_f_label = gen_label(expr_opr, false);

                    if (!t_fall)
                        condn_expression(root.children[0], t_label, new_f_label, false, true);
                    else
                        condn_expression(root.children[0], new_t_label, new_f_label, false, true);

                    condn_expression(root.children[2], t_label, f_label, t_fall, f_fall);

                    if (t_fall)
                    {
                        builder.label(new_t_label);
                    }

                    break;
                }
                default:
                {
                    icode::Instruction opcode = tokenToCompareOperator(expr_opr);

                    /* Create icode entry for comparing two expressions */
                    Unit first_operand = expression(*this, root.children[0]);

                    /* Cannot compare structs and arrays */
                    if (first_operand.second.dtype == icode::STRUCT || first_operand.second.dimensions.size() != 0)
                        console.compileErrorOnToken("Cannot compare STRUCT or ARRAYS", expr_opr);

                    Unit second_operand = expression(*this, root.children[2]);

                    /* Type check */
                    if (!icode::isSameType(first_operand.second, second_operand.second))
                        console.typeError(root.children[2].tok, first_operand.second, second_operand.second);

                    /* If second operand is a ptr, read it into a temp */
                    builder.compareOperator(opcode, first_operand.first, second_operand.first);

                    /* Create icode entry for goto */

                    if (!t_fall)
                        builder.createBranch(icode::IF_TRUE_GOTO, t_label);

                    if (!f_fall)
                        builder.createBranch(icode::IF_FALSE_GOTO, f_label);
                }
            }
        }
    }

    void ir_generator::ifstmt(const Node& root,
                              bool loop,
                              const icode::Operand& start_label,
                              const icode::Operand& break_label,
                              const icode::Operand& cont_label)
    {
        /* Create label for end of all if statements */
        icode::Operand end_label = gen_label(root.tok, false, "ifend");

        for (size_t i = 0; i < root.children.size(); i++)
        {
            Node child = root.children[i];

            icode::Operand new_t_label = gen_label(child.tok, true, "if");
            icode::Operand new_f_label = gen_label(child.tok, false, "if");

            if (child.type != node::ELSE)
            {
                /* Process conditional expression */
                condn_expression(child.children[0], new_t_label, new_f_label, true, false);

                /* Process block */
                block(child.children[1], loop, start_label, break_label, cont_label);

                if (i != root.children.size() - 1)
                {
                    /* Go to end */
                    builder.createBranch(icode::GOTO, end_label);
                }

                /* Create label to skip block */
                builder.label(new_f_label);
            }
            else
            {
                block(child.children[0], loop, start_label, break_label, cont_label);
            }
        }

        if (root.children.size() != 1)
        {
            /* Create label for end of if statement */
            builder.label(end_label);
        }
    }

    void ir_generator::whileloop(const Node& root)
    {
        icode::Operand new_t_label = gen_label(root.tok, true, "while");
        icode::Operand new_f_label = gen_label(root.tok, false, "while");

        /* Create label for looping */
        builder.label(new_t_label);

        /* Process conditional expression */
        condn_expression(root.children[0], new_t_label, new_f_label, true, false);

        /* Process block */
        block(root.children[1], true, new_t_label, new_f_label, new_t_label);

        /* Go back to beginning */
        builder.createBranch(icode::GOTO, new_t_label);

        /* Create label to skip block */
        builder.label(new_f_label);
    }

    void ir_generator::forloop(const Node& root)
    {
        /* Process initialization */
        if (root.children[0].type == node::VAR)
            var(root.children[0]);
        else
            assignment(root.children[0]);

        /* Process conditional  */
        icode::Operand new_t_label = gen_label(root.tok, true, "for");
        icode::Operand new_f_label = gen_label(root.tok, false, "for");
        icode::Operand cont_label = gen_label(root.tok, true, "for_cont");

        /* Create label for looping */
        builder.label(new_t_label);

        /* Process conditional expression */
        condn_expression(root.children[1], new_t_label, new_f_label, true, false);

        /* Process block */
        block(root.children[3], true, new_t_label, new_f_label, cont_label);

        /* Create label for continue */
        builder.label(cont_label);

        /* Process assignment */
        assignment(root.children[2]);

        /* Go back to beginning */
        builder.createBranch(icode::GOTO, new_t_label);

        /* Create label to skip block */
        builder.label(new_f_label);
    }

    void ir_generator::print(const Node& root)
    {
        for (size_t i = 0; i < root.children.size(); i++)
        {
            Node child = root.children[i];

            /* If string literal, create temp string an print it */
            if (child.type == node::STR_LITERAL)
            {
                /* Get str len and str size */
                int char_count = child.tok.toUnescapedString().length();

                icode::Operand str_dat_opr = gen_str_dat(child.tok, char_count, icode::UI8);

                builder.printOperator(icode::PRINT_STR, str_dat_opr);
            }
            /* Else expression */
            else
            {
                Unit print_var = expression(*this, child);

                /* Cannot peint struct or arrays */
                if (print_var.second.dtype == icode::STRUCT || print_var.second.dimensions.size() > 1)
                    console.compileErrorOnToken("Cannot print STRUCT or multi-dimensional ARRAY", child.tok);

                if (print_var.second.dimensions.size() != 0)
                    builder.printOperator(icode::PRINT_STR, print_var.first);
                else
                    builder.printOperator(icode::PRINT, print_var.first);
            }

            /* Add space after printing (except for the last print) */
            if (i != root.children.size() - 1)
                builder.noArgumentEntry(icode::SPACE);

            /* If println, the last print should add new line */
            if (i == root.children.size() - 1 && root.type == node::PRINTLN)
                builder.noArgumentEntry(icode::NEWLN);
        }
    }

    void ir_generator::input(const Node& root)
    {
        Unit input_var = expression(*this, root.children[0]);

        /* Check if the input var is writable */
        if (!(input_var.first.operandType == icode::VAR || input_var.first.operandType == icode::GBL_VAR ||
              input_var.first.operandType == icode::TEMP_PTR || input_var.first.operandType == icode::PTR))
            console.compileErrorOnToken("Invalid term for INPUT", root.children[0].tok);

        if (input_var.second.dtype == icode::STRUCT)
            console.compileErrorOnToken("Cannot INPUT STRUCT", root.children[0].tok);

        if (input_var.second.dimensions.size() > 1)
            console.compileErrorOnToken("Cannot INPUT more than 1D ARRAY", root.children[0].tok);

        if (input_var.second.dimensions.size() == 1 && input_var.first.dtype == icode::UI8)
            console.compileErrorOnToken("String input requires 1D CHAR ARRAY", root.children[0].tok);

        /* Create INPUT or INPUT_STR entry */
        if (input_var.second.dimensions.size() == 0)
            builder.inputOperator(icode::INPUT, input_var.first);
        else
            builder.inputOperator(icode::INPUT_STR, input_var.first, input_var.second.dimensions[0]);
    }

    void ir_generator::block(const Node& root,
                             bool loop,
                             const icode::Operand& start_label,
                             const icode::Operand& break_label,
                             const icode::Operand& cont_label)
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
                    /* The variable to write to */
                    Unit var = getUnitFromIdentifier(*this, stmt.children[0]);

                    assign_str_literal_tovar(var, stmt.children[1]);

                    break;
                }
                case node::ASSIGNMENT_INITLIST:
                {
                    /* The variable to write to */
                    Unit var = getUnitFromIdentifier(*this, stmt.children[0]);

                    assign_init_list_tovar(var, stmt.children[1]);

                    break;
                }
                case node::FUNCCALL:
                    funccall(stmt);
                    break;
                case node::TERM:
                    funccall(stmt.children[0]);
                    break;
                case node::MODULE:
                {
                    /* Check if the module exists */
                    if (!(*workingModule).useExists(stmt.tok.toString()))
                        console.compileErrorOnToken("Module does not exist", stmt.tok);

                    /* Switch to external module */
                    icode::ModuleDescription* temp = workingModule;
                    workingModule = &modulesMap[stmt.tok.toString()];

                    Unit ret_val = funccall(stmt.children[0]);

                    /* Switch back to self */
                    workingModule = temp;

                    break;
                }
                case node::IF:
                    ifstmt(stmt, loop, start_label, break_label, cont_label);
                    break;
                case node::WHILE:
                    whileloop(stmt);
                    break;
                case node::FOR:
                    forloop(stmt);
                    break;
                case node::BREAK:
                {
                    if (!loop)
                        console.compileErrorOnToken("BREAK outside loop", stmt.tok);

                    /* Go to end */
                    builder.createBranch(icode::GOTO, break_label);

                    break;
                }
                case node::CONTINUE:
                {
                    if (!loop)
                        console.compileErrorOnToken("CONTINUE outside loop", stmt.tok);

                    /* Go to end */
                    builder.createBranch(icode::GOTO, cont_label);

                    break;
                }
                case node::RETURN:
                {
                    icode::TypeDescription ret_info = (*workingFunction).functionReturnDescription;

                    /* Get return value */
                    if (stmt.children.size() != 0)
                    {
                        Unit ret_val = expression(*this, stmt.children[0]);

                        /* Type check */
                        if (!icode::isSameType(ret_info, ret_val.second))
                            console.typeError(stmt.children[0].tok, ret_info, ret_val.second);

                        /* Assign return value to return pointer */
                        icode::Operand ret_ptr = opBuilder.createRetPointerOperand(ret_info.dtype, ret_info.dtypeName);

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
                    print(stmt);
                    break;
                case node::INPUT:
                    input(stmt);
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

    bool ir_generator::current_function_terminates()
    {
        if ((*workingFunction).icodeTable.size() < 1)
            return false;

        icode::Instruction last_opcode = (*workingFunction).icodeTable.back().opcode;

        if (last_opcode == icode::RET)
            return true;

        return false;
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
                builder.setFunctionDescription(workingFunction);
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
                if (!current_function_terminates())
                {
                    if (workingFunction->functionReturnDescription.dtype != icode::VOID)
                        console.compileErrorOnToken("Missing RETURN for this FUNCTION", child.tok);

                    builder.noArgumentEntry(icode::RET);
                }
            }
        }
    }
} // namespace irgen
