#include "Branch.hpp"

using namespace llvm;

void createLabel(const ModuleContext& ctx, BranchContext& branchContext, const icode::Entry& e, Function* function)
{
    /* Converts ShnooTalk CREATE_LABEL to llvm basic block  */
    BasicBlock* newBlock = BasicBlock::Create(*ctx.context, e.op1.name, function);

    branchContext.labelToBasicBlockMap[e.op1.name] = newBlock;

    /* Make sure old block has a terminator */
    if (!branchContext.prevInstructionGotoOrRet)
        ctx.builder->CreateBr(newBlock);

    ctx.builder->SetInsertPoint(newBlock);
}

void createGotoBackpatch(const ModuleContext& ctx,
                         BranchContext& branchContext,
                         const icode::Entry& e,
                         Function* F,
                         size_t entryIndex)
{
    /* All branch instructions GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO are backpatched,
    i.e the task is stored in a queue and instructions are created in a second pass */

    /* To prevent duplicate block terminators */
    if (branchContext.prevInstructionGotoOrRet)
        return;

    /* Save llvm insertion point corresponding to this entry */
    branchContext.insertionPoints[entryIndex] =
        InsertionPoint(ctx.builder->GetInsertBlock(), ctx.builder->GetInsertPoint()++);

    /* Append to backpatch queue */
    branchContext.backpatchQueue.push_back(EnumeratedEntry(entryIndex, e));

    if (e.opcode == icode::GOTO)
    {
        branchContext.fallBlocks[entryIndex] = nullptr;
        return;
    }

    /* Create basic block for fall through for IF_TRUE_GOTO, IF_FALSE_GOTO */
    BasicBlock* fallBlock = BasicBlock::Create(*ctx.context, "_fall_e" + std::to_string(entryIndex), F);

    branchContext.fallBlocks[entryIndex] = fallBlock;

    /* Start inserstion in fall through block */
    ctx.builder->SetInsertPoint(fallBlock);
}

void createBranch(const ModuleContext& ctx,
                  const icode::Entry& e,
                  Value* flag,
                  BasicBlock* gotoBlock,
                  BasicBlock* fallBlock)
{
    /* Convert ShnooTalk GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO int llvm ir  */

    switch (e.opcode)
    {
        case icode::GOTO:
            ctx.builder->CreateBr(gotoBlock);
            break;
        case icode::IF_TRUE_GOTO:
            ctx.builder->CreateCondBr(flag, gotoBlock, fallBlock);
            break;
        case icode::IF_FALSE_GOTO:
            ctx.builder->CreateCondBr(flag, fallBlock, gotoBlock);
            break;
        default:
            ctx.console.internalBugError();
    }
}

void processGotoBackpatches(const ModuleContext& ctx, BranchContext& branchContext)
{
    /* Go through backpatch queue */
    for (size_t i = 0; i < branchContext.backpatchQueue.size(); i++)
    {
        /* Get the entry from backpatch q */
        const EnumeratedEntry& enumeratedEntry = branchContext.backpatchQueue[i];
        size_t entryIndex = enumeratedEntry.first;
        const icode::Entry& e = enumeratedEntry.second;

        /* Get branch flags and blocks for the goto */
        BasicBlock* gotoBlock = branchContext.labelToBasicBlockMap.at(e.op1.name);
        BasicBlock* fallBlock = branchContext.fallBlocks.at(entryIndex);

        /* Conditional branch instruction are always follwed by compare instructions that set
         the imaginary flag register */
        Value* flag = nullptr;
        if (e.opcode != icode::GOTO)
            flag = branchContext.branchFlags.front();

        /* Get insertion point corresponding to the entry */
        InsertionPoint insertPoint = branchContext.insertionPoints.at(entryIndex);
        ctx.builder->SetInsertPoint(insertPoint.first, insertPoint.second);

        /* Create branch in the ^ insertion point */
        createBranch(ctx, e, flag, gotoBlock, fallBlock);

        /* Pop from flags queue after processing branch */
        if (e.opcode != icode::GOTO)
            branchContext.branchFlags.pop();
    }
}