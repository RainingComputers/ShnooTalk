#include "BranchContext.hpp"

void BranchContext::clear()
{
    labelToBasicBlockMap.clear();
    fallBlocks.clear();
    insertionPoints.clear();
    backpatchQueue.clear();
    prevInstructionGotoOrRet = false;
}