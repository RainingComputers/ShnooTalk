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

    Value* llvm_generator::gen_ltrl(icode::operand& op)
    {
        Value* llvm_value;

        switch (op.dtype)
        {
            case icode::I32:
            case icode::UI32:
            case icode::INT:
                llvm_value = ConstantInt::get(Type::getInt32Ty(*llvm_context), op.val.integer);
                break;
            case icode::I8:
                llvm_value = ConstantInt::get(Type::getInt8Ty(*llvm_context), op.val.integer);
                break;
            case icode::F32:
            case icode::FLOAT:
                llvm_value = ConstantFP::get(Type::getFloatTy(*llvm_context), op.val.floating);
                break;
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }

        return llvm_value;
    }

    Value* llvm_generator::gen_addr(icode::operand& op)
    {
        return ConstantInt::get(Type::getInt32Ty(*llvm_context), op.val.integer);
    }

    Value* llvm_generator::get_llvm_value(icode::operand& op, bool write)
    {
        switch (op.optype)
        {
            case icode::LITERAL: 
                return gen_ltrl(op);
            case icode::ADDR: 
                return gen_addr(op);
            case icode::VAR:
            {
                AllocaInst* alloca_value = alloca_inst_map[op.name];

                if(write) return alloca_value;
                else return llvm_builder->CreateLoad(alloca_value, op.name.c_str());
            }
            case icode::TEMP:
                return operand_value_map[op];
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    void llvm_generator::eq(icode::entry& e)
    {
        Value* what_to_store = get_llvm_value(e.op2);
        Value* where_to_Store = get_llvm_value(e.op1, true);

        llvm_builder->CreateStore(what_to_store, where_to_Store);
    }

    void llvm_generator::binop(icode::entry& e)
    {
        Value* result;
        Value* LHS = get_llvm_value(e.op2);
        Value* RHS = get_llvm_value(e.op3);

        switch (e.opcode)
        {
            case icode::ADD:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                        result = llvm_builder->CreateNSWAdd(LHS, RHS);
                        break;
                    case icode::UI32:
                        result = llvm_builder->CreateNUWAdd(LHS, RHS);
                        break;
                    case icode::FLOAT:
                    case icode::F32:
                        result = llvm_builder->CreateFAdd(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            case icode::SUB:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                        result = llvm_builder->CreateNSWSub(LHS, RHS);
                        break;
                    case icode::UI32:
                        result = llvm_builder->CreateNUWSub(LHS, RHS);
                        break;
                    case icode::FLOAT:
                    case icode::F32:
                        result = llvm_builder->CreateFSub(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            case icode::MUL:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                        result = llvm_builder->CreateNSWMul(LHS, RHS);
                        break;
                    case icode::UI32:
                        result = llvm_builder->CreateNUWMul(LHS, RHS);
                        break;
                    case icode::FLOAT:
                    case icode::F32:
                        result = llvm_builder->CreateFMul(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            case icode::DIV:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                        result = llvm_builder->CreateSDiv(LHS, RHS);
                        break;
                    case icode::UI32:
                        result = llvm_builder->CreateUDiv(LHS, RHS);
                        break;
                    case icode::FLOAT:
                    case icode::F32:
                        result = llvm_builder->CreateFDiv(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            case icode::MOD:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                        result = llvm_builder->CreateSRem(LHS, RHS);
                        break;
                    case icode::UI32:
                        result = llvm_builder->CreateURem(LHS, RHS);
                        break;
                    case icode::FLOAT:
                    case icode::F32:
                        result = llvm_builder->CreateFRem(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            case icode::RSH:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                        result = llvm_builder->CreateAShr(LHS, RHS);
                        break;
                    case icode::UI32:
                        result = llvm_builder->CreateLShr(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            case icode::LSH:
                result = llvm_builder->CreateShl(LHS, RHS);
                break;
            case icode::BWA:
                result = llvm_builder->CreateAnd(LHS, RHS);
                break;
            case icode::BWO:
                result = llvm_builder->CreateOr(LHS, RHS);
                break;
            case icode::BWX:
                result = llvm_builder->CreateXor(LHS, RHS);
                break;
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }

        /* Store result llvm in map so it can be used by other llvm trnaslations */
        operand_value_map[e.op1] = result;

    }

    void llvm_generator::symbol_alloca(icode::var_info& var_info, const std::string& name)
    {
        AllocaInst* alloca_inst;

        switch (var_info.dtype)
        {
            case icode::I32:
            case icode::UI32:
                alloca_inst = llvm_builder->CreateAlloca(
                  Type::getInt32Ty(*llvm_context), nullptr, name);
                break;
            case icode::I8:
                alloca_inst = llvm_builder->CreateAlloca(
                  Type::getInt8Ty(*llvm_context), nullptr, name);
                break;
            case icode::F32:
                alloca_inst = llvm_builder->CreateAlloca(
                  Type::getFloatTy(*llvm_context), nullptr, name);
                break;
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
                
        }

        alloca_inst_map[name] = alloca_inst;
    }

    void llvm_generator::gen_function(icode::func_desc& func_desc, const std::string& name)
    {
        /* Setup llvm function */
        std::vector<Type*> types;
        FunctionType* FT =
          FunctionType::get(Type::getVoidTy(*llvm_context), types, false);

        Function* F =
          Function::Create(FT, Function::ExternalLinkage, name, llvm_module.get());

        BasicBlock* BB = BasicBlock::Create(*llvm_context, "entry", F);
        llvm_builder->SetInsertPoint(BB);

        /* Go through the symbol table and create alloc instructions */
        for (auto symbol : func_desc.symbols)
        {
            symbol_alloca(symbol.second, symbol.first);
        }

        /* Go through icode and generate llvm ir */
        for (icode::entry e : func_desc.icode_table)
        {
            switch (e.opcode)
            {
                case icode::EQUAL:
                    eq(e);
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
                    binop(e);
                    break;
                case icode::RET:
                case icode::EXIT:
                    break;
                default:
                    miklog::internal_error(module.name);
                    throw miklog::internal_bug_error();
            }
        }
        

        /* Terminate function */
        llvm_builder->CreateRetVoid();
        verifyFunction(*F);
    }

    llvm_generator::llvm_generator(icode::module_desc& module_desc) : module(module_desc)
    {  
        /* Setup LLVM context, module and builder */
        llvm_context = std::make_unique<LLVMContext>();
        llvm_module = std::make_unique<Module>(module.name, *llvm_context);
        llvm_builder = std::make_unique<IRBuilder<>>(*llvm_context);

        /* Loop through each function and convert mikuro IR to llvm IR */
        for (auto func : module.functions)
        {
            gen_function(func.second, func.first);
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