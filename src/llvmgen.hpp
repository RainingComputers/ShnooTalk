#ifndef LLVMGEN_HPP
#define LLVMGEN_HPP

#include <map>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "icode.hpp"
#include "log.hpp"

namespace llvmgen
{
    icode::target_desc target_desc();

    class llvm_generator
    {
        std::unique_ptr<llvm::LLVMContext> llvm_context;
        std::unique_ptr<llvm::Module> llvm_module;
        std::unique_ptr<llvm::IRBuilder<>> llvm_builder;

        std::map<std::string, llvm::AllocaInst*> alloca_inst_map;
        std::map<std::string, llvm::Function*> llvm_function_map;

        void symbol_alloca(icode::var_info& var_info, const std::string& name);

      public:
        void gen_module(icode::module_desc& module);
    };

}

#endif