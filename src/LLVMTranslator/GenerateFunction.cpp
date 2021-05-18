#include "llvm/IR/Verifier.h"

#include "../log.hpp"
#include "BinaryOperator.hpp"
#include "Branch.hpp"
#include "CastOperator.hpp"
#include "CompareOperator.hpp"
#include "CreateSymbol.hpp"
#include "FunctionCall.hpp"
#include "GetAndSetLLVM.hpp"
#include "Print.hpp"
#include "Input.hpp"
#include "ReadWriteCopy.hpp"
#include "ToLLVMType.hpp"
#include "UnaryOperator.hpp"

#include "GenerateFunction.hpp"

using namespace llvm;

void translateFunctionIcode(ModuleContext& ctx,
                            BranchContext& branchContext,
                            const FormatStringsContext& formatStringsContext,
                            const icode::FunctionDescription& functionDesc,
                            Function* function)
{
    /* Go through icode and generate llvm ir */
    for (size_t i = 0; i < functionDesc.icodeTable.size(); i++)
    {
        icode::Entry e = functionDesc.icodeTable[i];

        switch (e.opcode)
        {
            case icode::EQUAL:
                copy(ctx, e);
                break;
            case icode::ADD:
            case icode::SUB:
            case icode::MUL:
            case icode::DIV:
            case icode::MOD:
            case icode::LSH:
            case icode::RSH:
            case icode::BWA:
            case icode::BWO:
            case icode::BWX:
                binaryOperator(ctx, e);
                break;
            case icode::NOT:
                bitwiseNot(ctx, e);
                break;
            case icode::UNARY_MINUS:
                unaryMinus(ctx, e);
                break;
            case icode::CAST:
                castOperator(ctx, e);
                break;
            case icode::EQ:
            case icode::NEQ:
            case icode::LT:
            case icode::LTE:
            case icode::GT:
            case icode::GTE:
                compareOperator(ctx, branchContext, e);
                break;
            case icode::CREATE_LABEL:
                createLabel(ctx, branchContext, e, function);
                break;
            case icode::IF_TRUE_GOTO:
            case icode::IF_FALSE_GOTO:
            case icode::GOTO:
                createGotoBackpatch(ctx, branchContext, e, function, i);
                break;
            case icode::CREATE_PTR:
                createPointer(ctx, e);
                break;
            case icode::ADDR_ADD:
            case icode::ADDR_MUL:
                addressBinaryOperator(ctx, e);
                break;
            case icode::READ:
                read(ctx, e);
                break;
            case icode::WRITE:
                write(ctx, e);
                break;
            case icode::PRINT:
                print(ctx, formatStringsContext, e);
                break;
            case icode::PRINT_STR:
                printString(ctx, e);
                break;
            case icode::NEWLN:
                callPrintf(ctx, formatStringsContext.newLineString, nullptr);
                break;
            case icode::SPACE:
                callPrintf(ctx, formatStringsContext.spaceString, nullptr);
                break;
            case icode::PASS:
                pass(ctx, e);
                break;
            case icode::PASS_ADDR:
                passPointer(ctx, e);
                break;
            case icode::CALL:
                call(ctx, e);
                break;
            case icode::RET:
                ret(ctx, e, functionDesc.functionReturnDescription.dtype);
                break;
            case icode::INPUT:
                input(ctx, formatStringsContext, e);
                break;
            case icode::INPUT_STR:
                inputString(ctx, e);
                break;
            case icode::EXIT:
                break;
            default:
                miklog::internal_error(ctx.moduleDescription.name);
                throw miklog::internal_bug_error();
        }

        branchContext.prevInstructionGotoOrRet = e.opcode == icode::GOTO || e.opcode == icode::RET;
    }
}

void setupFunctionStack(ModuleContext& ctx, const icode::FunctionDescription& functionDesc, Function* function)
{
    /* Allocate space for local variables */
    for (auto symbol : functionDesc.symbols)
        if (!symbol.second.checkProperty(icode::IS_PARAM))
            createLocalSymbol(ctx, symbol.second, symbol.first);

    /* Assign passed function args */
    unsigned int argumentCounter = 0;
    for (auto& arg : function->args())
    {
        const std::string& argumentName = functionDesc.parameters[argumentCounter];
        arg.setName(argumentName);
        createFunctionParameter(ctx, functionDesc.symbols.at(argumentName), argumentName, &arg);
        argumentCounter++;
    }
}

void setCurrentFunctionReturnPointer(ModuleContext& ctx,
                                     const icode::FunctionDescription& functionDesc,
                                     const std::string& name)
{
    ctx.currentFunctionReturnPointer =
      ctx.builder->CreateAlloca(variableDescriptionToLLVMType(ctx, functionDesc.functionReturnDescription),
                                nullptr,
                                name + ".retPointer");
}

void generateFunction(ModuleContext& ctx,
                      BranchContext& branchContext,
                      const FormatStringsContext& formatStringsContext,
                      const icode::FunctionDescription& functionDesc,
                      const std::string& name)
{
    ctx.clear();
    branchContext.clear();

    Function* function = getLLVMFunction(ctx, name, ctx.moduleDescription.name);

    /* Set insertion point to function body */
    BasicBlock* functionBlock = BasicBlock::Create(*ctx.context, "entry", function);
    ctx.builder->SetInsertPoint(functionBlock);

    setupFunctionStack(ctx, functionDesc, function);

    /* Set ret ptr */
    if (functionDesc.functionReturnDescription.dtype != icode::VOID)
        setCurrentFunctionReturnPointer(ctx, functionDesc, name);

    /* Convert mikuro function ir to llvm ir */
    translateFunctionIcode(ctx, branchContext, formatStringsContext, functionDesc, function);

    /* Process goto backpacthing */
    processGotoBackpatches(ctx, branchContext);

    verifyFunction(*function);
}