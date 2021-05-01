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
            case icode::FLOAT:
                return Type::getFloatTy(*llvm_context);
            case icode::F64:
                return Type::getDoubleTy(*llvm_context);
            case icode::VOID:
                return Type::getVoidTy(*llvm_context);
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
        if (var_info.check(icode::IS_PTR))
            return to_llvm_ptr_type(var_info.dtype);

        if (var_info.dimensions.size() > 0 || var_info.dtype == icode::STRUCT)
            return ArrayType::get(Type::getInt8Ty(*llvm_context), var_info.size);

        return to_llvm_type(var_info.dtype);
    }

    FunctionType* llvm_generator::fdesc_to_llvm_type(icode::func_desc& func_desc)
    {
        std::vector<Type*> arg_types;

        /* Set the types vector */
        for (std::string param : func_desc.params)
        {
            Type* param_type = vinfo_to_llvm_type(func_desc.symbols[param]);
            arg_types.push_back(param_type);
        }

        /* Setup llvm function */
        FunctionType* FT = FunctionType::get(vinfo_to_llvm_type(func_desc.func_info), arg_types, false);

        return FT;
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

    void llvm_generator::local_symbol_alloca(const icode::var_info& var_info, const std::string& name)
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

    void llvm_generator::param_symbol_alloca(const icode::var_info& var_info, const std::string& name, Value* arg)
    {
        /* If mutable parameters, stores the pointer as int, else allocate space and stores arg */

        if (!var_info.check(icode::IS_PTR))
        {
            Value* alloca = llvm_builder->CreateAlloca(vinfo_to_llvm_type(var_info), nullptr, name);
            llvm_builder->CreateStore(arg, alloca);
            alloca_inst_map[name] = alloca;
        }
        else
        {
            alloca_inst_map[name] = arg;
            ptr_val_map[name] = llvm_builder->CreatePtrToInt(arg, to_llvm_type(icode::I64));
        }
    }

    Value* llvm_generator::get_llvm_alloca(const icode::operand& op)
    {
        /* Returns llvm value allocated by symbol_alloca */

        switch (op.optype)
        {
            case icode::PTR:
            case icode::VAR:
                return alloca_inst_map[op.name];
            case icode::GBL_VAR:
                return llvm_global_map[op.name];
            case icode::STR_DATA:
                return llvm_builder->CreateGlobalStringPtr(module.str_data[op.name]);
            case icode::RET_PTR:
                return current_ret_value;
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
                return llvm_builder->CreateLoad(alloca_inst_map.at(op.name), op.name.c_str());
            case icode::TEMP_PTR:
            case icode::TEMP:
            case icode::RET_VAL:
                return operand_value_map[op];
            case icode::RET_PTR:
                return llvm_builder->CreatePtrToInt(current_ret_value, to_llvm_type(icode::I64));
            case icode::PTR:
                return ptr_val_map[op.name];
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
            case icode::RET_VAL:
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

    Value* llvm_generator::get_ret_val_ptr(const icode::operand& op)
    {
        Value* ret_val = operand_value_map[op];
        Value* ret_val_ptr = llvm_builder->CreateAlloca(ret_val->getType());
        llvm_builder->CreateStore(ret_val, ret_val_ptr);

        return ret_val_ptr;
    }

    void llvm_generator::create_ptr(const icode::entry& e)
    {
        /* Converts mikuro CREATE_PTR to llvm ir */

        switch (e.op2.optype)
        {
            case icode::TEMP_PTR:
                operand_value_map[e.op1] = operand_value_map[e.op2];
                break;
            case icode::VAR:
            case icode::GBL_VAR:
                operand_value_map[e.op1] =
                  llvm_builder->CreatePtrToInt(get_llvm_alloca(e.op2), to_llvm_type(icode::I64));
                break;
            case icode::PTR:
                operand_value_map[e.op1] = alloca_inst_map[e.op2.name];
                break;
            case icode::RET_VAL:
                operand_value_map[e.op1] = get_ret_val_ptr(e.op2);
                break;
            case icode::RET_PTR:
                operand_value_map[e.op1] = current_ret_value;
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

    Value* llvm_generator::ensure_i64(Value* value)
    {
        if (value->getType() == to_llvm_type(icode::I64))
            return value;

        return llvm_builder->CreateZExtOrTrunc(value, to_llvm_type(icode::I64));
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
                result = llvm_builder->CreateNUWMul(ensure_i64(LHS), RHS);
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

    void llvm_generator::bwnot(const icode::entry& e)
    {
        Value* result = llvm_builder->CreateNot(get_llvm_value(e.op2));

        set_llvm_value(e.op1, result);
    }

    void llvm_generator::minus(const icode::entry& e)
    {
        Value* result = llvm_builder->CreateNeg(get_llvm_value(e.op2));

        set_llvm_value(e.op1, result);
    }

    Value* llvm_generator::cast_to_sint(const icode::entry& e, Type* destination_type)
    {
        if (icode::is_sint(e.op2.dtype))
            return llvm_builder->CreateSExtOrTrunc(get_llvm_value(e.op2), destination_type);

        if (icode::is_uint(e.op2.dtype))
            return llvm_builder->CreateZExtOrTrunc(get_llvm_value(e.op2), destination_type);

        if (icode::is_float(e.op2.dtype))
            return llvm_builder->CreateFPToSI(get_llvm_value(e.op2), destination_type);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::cast_to_uint(const icode::entry& e, Type* destination_type)
    {
        if (icode::is_int(e.op2.dtype))
            return llvm_builder->CreateZExtOrTrunc(get_llvm_value(e.op2), destination_type);

        if (icode::is_float(e.op2.dtype))
            return llvm_builder->CreateFPToUI(get_llvm_value(e.op2), destination_type);

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    Value* llvm_generator::cast_to_float(const icode::entry& e, Type* destination_type)
    {
        if (icode::is_sint(e.op2.dtype))
            return llvm_builder->CreateSIToFP(get_llvm_value(e.op2), destination_type);

        if (icode::is_uint(e.op2.dtype))
            return llvm_builder->CreateUIToFP(get_llvm_value(e.op2), destination_type);

        if (icode::is_float(e.op2.dtype))
        {
            if (icode::dtype_size[e.op1.dtype] > icode::dtype_size[e.op2.dtype])
                return llvm_builder->CreateFPExt(get_llvm_value(e.op2), destination_type);

            if (icode::dtype_size[e.op1.dtype] < icode::dtype_size[e.op2.dtype])
                return llvm_builder->CreateFPTrunc(get_llvm_value(e.op2), destination_type);

            if (icode::dtype_size[e.op1.dtype] == icode::dtype_size[e.op2.dtype])
                return get_llvm_value(e.op2);
        }

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    void llvm_generator::cast(const icode::entry& e)
    {
        Type* destination_type = to_llvm_type(e.op1.dtype);
        Value* result;

        if (icode::is_sint(e.op1.dtype))
            result = cast_to_sint(e, destination_type);
        else if (icode::is_uint(e.op1.dtype))
            result = cast_to_uint(e, destination_type);
        else if (icode::is_float(e.op1.dtype))
            result = cast_to_float(e, destination_type);
        else
        {
            miklog::internal_error(module.name);
            throw miklog::internal_bug_error();
        }

        set_llvm_value(e.op1, result);
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

    Value* llvm_generator::get_format_string(icode::data_type dtype)
    {
        if (icode::is_uint(dtype))
            return uint_format_str;

        if (icode::is_sint(dtype))
            return int_format_str;

        if (icode::is_float(dtype))
            return float_format_str;

        miklog::internal_error(module.name);
        throw miklog::internal_bug_error();
    }

    void llvm_generator::call_printf(Value* format_str, Value* value)
    {
        /* Set up printf arguments*/
        std::vector<Value*> printArgs;
        printArgs.push_back(format_str);
        if (value)
            printArgs.push_back(value);

        /* Call printf */
        llvm_builder->CreateCall(llvm_module->getFunction("printf"), printArgs);
    }

    void llvm_generator::print(const icode::entry& e)
    {
        Value* value = get_llvm_value(e.op1);

        /* Cast value to double if float */
        if (icode::is_float(e.op1.dtype))
            value = llvm_builder->CreateFPCast(value, Type::getDoubleTy(*llvm_context));
        else
            value = llvm_builder->CreateSExt(value, Type::getInt32Ty(*llvm_context));

        call_printf(get_format_string(e.op1.dtype), value);
    }

    void llvm_generator::print_str(const icode::entry& e)
    {
        Value* str_value = get_llvm_alloca(e.op1);

        call_printf(str_value);
    }

    void llvm_generator::call(const icode::entry& e)
    {
        Value* result = llvm_builder->CreateCall(llvm_module->getFunction(e.op2.name), params);

        set_llvm_value(e.op1, result);

        params.clear();
    }

    void llvm_generator::ret(const icode::entry& e, icode::data_type dtype)
    {
        if (dtype == icode::VOID)
            llvm_builder->CreateRetVoid();
        else
            llvm_builder->CreateRet(llvm_builder->CreateLoad(current_ret_value));
    }

    void llvm_generator::pass(const icode::entry& e) { params.push_back(get_llvm_value(e.op1)); }

    void llvm_generator::pass_addr(const icode::entry& e) { params.push_back(get_llvm_alloca(e.op1)); }

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
                case icode::NOT:
                    bwnot(e);
                    break;
                case icode::UNARY_MINUS:
                    minus(e);
                    break;
                case icode::CAST:
                    cast(e);
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
                case icode::NEWLN:
                    call_printf(newln_format_str);
                    break;
                case icode::SPACE:
                    call_printf(space_format_str);
                    break;
                case icode::PASS:
                    pass(e);
                    break;
                case icode::PASS_ADDR:
                    pass_addr(e);
                    break;
                case icode::CALL:
                    call(e);
                    break;
                case icode::RET:
                    ret(e, func_desc.func_info.dtype);
                    break;
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

    void llvm_generator::reset_state()
    {
        alloca_inst_map.clear();
        operand_value_map.clear();

        label_block_map.clear();
        fall_block_map.clear();
        backpatch_point_map.clear();
        backpatch_entry_q.clear();

        params.clear();
    }

    void llvm_generator::setup_func_stack(icode::func_desc& func_desc, Function* F)
    {
        /* Allocate space for local variables */
        for (auto symbol : func_desc.symbols)
            if (!symbol.second.check(icode::IS_PARAM))
                local_symbol_alloca(symbol.second, symbol.first);

        /* Assign passed function args */
        unsigned int i = 0;
        for (auto& arg : F->args())
        {
            std::string& symbol_name = func_desc.params[i];
            arg.setName(symbol_name);
            param_symbol_alloca(func_desc.symbols[symbol_name], symbol_name, &arg);
            i++;
        }
    }

    void llvm_generator::gen_function(icode::func_desc& func_desc, const std::string& name)
    {
        reset_state();

        /* Create function */
        Function* llvm_func = llvm_function_map[name];

        /* Set insertion point to function body */
        BasicBlock* BB = BasicBlock::Create(*llvm_context, "entry", llvm_func);
        llvm_builder->SetInsertPoint(BB);

        /* Allocate stack space for local variables */
        setup_func_stack(func_desc, llvm_func);

        /* Set ret ptr */
        if (func_desc.func_info.dtype != icode::VOID)
            current_ret_value = llvm_builder->CreateAlloca(vinfo_to_llvm_type(func_desc.func_info), nullptr);

        /* Convert mikuro function ir to llvm ir */
        gen_func_icode(func_desc, llvm_func);

        /* Process goto backpathing */
        process_goto_backpatch();

        verifyFunction(*llvm_func);
    }

    void llvm_generator::gen_globals()
    {
        for (auto symbol : module.globals)
            global_symbol_alloca(symbol.second, symbol.first);
    }

    void llvm_generator::declare_functions()
    {
        for (auto func : module.functions)
        {
            Function* llvm_func = Function::Create(fdesc_to_llvm_type(func.second),
                                                   Function::ExternalLinkage,
                                                   func.first,
                                                   llvm_module.get());

            llvm_function_map[func.first] = llvm_func;
        }
    }

    void llvm_generator::setup_printf()
    {
        /* Declare printf function */
        std::vector<Type*> args;
        args.push_back(Type::getInt8PtrTy(*llvm_context));
        FunctionType* printf_type = FunctionType::get(llvm_builder->getInt32Ty(), args, true);
        Function::Create(printf_type, Function::ExternalLinkage, "printf", llvm_module.get());

        /* Setup global format strings */
        int_format_str = llvm_builder->CreateGlobalString("%d", "int_format_str", 0U, llvm_module.get());
        uint_format_str = llvm_builder->CreateGlobalString("%s", "uint_format_str", 0U, llvm_module.get());
        float_format_str = llvm_builder->CreateGlobalString("%.2f", "float_format_str", 0U, llvm_module.get());
        newln_format_str = llvm_builder->CreateGlobalString("\n", "newln", 0U, llvm_module.get());
        space_format_str = llvm_builder->CreateGlobalString(" ", "space", 0U, llvm_module.get());
    }

    llvm_generator::llvm_generator(icode::module_desc& module_desc)
      : module(module_desc)
    {
        /* Setup LLVM context, module and builder */
        llvm_context = std::make_unique<LLVMContext>();
        llvm_module = std::make_unique<Module>(module.name, *llvm_context);
        llvm_builder = std::make_unique<IRBuilder<>>(*llvm_context);
        prev_instr_branch = false;

        /* Generate global variables */
        gen_globals();

        /* Declare all the functions */
        declare_functions();

        /* Declare that printf exists and has signature int (i8*, ...) */
        setup_printf();

        /* Loop through each function and convert mikuro IR to llvm IR */
        for (auto func : module.functions)
        {
            gen_function(func.second, func.first);
        }

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
            miklog::println("LLVM ERROR: Could not open file: " + EC.message());
            throw miklog::internal_bug_error();
        }

        legacy::PassManager pass;
        auto FileType = CGFT_ObjectFile;

        if (target_machine->addPassesToEmitFile(pass, dest, nullptr, FileType))
        {
            miklog::println("LLVM ERROR: LLVM target machine can't emit a file of this type");
            throw miklog::internal_bug_error();
        }

        pass.run(*llvm_module);

        dest.flush();
    }

    std::string llvm_generator::get_llvm_str()
    {
        std::string mod_string;
        raw_string_ostream OS(mod_string);
        OS << *llvm_module;
        OS.flush();

        return mod_string;
    }
}