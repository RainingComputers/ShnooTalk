#ifndef TRANSLATOR_BRANCH_CONTEXT
#define TRANSLATOR_BRANCH_CONTEXT

#include <map>
#include <queue>
#include <vector>

#include "llvm/IR/BasicBlock.h"

#include "../IntermediateRepresentation/All.hpp"

typedef std::pair<llvm::BasicBlock*, llvm::BasicBlock::iterator> InsertionPoint;
typedef std::pair<size_t, icode::Entry> EnumeratedEntry;

struct BranchContext
{
    std::map<std::string, llvm::BasicBlock*> labelToBasicBlockMap;
    std::map<size_t, llvm::BasicBlock*> fallBlocks;
    std::queue<llvm::Value*> branchFlags;
    std::map<size_t, InsertionPoint> insertionPoints;
    std::vector<EnumeratedEntry> backpatchQueue;

    bool prevInstructionGotoOrRet;

    void clear();
};

#endif