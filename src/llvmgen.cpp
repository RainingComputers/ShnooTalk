#include "llvmgen.hpp"

using namespace llvm;
using namespace sys;

namespace llvmgen
{
    icode::target_desc target_desc()
    {
        /* Target description for mikuro-llvm */
        icode::target_desc uhlltarget;

        uhlltarget.dtype_strings_map = { { "byte", icode::I8 },     { "ubyte", icode::UI8 },  { "short", icode::I16 },
                                         { "ushort", icode::UI16 }, { "int", icode::I32 },    { "uint", icode::UI32 },
                                         { "long", icode::I64 },    { "ulong", icode::UI64 }, { "float", icode::F32 },
                                         { "double", icode::F64 },  { "char", icode::UI8 } };

        /* true and false defines */
        icode::def true_def;
        true_def.dtype = icode::INT;
        true_def.val.integer = 1;

        icode::def false_def;
        false_def.dtype = icode::INT;
        false_def.val.integer = 0;

        uhlltarget.defines = { { "true", true_def }, { "false", false_def } };

        /* Default int or word */
        uhlltarget.default_int = icode::I32;
        uhlltarget.str_int = icode::UI8;

        return uhlltarget;
    }

    std::string get_format_string(icode::data_type dtype)
    {
        if (icode::is_uint(dtype))
            return "%u\n";

        if (icode::is_sint(dtype))
            return "%d\n";

        if (icode::is_float(dtype))
            return "%.2f\n";
    }

