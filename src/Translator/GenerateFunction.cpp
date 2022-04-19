#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"

#include "AllocPointer.hpp"
#include "BinaryOperator.hpp"
#include "Branch.hpp"
#include "CastOperator.hpp"
#include "CompareOperator.hpp"
#include "CreateSymbol.hpp"
#include "FunctionCall.hpp"
#include "GetAndSetLLVM.hpp"
#include "Input.hpp"
#include "Print.hpp"
#include "ReadWriteCopy.hpp"
#include "StackAlloca.hpp"
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
            case icode::PTR_ASSIGN:
                pointerAssign(ctx, e);
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
            case icode::ADDR:
                addrOperator(ctx, e);
                break;
            case icode::PTR_CAST:
                pointerCastOperator(ctx, e);
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
            case icode::ALLOC_PTR:
                allocPointer(ctx, e);
                break;
            case icode::ALLOC_ARRAY_PTR:
                allocArrayPointer(ctx, e);
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
            case icode::PASS_PTR:
                passPointer(ctx, e);
                break;
            case icode::PASS_PTR_PTR:
                passPointerPointer(ctx, e);
                break;
            case icode::CALL:
                call(ctx, e);
                break;
            case icode::RET:
                ret(ctx, functionDesc.functionReturnType);
                break;
            case icode::INPUT:
                input(ctx, formatStringsContext, e);
                break;
            case icode::INPUT_STR:
                inputString(ctx, e);
                break;
            case icode::MEMCPY:
                memCopy(ctx, e);
                break;
            default:
                ctx.console.internalBugError();
        }

        branchContext.prevInstructionGotoOrRet = e.opcode == icode::GOTO || e.opcode == icode::RET;
    }
}

void addParameterAttribute(unsigned int argNo,
                           const icode::TypeDescription& type,
                           llvm::Argument* arg,
                           Function* function)
{
    if (type.isMutable() || !type.isPassedByReference())
        return;

    function->addParamAttr(argNo, llvm::Attribute::ReadOnly);
    function->addParamAttr(argNo, llvm::Attribute::NoCapture);
}

void setupFunctionStack(ModuleContext& ctx, const icode::FunctionDescription& functionDesc, Function* function)
{
    /* Allocate space for local variables */
    for (auto symbol : functionDesc.symbols)
        if (!symbol.second.checkProperty(icode::IS_PARAM))
            createLocalSymbol(ctx, symbol.second, symbol.first);

    for (unsigned int i = 0; i < functionDesc.numParameters(); i++)
    {
        llvm::Argument* arg = function->getArg(i);

        const std::string& argumentName = functionDesc.parameters[i];
        arg->setName(argumentName);

        const icode::TypeDescription& type = functionDesc.symbols.at(argumentName);

        addParameterAttribute(i, type, arg, function);
        createFunctionParameter(ctx, type, arg, argumentName);
    }
}

void setCurrentFunctionReturnValue(ModuleContext& ctx,
                                   const icode::FunctionDescription& functionDesc,
                                   const std::string& name,
                                   Function* function)
{
    if (functionDesc.functionReturnType.isStructOrArrayAndNotPointer())
    {
        llvm::Argument* lastArg = function->arg_end() - 1;
        lastArg->setName(name + "_retValue");
        ctx.currentFunctionReturnValue = lastArg;
    }
    else
    {
        ctx.currentFunctionReturnValue =
            stackAllocName(ctx,
                           typeDescriptionToAllocaLLVMType(ctx, functionDesc.functionReturnType),
                           name + "_retValue");
    }
}

void generateFunction(ModuleContext& ctx,
                      BranchContext& branchContext,
                      const FormatStringsContext& formatStringsContext,
                      const icode::FunctionDescription& functionDesc,
                      const std::string& name)
{
    ctx.clear();
    branchContext.clear();

    Function* function = getLLVMFunction(ctx, name, functionDesc);
    ctx.currentWorkingFunction = function;

    /* Set insertion point to function body */
    BasicBlock* functionBlock = BasicBlock::Create(*ctx.context, "entry", function);
    ctx.builder->SetInsertPoint(functionBlock);

    setupFunctionStack(ctx, functionDesc, function);

    /* Set ret ptr */
    if (functionDesc.functionReturnType.dtype != icode::VOID)
        setCurrentFunctionReturnValue(ctx, functionDesc, name, function);

    /* Convert ShnooTalk function ir to llvm ir */
    translateFunctionIcode(ctx, branchContext, formatStringsContext, functionDesc, function);

    /* Process goto backpacthing */
    processGotoBackpatches(ctx, branchContext);

    verifyFunction(*function);
}
