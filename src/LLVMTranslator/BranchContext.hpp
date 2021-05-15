#ifndef TRANSLATOR_BRANCH_CONTEXT_HPP
#define TRANSLATOR_BRANCH_CONTEXT_HPP

#include <map>
#include <queue>
#include <vector>

#include "llvm/IR/BasicBlock.h"

#include "../IntermediateRepresentation/All.hpp"

typedef std::pair<llvm::BasicBlock*, llvm::BasicBlock::iterator> InsertionPoint;
typedef std::pair<size_t, icode::Entry> EnumeratedEntry;

struct BranchContext
{
    std::map<icode::Operand, llvm::BasicBlock*> labelToBasicBlockMap;
    std::map<size_t, llvm::BasicBlock*> fallBlocks;
    std::queue<llvm::Value*> branchFlags;
    std::map<size_t, InsertionPoint> insertionPoints;
    std::vector<EnumeratedEntry> backpatchQueue;

    bool prevInstructionGotoOrRet;

    void clear();
};

#endif