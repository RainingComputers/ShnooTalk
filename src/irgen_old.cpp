#include <algorithm>

#include "Builder/TypeDescriptionUtil.hpp"
#include "Generator/Assignment.hpp"
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
#include "Generator/Local.hpp"
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

    void ir_generator::setWorkingFunction(icode::FunctionDescription* functionDescription)
    {
        workingFunction = functionDescription;
        functionBuilder.setWorkingFunction(workingFunction);
        descriptionFinder.setWorkingFunction(workingFunction);
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
                    local(*this, stmt);
                    break;
                case node::ASSIGNMENT:
                case node::ASSIGNMENT_STR:
                case node::ASSIGNMENT_INITLIST:
                    assignment(*this, stmt);
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
                const Token& functionNameToken = child.children[0].tok;

                setWorkingFunction(&rootModule.functions[functionNameToken.toString()]);

                scope.resetScope();

                block(child.children.back(),
                      false,
                      opBuilder.createLabelOperand(""),
                      opBuilder.createLabelOperand(""),
                      opBuilder.createLabelOperand(""));

                functionBuilder.terminateFunction(functionNameToken);
            }
        }
    }
} // namespace irgen
