#include "llvmgen.hpp"

using namespace llvm;
using namespace llvm::sys;

namespace llvmgen
{
    icode::target_desc target_desc()
    {
        /* Target descroption for uhllvm */
        icode::target_desc uhlltarget;

        uhlltarget.dtype_strings_map = { { "int", icode::I32 },
                                         { "bool", icode::I8 },
                                         { "float", icode::F32 },
                                         { "uint", icode::UI32 } };

        /* true and false defines */
        icode::def true_def;
        true_def.dtype = icode::INT;
        true_def.val.integer = 1;

        icode::def false_def;
        false_def.dtype = icode::INT;
        false_def.val.integer = 0;

        uhlltarget.defines = { { "true", true_def }, { "false", false_def } };

        /* default int or word */
        uhlltarget.default_int = icode::I32;

        return uhlltarget;
    }

    void llvm_generator::symbol_alloca(icode::var_info& var_info, const std::string& name)
    {
        AllocaInst* alloca_inst;

        switch (var_info.dtype)
        {
            case icode::I32:
                alloca_inst = llvm_builder->CreateAlloca(
                  Type::getInt32Ty(*llvm_context), nullptr, name);
                break;
            case icode::F32:
                alloca_inst = llvm_builder->CreateAlloca(
                  Type::getFloatTy(*llvm_context), nullptr, name);
                break;
            default:
                break;
        }

        alloca_inst_map[name] = alloca_inst;
    }

    void llvm_generator::gen_module(icode::module_desc& module)
    {
        /* Setup LLVM context, module and builder */
        llvm_context = std::make_unique<LLVMContext>();
        llvm_module = std::make_unique<Module>(module.name, *llvm_context);
        llvm_builder = std::make_unique<IRBuilder<>>(*llvm_context);

        /* Loop through each function and convert mikuro IR to llvm IR */
        for (auto func : module.functions)
        {
            /* Setup llvm function */
            std::vector<Type*> types;
            FunctionType* FT =
              FunctionType::get(Type::getVoidTy(*llvm_context), types, false);

            Function* F = Function::Create(
              FT, Function::ExternalLinkage, func.first, llvm_module.get());

            BasicBlock *BB = BasicBlock::Create(*llvm_context, "entry", F);
              llvm_builder->SetInsertPoint(BB);

            /* Go through the symbol table and create alloc instructions */
            for (auto symbol : func.second.symbols)
            {
                symbol_alloca(symbol.second, symbol.first);
            }

            /* Terminate function */
            llvm_builder->CreateRetVoid();
            verifyFunction(*F);
        }

        errs() << *llvm_module;

        /* Initialize the target registry etc */
        InitializeAllTargetInfos();
        InitializeAllTargets();
        InitializeAllTargetMCs();
        InitializeAllAsmParsers();
        InitializeAllAsmPrinters();

        /* Setup LLVM target triple */
        auto target_triple = sys::getDefaultTargetTriple();
        llvm_module->setTargetTriple(target_triple);

        std::string error;
        auto Target = TargetRegistry::lookupTarget(target_triple, error);

        if (!Target)
        {
            miklog::println(error);
            throw miklog::internal_bug_error();
        }

        /* Setup LLVM data layout */
        std::string CPU = "generic";
        std::string features = "";

        TargetOptions opt;
        auto RM = Optional<Reloc::Model>();
        auto target_machine =
          Target->createTargetMachine(target_triple, CPU, features, opt, RM);

        llvm_module->setDataLayout(target_machine->createDataLayout());

        /* Setup output object file and pass manager */
        auto filename = module.name + ".o";
        std::error_code EC;
        raw_fd_ostream dest(filename, EC, sys::fs::OF_None);

        if (EC)
        {
            // TODO: change this
            miklog::println("Could not open file: " + EC.message());
            throw miklog::internal_bug_error();
        }

        legacy::PassManager pass;
        auto FileType = CGFT_ObjectFile;

        if (target_machine->addPassesToEmitFile(pass, dest, nullptr, FileType))
        {
            miklog::println("LLVM target machine can't emit a file of this type");
            throw miklog::internal_bug_error();
        }

        pass.run(*llvm_module);
        dest.flush();
    }
}