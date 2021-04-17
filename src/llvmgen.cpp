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
                                         { "uint", icode::UI32 },
                                         { "long", icode::I64 },
                                         { "bool", icode::I8 },
                                         { "float", icode::F32 } };

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

    std::string get_format_string(icode::data_type dtype)
    {
        if (icode::is_uint(dtype))
            return "%u\n";
        else if (icode::is_int(dtype))
            return "%d\n";
        else if (icode::is_float(dtype))
            return "%.2f\n";
    }

    Type* llvm_generator::to_llvm_type(icode::data_type dtype)
    {
        switch (dtype)
        {
            case icode::I8:
            case icode::UI8:
                return Type::getInt8Ty(*llvm_context);
            case icode::I16:
            case icode::UI16:
                return Type::getInt16Ty(*llvm_context);
            case icode::I32:
            case icode::UI32:
            case icode::INT:
                return Type::getInt32Ty(*llvm_context);
            case icode::I64:
            case icode::UI64:
                return Type::getInt64Ty(*llvm_context);
            case icode::F32:
                return Type::getFloatTy(*llvm_context);
            case icode::F64:
                return Type::getDoubleTy(*llvm_context);
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    llvm::Type* llvm_generator::to_llvm_ptr_type(icode::data_type dtype)
    {
        switch (dtype)
        {
            case icode::I8:
            case icode::UI8:
                return Type::getInt8PtrTy(*llvm_context);
            case icode::I16:
            case icode::UI16:
                return Type::getInt16PtrTy(*llvm_context);
            case icode::I32:
            case icode::UI32:
            case icode::INT:
                return Type::getInt32PtrTy(*llvm_context);
            case icode::I64:
            case icode::UI64:
                return Type::getInt64PtrTy(*llvm_context);
            case icode::F32:
                return Type::getFloatPtrTy(*llvm_context);
            case icode::F64:
                return Type::getDoublePtrTy(*llvm_context);
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    Value* llvm_generator::gen_ltrl(const icode::operand& op)
    {
        Value* llvm_value;

        switch (op.dtype)
        {
            case icode::I32:
            case icode::UI32:
            case icode::I8:
            case icode::INT:
                llvm_value = ConstantInt::get(to_llvm_type(op.dtype), op.val.integer);
                break;
            case icode::F32:
                llvm_value = ConstantFP::get(to_llvm_type(op.dtype), op.val.floating);
                break;
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }

        return llvm_value;
    }

    Value* llvm_generator::gen_addr(const icode::operand& op)
    {
        return ConstantInt::get(Type::getInt64Ty(*llvm_context), op.val.integer);
    }

    void llvm_generator::symbol_alloca(icode::var_info& var_info, const std::string& name)
    {
        AllocaInst* alloca_inst;

        if (var_info.dimensions.size() > 0 || var_info.dtype == icode::STRUCT)
        {
            alloca_inst = llvm_builder->CreateAlloca(
              ArrayType::get(Type::getInt8Ty(*llvm_context), var_info.size),
              nullptr,
              name);
        }
        else
        {
            alloca_inst =
              llvm_builder->CreateAlloca(to_llvm_type(var_info.dtype), nullptr, name);
        }

        alloca_inst_map[name] = alloca_inst;
    }

    Value* llvm_generator::get_llvm_alloca(const icode::operand& op)
    {
        switch (op.optype)
        {
            case icode::VAR:
                return alloca_inst_map[op.name];
                break;
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    Value* llvm_generator::get_llvm_value(const icode::operand& op)
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
                return llvm_builder->CreateLoad(alloca_value, op.name.c_str());
            }
            case icode::TEMP_PTR:
            case icode::TEMP:
                return operand_value_map[op];
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    void llvm_generator::set_llvm_value(const icode::operand& op, llvm::Value* value)
    {
        switch (op.optype)
        {
            case icode::TEMP:
            case icode::TEMP_PTR:
                operand_value_map[op] = value;
                break;
            case icode::VAR:
            case icode::GBL_VAR:
            {
                Value* where_to_store = get_llvm_alloca(op);
                llvm_builder->CreateStore(value, where_to_store);
                break;
            }
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    void llvm_generator::create_ptr(const icode::entry& e)
    {
        switch (e.op2.optype)
        {
            case icode::TEMP_PTR:
            case icode::PTR:
            {
                operand_value_map[e.op1] = operand_value_map[e.op2];
                break;
            }
            case icode::VAR:
            case icode::GBL_VAR:
            {
                Value* alloca = get_llvm_alloca(e.op2);
                operand_value_map[e.op1] =
                  llvm_builder->CreatePtrToInt(alloca, to_llvm_type(icode::I64));
                break;
            }
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    void llvm_generator::eq(const icode::entry& e)
    {
        Value* what_to_store = get_llvm_value(e.op2);
        set_llvm_value(e.op1, what_to_store);
    }

    void llvm_generator::read(const icode::entry& e)
    {
        Value* ptr = llvm_builder->CreateIntToPtr(get_llvm_value(e.op2),
                                                  to_llvm_ptr_type(e.op1.dtype));
        Value* value = llvm_builder->CreateLoad(ptr);

        set_llvm_value(e.op1, value);
    }

    void llvm_generator::write(const icode::entry& e)
    {
        Value* where_to_store = llvm_builder->CreateIntToPtr(
          get_llvm_value(e.op1), to_llvm_ptr_type(e.op2.dtype));
        Value* what_to_store = get_llvm_value(e.op2);
        llvm_builder->CreateStore(what_to_store, where_to_store);
    }

    void llvm_generator::addrop(const icode::entry& e)
    {
        Value* result;
        Value* LHS = get_llvm_value(e.op2);
        Value* RHS = get_llvm_value(e.op3);

        switch (e.opcode)
        {
            case icode::ADDR_ADD:
                result = llvm_builder->CreateNUWAdd(LHS, RHS);
                break;
            case icode::ADDR_MUL:
                result = llvm_builder->CreateNUWMul(LHS, RHS);
                break;
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }

        /* Store result llvm in map so it can be used by other llvm trnaslations */
        set_llvm_value(e.op1, result);
    }

    void llvm_generator::binop(const icode::entry& e)
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
        set_llvm_value(e.op1, result);
    }

    void llvm_generator::cmpop(const icode::entry& e, size_t entry_idx)
    {
        Value* result;
        Value* LHS = get_llvm_value(e.op1);
        Value* RHS = get_llvm_value(e.op2);

        switch (e.opcode)
        {
            case icode::EQ:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                    case icode::UI32:
                        result = llvm_builder->CreateICmpEQ(LHS, RHS);
                        break;
                    case icode::FLOAT:
                    case icode::F32:
                        result = llvm_builder->CreateFCmpUEQ(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            case icode::NEQ:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                    case icode::UI32:
                        result = llvm_builder->CreateICmpNE(LHS, RHS);
                        break;
                    case icode::FLOAT:
                    case icode::F32:
                        result = llvm_builder->CreateFCmpUNE(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            case icode::LT:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                        result = llvm_builder->CreateICmpSLT(LHS, RHS);
                        break;
                    case icode::UI32:
                        result = llvm_builder->CreateICmpULT(LHS, RHS);
                        break;
                    case icode::FLOAT:
                    case icode::F32:
                        result = llvm_builder->CreateFCmpULT(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            case icode::LTE:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                        result = llvm_builder->CreateICmpSLE(LHS, RHS);
                        break;
                    case icode::UI32:
                        result = llvm_builder->CreateICmpULE(LHS, RHS);
                        break;
                    case icode::FLOAT:
                    case icode::F32:
                        result = llvm_builder->CreateFCmpULE(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            case icode::GT:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                        result = llvm_builder->CreateICmpSGT(LHS, RHS);
                        break;
                    case icode::UI32:
                        result = llvm_builder->CreateICmpUGT(LHS, RHS);
                        break;
                    case icode::FLOAT:
                    case icode::F32:
                        result = llvm_builder->CreateFCmpUGT(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            case icode::GTE:
            {
                switch (e.op1.dtype)
                {
                    case icode::I8:
                    case icode::I32:
                    case icode::INT:
                        result = llvm_builder->CreateICmpSGE(LHS, RHS);
                        break;
                    case icode::UI32:
                        result = llvm_builder->CreateICmpUGE(LHS, RHS);
                        break;
                    case icode::FLOAT:
                    case icode::F32:
                        result = llvm_builder->CreateFCmpUGE(LHS, RHS);
                        break;
                    default:
                        miklog::internal_error(module.name);
                        throw miklog::internal_bug_error();
                }
                break;
            }
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }

        /* Store result llvm in map so it can be used by goto/jump llvm trnaslations */
        cmp_flag_q.push(result);
    }

    void
    llvm_generator::create_backpatch(const icode::entry& e, Function* F, size_t entry_idx)
    {
        BasicBlock* block = llvm_builder->GetInsertBlock();
        BasicBlock::iterator insert_point = llvm_builder->GetInsertPoint()++;

        backpatch_point_map[entry_idx] = llvm_bb_it_pair(block, insert_point);

        backpatch_entry_q.push_back(entry_idx_pair(entry_idx, e));

        if (e.opcode == icode::GOTO)
            return;

        BasicBlock* fall_block =
          BasicBlock::Create(*llvm_context, "_fall_e" + std::to_string(entry_idx), F);

        fall_block_map[entry_idx] = fall_block;

        llvm_builder->SetInsertPoint(fall_block);
    }

    void llvm_generator::create_label(const icode::entry& e, Function* F)
    {
        BasicBlock* llvm_bb = BasicBlock::Create(*llvm_context, e.op1.name, F);

        label_block_map[e.op1] = llvm_bb;

        if (!prev_instr_branch)
            llvm_builder->CreateBr(llvm_bb);

        llvm_builder->SetInsertPoint(llvm_bb);
    }

    void llvm_generator::print(icode::entry& e)
    {
        Value* value = get_llvm_value(e.op1);

        /* Cast value to double if float */
        if (icode::is_float(e.op1.dtype))
            value = llvm_builder->CreateFPCast(value, Type::getDoubleTy(*llvm_context));
        else
            value = llvm_builder->CreateSExt(value, Type::getInt32Ty(*llvm_context));

        Value* format_str =
          llvm_builder->CreateGlobalStringPtr(get_format_string(e.op1.dtype));

        /* Set up printf arguments*/
        std::vector<Value*> printArgs;
        printArgs.push_back(format_str);
        printArgs.push_back(value);

        /* Call printf */
        llvm_builder->CreateCall(llvm_module->getFunction("printf"), printArgs);
    }

    void
    llvm_generator::gen_function(icode::func_desc& func_desc, const std::string& name)
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
        for (size_t i = 0; i < func_desc.icode_table.size(); i++)
        {
            icode::entry& e = func_desc.icode_table[i];

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
                case icode::EQ:
                case icode::NEQ:
                case icode::LT:
                case icode::LTE:
                case icode::GT:
                case icode::GTE:
                    cmpop(e, i);
                    break;
                case icode::CREATE_LABEL:
                    create_label(e, F);
                    break;
                case icode::IF_TRUE_GOTO:
                case icode::IF_FALSE_GOTO:
                case icode::GOTO:
                    create_backpatch(e, F, i);
                    break;
                case icode::CREATE_PTR:
                    create_ptr(e);
                    break;
                case icode::ADDR_ADD:
                case icode::ADDR_MUL:
                    addrop(e);
                    break;
                case icode::READ:
                    read(e);
                    break;
                case icode::WRITE:
                    write(e);
                    break;
                case icode::PRINT:
                    print(e);
                    break;
                case icode::RET:
                case icode::EXIT:
                    break;
                default:
                    miklog::internal_error(module.name);
                    throw miklog::internal_bug_error();
            }

            prev_instr_branch = e.opcode == icode::GOTO ||
                                e.opcode == icode::IF_TRUE_GOTO ||
                                e.opcode == icode::IF_FALSE_GOTO;
        }

        /* Terminate function */
        llvm_builder->CreateRetVoid();
        verifyFunction(*F);

        /* Go through backpatch queue */
        for (size_t i = 0; i < backpatch_entry_q.size(); i++)
        {
            entry_idx_pair& q = backpatch_entry_q[i];

            size_t entry_idx = q.first;
            icode::entry& e = q.second;

            BasicBlock* goto_bb = label_block_map[e.op1];
            BasicBlock* fall_bb = fall_block_map[entry_idx];
            llvm_bb_it_pair insert_point = backpatch_point_map[entry_idx];
            Value* goto_flag = cmp_flag_q.front();

            llvm_builder->SetInsertPoint(insert_point.first, insert_point.second);

            switch (e.opcode)
            {
                case icode::GOTO:
                    llvm_builder->CreateBr(goto_bb);
                    break;
                case icode::IF_TRUE_GOTO:
                {
                    cmp_flag_q.pop();
                    llvm_builder->CreateCondBr(goto_flag, goto_bb, fall_bb);
                    break;
                }
                case icode::IF_FALSE_GOTO:
                {
                    cmp_flag_q.pop();
                    llvm_builder->CreateCondBr(goto_flag, fall_bb, goto_bb);
                    break;
                }
                default:
                    miklog::internal_error(module.name);
                    throw miklog::internal_bug_error();
            }
        }
    }

    void llvm_generator::setup_printf()
    {
        std::vector<Type*> args;
        args.push_back(Type::getInt8PtrTy(*llvm_context));
        FunctionType* printf_type =
          FunctionType::get(llvm_builder->getInt32Ty(), args, true);
        Function::Create(
          printf_type, Function::ExternalLinkage, "printf", llvm_module.get());
    }

    llvm_generator::llvm_generator(icode::module_desc& module_desc)
      : module(module_desc)
    {
        /* Setup LLVM context, module and builder */
        llvm_context = std::make_unique<LLVMContext>();
        llvm_module = std::make_unique<Module>(module.name, *llvm_context);
        llvm_builder = std::make_unique<IRBuilder<>>(*llvm_context);
        prev_instr_branch = false;

        /* Declare that printf exists and has signature int (i8*, ...) */
        setup_printf();

        /* Loop through each function and convert mikuro IR to llvm IR */
        for (auto func : module.functions)
        {
            gen_function(func.second, func.first);
        }

        outs() << *llvm_module;

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