    Type* llvm_generator::to_llvm_type(icode::data_type dtype)
    {
        /* Converts mikuro ir icode::data_type to llvm type */

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

    Type* llvm_generator::to_llvm_ptr_type(icode::data_type dtype)
    {
        /* Converts mikuro ir icode::data_type to llvm pointer type */

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

    Type* llvm_generator::vinfo_to_llvm_type(const icode::var_info& var_info)
    {
        if (var_info.dimensions.size() > 0 || var_info.dtype == icode::STRUCT)
            return ArrayType::get(Type::getInt8Ty(*llvm_context), var_info.size);

        return to_llvm_type(var_info.dtype);
    }

    Value* llvm_generator::gen_ltrl(const icode::operand& op)
    {
        /* Convetrs mikuro icode::LITERAL operand type to llvm value  */

        if (icode::is_int(op.dtype))
            return ConstantInt::get(to_llvm_type(op.dtype), op.val.integer);

        if (icode::is_float(op.dtype))
            return ConstantFP::get(to_llvm_type(op.dtype), op.val.floating);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::gen_addr(const icode::operand& op)
    {
        /* Converts mikuro icode::ADDR operand to llvm value */

        return ConstantInt::get(Type::getInt64Ty(*llvm_context), op.val.integer);
    }

    void llvm_generator::local_symbol_alloca(icode::var_info& var_info, const std::string& name)
    {
        /* Creates llvm alloca instruction for icode::var_info symbols in symbol tables */

        alloca_inst_map[name] = llvm_builder->CreateAlloca(vinfo_to_llvm_type(var_info), nullptr, name);
    }

    void llvm_generator::global_symbol_alloca(icode::var_info& var_info, const std::string& name)
    {
        GlobalVariable* global_value;
        Type* global_type = vinfo_to_llvm_type(var_info);

        global_value =
          new GlobalVariable(*llvm_module, global_type, false, GlobalVariable::CommonLinkage, nullptr, name);

        global_value->setInitializer(Constant::getNullValue(global_type));

        llvm_global_map[name] = global_value;
    }

    Value* llvm_generator::get_llvm_alloca(const icode::operand& op)
    {
        /* Returns llvm value allocated by symbol_alloca */

        switch (op.optype)
        {
            case icode::VAR:
                return alloca_inst_map[op.name];
            case icode::GBL_VAR:
                return llvm_global_map[op.name];
            case icode::STR_DATA:
                return llvm_builder->CreateGlobalStringPtr(module.str_data[op.name]);
            default:
                break;
        }

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::get_llvm_value(const icode::operand& op)
    {
        /* Convertes mikuro icode::operand to llvm value */

        switch (op.optype)
        {
            case icode::LITERAL:
                return gen_ltrl(op);
            case icode::ADDR:
                return gen_addr(op);
            case icode::GBL_VAR:
                return llvm_builder->CreateLoad(llvm_global_map[op.name], op.name.c_str());
            case icode::VAR:
                return llvm_builder->CreateLoad(alloca_inst_map[op.name], op.name.c_str());
            case icode::TEMP_PTR:
            case icode::TEMP:
                return operand_value_map[op];
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    void llvm_generator::set_llvm_value(const icode::operand& op, Value* value)
    {
        /* Assigns llvm value to a mikuro icode::operand */

        switch (op.optype)
        {
            case icode::TEMP:
            case icode::TEMP_PTR:
                operand_value_map[op] = value;
                break;
            case icode::VAR:
            case icode::GBL_VAR:
                llvm_builder->CreateStore(value, get_llvm_alloca(op));
                break;
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    void llvm_generator::create_ptr(const icode::entry& e)
    {
        /* Converts mikuro CREATE_PTR to llvm ir */

        switch (e.op2.optype)
        {
            case icode::TEMP_PTR:
            case icode::PTR:
                operand_value_map[e.op1] = operand_value_map[e.op2];
                break;
            case icode::VAR:
            case icode::GBL_VAR:
                operand_value_map[e.op1] =
                  llvm_builder->CreatePtrToInt(get_llvm_alloca(e.op2), to_llvm_type(icode::I64));
                break;
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    void llvm_generator::eq(const icode::entry& e)
    {
        /* Converts mikuro EQUAL to llvm ir */

        Value* what_to_store = get_llvm_value(e.op2);
        set_llvm_value(e.op1, what_to_store);
    }

    void llvm_generator::read(const icode::entry& e)
    {
        /* Converts mikuro READ to llvm ir */

        Value* ptr = llvm_builder->CreateIntToPtr(get_llvm_value(e.op2), to_llvm_ptr_type(e.op1.dtype));
        Value* value = llvm_builder->CreateLoad(ptr);

        set_llvm_value(e.op1, value);
    }

    void llvm_generator::write(const icode::entry& e)
    {
        /* Converts mikuro WRITE to llvm ir */

        Value* where_to_store = llvm_builder->CreateIntToPtr(get_llvm_value(e.op1), to_llvm_ptr_type(e.op2.dtype));

        Value* what_to_store = get_llvm_value(e.op2);

        llvm_builder->CreateStore(what_to_store, where_to_store);
    }

    void llvm_generator::addrop(const icode::entry& e)
    {
        /* Converts mikuro ADDR_ADD and ADDR_MUL to llvm ir */

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

    Value* llvm_generator::add(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro ADD to llvm ir */

        if (icode::is_sint(dtype))
            return llvm_builder->CreateNSWAdd(LHS, RHS);

        if (icode::is_uint(dtype))
            return llvm_builder->CreateNUWAdd(LHS, RHS);

        if (icode::is_float(dtype))
            return llvm_builder->CreateFAdd(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::sub(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro SUB to llvm ir */

        if (icode::is_sint(dtype))
            return llvm_builder->CreateNSWSub(LHS, RHS);

        if (icode::is_uint(dtype))
            return llvm_builder->CreateNUWSub(LHS, RHS);

        if (icode::is_float(dtype))
            return llvm_builder->CreateFSub(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::mul(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro MUL to llvm ir */

        if (icode::is_sint(dtype))
            return llvm_builder->CreateNSWMul(LHS, RHS);

        if (icode::is_uint(dtype))
            return llvm_builder->CreateNUWMul(LHS, RHS);

        if (icode::is_float(dtype))
            return llvm_builder->CreateFMul(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::div(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro DIV to llvm ir */

        if (icode::is_sint(dtype))
            return llvm_builder->CreateSDiv(LHS, RHS);

        if (icode::is_uint(dtype))
            return llvm_builder->CreateUDiv(LHS, RHS);

        if (icode::is_float(dtype))
            return llvm_builder->CreateFDiv(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::mod(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro MOD to llvm ir */

        if (icode::is_sint(dtype))
            return llvm_builder->CreateSRem(LHS, RHS);

        if (icode::is_uint(dtype))
            return llvm_builder->CreateURem(LHS, RHS);

        if (icode::is_float(dtype))
            return llvm_builder->CreateFRem(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::rsh(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro RSH to llvm ir */

        if (icode::is_sint(dtype))
            return llvm_builder->CreateAShr(LHS, RHS);

        if (icode::is_uint(dtype))
            return llvm_builder->CreateLShr(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::lsh(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro LSH to llvm ir */

        if (icode::is_int(dtype))
            llvm_builder->CreateShl(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::bwa(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro BWA to llvm ir */

        if (icode::is_int(dtype))
            llvm_builder->CreateAnd(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::bwo(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro BWO to llvm ir */

        if (icode::is_int(dtype))
            llvm_builder->CreateOr(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::bwx(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro BWX to llvm ir */

        if (icode::is_int(dtype))
            llvm_builder->CreateXor(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    void llvm_generator::binop(const icode::entry& e)
    {
        Value* LHS = get_llvm_value(e.op2);
        Value* RHS = get_llvm_value(e.op3);
        icode::data_type dtype = e.op1.dtype;

        switch (e.opcode)
        {
            case icode::ADD:
                set_llvm_value(e.op1, add(LHS, RHS, dtype));
                break;
            case icode::SUB:
                set_llvm_value(e.op1, sub(LHS, RHS, dtype));
                break;
            case icode::MUL:
                set_llvm_value(e.op1, mul(LHS, RHS, dtype));
                break;
            case icode::DIV:
                set_llvm_value(e.op1, div(LHS, RHS, dtype));
                break;
            case icode::MOD:
                set_llvm_value(e.op1, mod(LHS, RHS, dtype));
                break;
            case icode::RSH:
                set_llvm_value(e.op1, rsh(LHS, RHS, dtype));
                break;
            case icode::LSH:
                set_llvm_value(e.op1, lsh(LHS, RHS, dtype));
                break;
            case icode::BWA:
                set_llvm_value(e.op1, bwa(LHS, RHS, dtype));
                break;
            case icode::BWO:
                set_llvm_value(e.op1, bwo(LHS, RHS, dtype));
                break;
            case icode::BWX:
                set_llvm_value(e.op1, bwx(LHS, RHS, dtype));
                break;
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    Value* llvm_generator::eq(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro EQ to llvm ir */

        if (icode::is_int(dtype))
            return llvm_builder->CreateICmpEQ(LHS, RHS);

        if (icode::is_float(dtype))
            return llvm_builder->CreateFCmpUEQ(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::neq(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro NEQ to llvm ir */

        if (icode::is_int(dtype))
            return llvm_builder->CreateICmpNE(LHS, RHS);

        if (icode::is_float(dtype))
            return llvm_builder->CreateFCmpUNE(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::lt(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro LT to llvm ir */

        if (icode::is_sint(dtype))
            return llvm_builder->CreateICmpSLT(LHS, RHS);

        if (icode::is_uint(dtype))
            return llvm_builder->CreateICmpULT(LHS, RHS);

        if (icode::is_float(dtype))
            return llvm_builder->CreateFCmpULT(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::lte(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro LTE to llvm ir */

        if (icode::is_sint(dtype))
            return llvm_builder->CreateICmpSLE(LHS, RHS);

        if (icode::is_uint(dtype))
            return llvm_builder->CreateICmpULE(LHS, RHS);

        if (icode::is_float(dtype))
            return llvm_builder->CreateFCmpULE(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::gt(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro GT to llvm ir */

        if (icode::is_sint(dtype))
            return llvm_builder->CreateICmpSGT(LHS, RHS);

        if (icode::is_uint(dtype))
            return llvm_builder->CreateICmpUGT(LHS, RHS);

        if (icode::is_float(dtype))
            return llvm_builder->CreateFCmpUGT(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::gte(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro GTE to llvm ir */

        if (icode::is_sint(dtype))
            return llvm_builder->CreateICmpSGE(LHS, RHS);

        if (icode::is_uint(dtype))
            return llvm_builder->CreateICmpUGE(LHS, RHS);

        if (icode::is_float(dtype))
            return llvm_builder->CreateFCmpUGE(LHS, RHS);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    void llvm_generator::cmpop(const icode::entry& e, size_t entry_idx)
    {
        Value* LHS = get_llvm_value(e.op1);
        Value* RHS = get_llvm_value(e.op2);
        icode::data_type dtype = e.op1.dtype;

        switch (e.opcode)
        {
            case icode::EQ:
                cmp_flag_q.push(eq(LHS, RHS, dtype));
                break;
            case icode::NEQ:
                cmp_flag_q.push(neq(LHS, RHS, dtype));
                break;
            case icode::LT:
                cmp_flag_q.push(lt(LHS, RHS, dtype));
                break;
            case icode::LTE:
                cmp_flag_q.push(lte(LHS, RHS, dtype));
                break;
            case icode::GT:
                cmp_flag_q.push(gt(LHS, RHS, dtype));
                break;
            case icode::GTE:
                cmp_flag_q.push(gte(LHS, RHS, dtype));
                break;
            default:
                miklog::internal_error(module.name);
                throw miklog::internal_bug_error();
        }
    }

    void llvm_generator::create_backpatch(const icode::entry& e, Function* F, size_t entry_idx)
    {
        /* All branch instructions GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO are backpatched,
            i.e the task is stores in a queue and instructions are create in second pass */

        /* Save llvm insertions point */
        BasicBlock* block = llvm_builder->GetInsertBlock();
        BasicBlock::iterator insert_point = llvm_builder->GetInsertPoint()++;
        backpatch_point_map[entry_idx] = llvm_bb_it_pair(block, insert_point);

        backpatch_entry_q.push_back(entry_idx_pair(entry_idx, e));

        if (e.opcode == icode::GOTO)
            return;

        /* Create basic block for fall through for IF_TRUE_GOTO, IF_FALSE_GOTO */
        BasicBlock* fall_block = BasicBlock::Create(*llvm_context, "_fall_e" + std::to_string(entry_idx), F);

        fall_block_map[entry_idx] = fall_block;

        /* Start inserstion in fall through block */
        llvm_builder->SetInsertPoint(fall_block);
    }

    void llvm_generator::create_label(const icode::entry& e, Function* F)
    {
        /* Converts mikuro CREATE_LABEL to llvm basic block  */
        BasicBlock* llvm_bb = BasicBlock::Create(*llvm_context, e.op1.name, F);

        label_block_map[e.op1] = llvm_bb;

        /* Make sure every block has a terminator */
        if (!prev_instr_branch)
            llvm_builder->CreateBr(llvm_bb);

        /* Start inserting ir into new block */
        llvm_builder->SetInsertPoint(llvm_bb);
    }

    void llvm_generator::print(const icode::entry& e)
    {
        Value* value = get_llvm_value(e.op1);

        /* Cast value to double if float */
        if (icode::is_float(e.op1.dtype))
            value = llvm_builder->CreateFPCast(value, Type::getDoubleTy(*llvm_context));
        else
            value = llvm_builder->CreateSExt(value, Type::getInt32Ty(*llvm_context));

        Value* format_str = llvm_builder->CreateGlobalStringPtr(get_format_string(e.op1.dtype));

        /* Set up printf arguments*/
        std::vector<Value*> printArgs;
        printArgs.push_back(format_str);
        printArgs.push_back(value);

        /* Call printf */
        llvm_builder->CreateCall(llvm_module->getFunction("printf"), printArgs);
    }

    void llvm_generator::print_str(const icode::entry& e)
    {
        Value* str_value = get_llvm_alloca(e.op1);

        /* Set up printf arguments*/
        std::vector<Value*> printArgs;
        printArgs.push_back(str_value);

        /* Call printf */
        llvm_builder->CreateCall(llvm_module->getFunction("printf"), printArgs);
    }

    void llvm_generator::create_symbols(const icode::func_desc& func_desc)
    {
        /* Go through the symbol table and create alloc instructions */
        for (auto symbol : func_desc.symbols)
            local_symbol_alloca(symbol.second, symbol.first);
    }

    void llvm_generator::gen_func_icode(const icode::func_desc& func_desc, Function* F)
    {
        /* Go through icode and generate llvm ir */
        for (size_t i = 0; i < func_desc.icode_table.size(); i++)
        {
            icode::entry e = func_desc.icode_table[i];

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
                case icode::PRINT_STR:
                    print_str(e);
                    break;
                case icode::RET:
                case icode::EXIT:
                    break;
                default:
                    miklog::internal_error(module.name);
                    throw miklog::internal_bug_error();
            }

            prev_instr_branch =
              e.opcode == icode::GOTO || e.opcode == icode::IF_TRUE_GOTO || e.opcode == icode::IF_FALSE_GOTO;
        }
    }

    void llvm_generator::process_goto_backpatch()
    {
        /* Go through backpatch queue */
        for (size_t i = 0; i < backpatch_entry_q.size(); i++)
        {
            /* Get the entry from backpatch q */
            entry_idx_pair& q = backpatch_entry_q[i];

            size_t entry_idx = q.first;
            icode::entry& e = q.second;

            /* Get branch flags and blocks for the goto */
            BasicBlock* goto_bb = label_block_map[e.op1];
            BasicBlock* fall_bb = fall_block_map[entry_idx];
            Value* goto_flag = cmp_flag_q.front();

            /* Get insertion point corresponding to the entry */
            llvm_bb_it_pair insert_point = backpatch_point_map[entry_idx];
            llvm_builder->SetInsertPoint(insert_point.first, insert_point.second);

            /* Convert mikuro GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO int llvm ir  */
            switch (e.opcode)
            {
                case icode::GOTO:
                    llvm_builder->CreateBr(goto_bb);
                    break;
                case icode::IF_TRUE_GOTO:
                    cmp_flag_q.pop();
                    llvm_builder->CreateCondBr(goto_flag, goto_bb, fall_bb);
                    break;
                case icode::IF_FALSE_GOTO:
                    cmp_flag_q.pop();
                    llvm_builder->CreateCondBr(goto_flag, fall_bb, goto_bb);
                    break;
                default:
                    miklog::internal_error(module.name);
                    throw miklog::internal_bug_error();
            }
        }
    }

    void llvm_generator::gen_function(const icode::func_desc& func_desc, const std::string& name)
    {
        /* Setup llvm function */
        std::vector<Type*> types;
        FunctionType* FT = FunctionType::get(Type::getVoidTy(*llvm_context), types, false);

        Function* F = Function::Create(FT, Function::ExternalLinkage, name, llvm_module.get());

        BasicBlock* BB = BasicBlock::Create(*llvm_context, "entry", F);
        llvm_builder->SetInsertPoint(BB);

        /* Allocate stack space for local variables */
        create_symbols(func_desc);

        /* Convert mikuro function ir to llvm ir */
        gen_func_icode(func_desc, F);

        /* Terminate function */
        llvm_builder->CreateRetVoid();

        /* Process goto backpathing */
        process_goto_backpatch();

        verifyFunction(*F);
    }

    void llvm_generator::gen_globals()
    {
        for (auto symbol : module.globals)
            global_symbol_alloca(symbol.second, symbol.first);
    }

    void llvm_generator::setup_printf()
    {
        std::vector<Type*> args;
        args.push_back(Type::getInt8PtrTy(*llvm_context));
        FunctionType* printf_type = FunctionType::get(llvm_builder->getInt32Ty(), args, true);
        Function::Create(printf_type, Function::ExternalLinkage, "printf", llvm_module.get());
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

        /* Generate global variables */
        gen_globals();

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
        auto target_machine = Target->createTargetMachine(target_triple, CPU, features, opt, RM);

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