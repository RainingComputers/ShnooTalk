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
        std::map<icode::operand, llvm::Value*> operand_value_map;

        icode::module_desc& module;

        llvm::Value* gen_ltrl(icode::operand& op);
        llvm::Value* gen_addr(icode::operand& op);
        llvm::Value* get_llvm_value(icode::operand& op, bool write = false);
        void symbol_alloca(icode::var_info& var_info, const std::string& name);
        
        void eq(icode::entry& e);
        void binop(icode::entry& e);

        void gen_function(icode::func_desc& func, const std::string& name);

      public:
        llvm_generator(icode::module_desc& module_desc);
    };

}

#endif