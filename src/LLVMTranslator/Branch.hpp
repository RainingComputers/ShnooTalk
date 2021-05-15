#ifndef BRANCH_HPP
#define BRANCH_HPP

#include "BranchContext.hpp"
#include "ModuleContext.hpp"

void createLabel(const ModuleContext& ctx,
                 BranchContext& branchContext,
                 const icode::Entry& e,
                 llvm::Function* function);

void createGotoBackpatch(const ModuleContext& ctx,
                         BranchContext& branchContext,
                         const icode::Entry& e,
                         llvm::Function* F,
                         size_t entryIndex);

void createBranch(const ModuleContext& ctx,
                  const icode::Entry& e,
                  llvm::Value* flag,
                  llvm::BasicBlock* gotoBlock,
                  llvm::BasicBlock* fallBlock);

void processGotoBackpatches(const ModuleContext& ctx, BranchContext& branchContext);

#endif