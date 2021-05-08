#include "llvmgen.hpp"

using namespace llvm;
using namespace sys;

namespace llvmgen
{
    icode::target_desc getTargetDescription()
    {
        /* Target description for mikuro-llvm */
        icode::target_desc target;

        target.dtype_strings_map = { { "byte", icode::I8 },     { "ubyte", icode::UI8 },  { "short", icode::I16 },
                                     { "ushort", icode::UI16 }, { "int", icode::I32 },    { "uint", icode::UI32 },
                                     { "long", icode::I64 },    { "ulong", icode::UI64 }, { "float", icode::F32 },
                                     { "double", icode::F64 },  { "char", icode::UI8 },   { "bool", icode::UI8 } };

        /* true and false defines */
        icode::def trueDef;
        trueDef.dtype = icode::INT;
        trueDef.val.integer = 1;

        icode::def falseDef;
        falseDef.dtype = icode::INT;
        falseDef.val.integer = 0;

        target.defines = { { "true", trueDef }, { "false", falseDef } };

        /* Default int or word */
        target.default_int = icode::I32;
        target.str_int = icode::UI8;

        return target;
    }

    Type* LLVMTranslator::dataTypeToLLVMType(icode::data_type dtype)
    {
        /* Converts mikuro ir icode::data_type to llvm type */

        switch (dtype)
        {
            case icode::I8:
            case icode::UI8:
                return Type::getInt8Ty(*context);
            case icode::I16:
            case icode::UI16:
                return Type::getInt16Ty(*context);
            case icode::I32:
            case icode::UI32:
            case icode::INT:
                return Type::getInt32Ty(*context);
            case icode::I64:
            case icode::UI64:
                return Type::getInt64Ty(*context);
            case icode::F32:
            case icode::FLOAT:
                return Type::getFloatTy(*context);
            case icode::F64:
                return Type::getDoubleTy(*context);
            case icode::VOID:
                return Type::getVoidTy(*context);
            default:
                miklog::internal_error(moduleDescription.name);
                throw miklog::internal_bug_error();
        }
    }

    Type* LLVMTranslator::dataTypeToLLVMPointerType(icode::data_type dtype)
    {
        /* Converts mikuro ir icode::data_type to llvm pointer type */

        switch (dtype)
        {
            case icode::I8:
            case icode::UI8:
            case icode::STRUCT:
                return Type::getInt8PtrTy(*context);
            case icode::I16:
            case icode::UI16:
                return Type::getInt16PtrTy(*context);
            case icode::I32:
            case icode::UI32:
            case icode::INT:
                return Type::getInt32PtrTy(*context);
            case icode::I64:
            case icode::UI64:
                return Type::getInt64PtrTy(*context);
            case icode::F32:
            case icode::FLOAT:
                return Type::getFloatPtrTy(*context);
            case icode::F64:
                return Type::getDoublePtrTy(*context);
            default:
                miklog::internal_error(moduleDescription.name);
                throw miklog::internal_bug_error();
        }
    }

    Type* LLVMTranslator::varDescriptionToLLVMType(const icode::var_info& varDescription)
    {
        if (varDescription.check(icode::IS_PTR))
            return dataTypeToLLVMPointerType(varDescription.dtype);

        if (varDescription.dimensions.size() > 0 || varDescription.dtype == icode::STRUCT)
            return ArrayType::get(Type::getInt8Ty(*context), varDescription.size);

        return dataTypeToLLVMType(varDescription.dtype);
    }

    FunctionType* LLVMTranslator::funcDescriptionToLLVMType(icode::func_desc& funcDescription)
    {
        std::vector<Type*> parameterTypes;

        /* Set the types vector */
        for (std::string paramString : funcDescription.params)
        {
            Type* type = varDescriptionToLLVMType(funcDescription.symbols[paramString]);
            parameterTypes.push_back(type);
        }

        /* Setup llvm function */
        FunctionType* FT =
          FunctionType::get(varDescriptionToLLVMType(funcDescription.func_info), parameterTypes, false);

        return FT;
    }

    Value* LLVMTranslator::getLLVMConstant(const icode::operand& op)
    {
        /* Convetrs mikuro icode::LITERAL operand type to llvm value  */

        if (op.optype == icode::ADDR)
            return ConstantInt::get(Type::getInt64Ty(*context), op.val.integer);

        if (icode::is_int(op.dtype))
            return ConstantInt::get(dataTypeToLLVMType(op.dtype), op.val.integer);

        if (icode::is_float(op.dtype))
            return ConstantFP::get(dataTypeToLLVMType(op.dtype), op.val.floating);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    std::string getFullFunctionName(const std::string& functionName, const std::string& moduleName)
    {
        if (functionName == "main")
            return functionName;

        return moduleName + "." + functionName;
    }

    Function* LLVMTranslator::getLLVMFunction(const std::string& functionName, const std::string& moduleName)
    {
        std::string expandedFunctionName = getFullFunctionName(functionName, moduleName);

        if (auto* F = LLVMModule->getFunction(expandedFunctionName))
            return F;

        FunctionType* functionType = funcDescriptionToLLVMType(externalModulesRef[moduleName].functions[functionName]);
        return Function::Create(functionType, Function::ExternalLinkage, expandedFunctionName, *LLVMModule);
    }

    Value* LLVMTranslator::getCurrentRetValuePointer(const icode::operand& op)
    {
        Value* returnValue = operandValueMap[op];
        Value* returnValuePointer = builder->CreateAlloca(returnValue->getType());
        builder->CreateStore(returnValue, returnValuePointer);

        return returnValuePointer;
    }

    Value* LLVMTranslator::getLLVMPointer(const icode::operand& op)
    {
        /* Returns llvm value allocated by symbol_alloca */

        switch (op.optype)
        {
            case icode::PTR:
            case icode::VAR:
                return symbolNamePointersMap[op.name];
            case icode::GBL_VAR:
                return symbolNameGlobalsMap[op.name];
            case icode::STR_DATA:
                return builder->CreateGlobalStringPtr(moduleDescription.str_data[op.name]);
            case icode::RET_PTR:
                return currentFunctionReturnValue;
            case icode::TEMP_PTR:
                return builder->CreateIntToPtr(getLLVMValue(op), dataTypeToLLVMPointerType(op.dtype));
            case icode::RET_VAL:
                return getCurrentRetValuePointer(op);
            default:
                break;
        }

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::getLLVMValue(const icode::operand& op)
    {
        /* Convertes mikuro icode::operand to llvm value */

        switch (op.optype)
        {
            case icode::LITERAL:
            case icode::ADDR:
                return getLLVMConstant(op);
                return getLLVMConstant(op);
            case icode::GBL_VAR:
                return builder->CreateLoad(symbolNameGlobalsMap[op.name], op.name.c_str());
            case icode::VAR:
                return builder->CreateLoad(symbolNamePointersMap.at(op.name), op.name.c_str());
            case icode::TEMP_PTR:
            case icode::TEMP:
            case icode::RET_VAL:
                return operandValueMap[op];
            case icode::RET_PTR:
                return builder->CreatePtrToInt(currentFunctionReturnValue, dataTypeToLLVMType(icode::I64));
            case icode::PTR:
                return symbolNamePointerIntMap[op.name];
            default:
                miklog::internal_error(moduleDescription.name);
                throw miklog::internal_bug_error();
        }
    }

    void LLVMTranslator::setLLVMValue(const icode::operand& op, Value* value)
    {
        /* Assigns llvm value to a mikuro icode::operand */

        switch (op.optype)
        {
            case icode::TEMP:
            case icode::TEMP_PTR:
            case icode::RET_VAL:
                operandValueMap[op] = value;
                break;
            case icode::VAR:
            case icode::GBL_VAR:
                builder->CreateStore(value, getLLVMPointer(op));
                break;
            default:
                miklog::internal_error(moduleDescription.name);
                throw miklog::internal_bug_error();
        }
    }

    void LLVMTranslator::createLocalSymbol(const icode::var_info& varDescription, const std::string& name)
    {
        /* Creates llvm alloca instruction for icode::var_info symbols in symbol tables */

        symbolNamePointersMap[name] = builder->CreateAlloca(varDescriptionToLLVMType(varDescription), nullptr, name);
    }

    void LLVMTranslator::createGlobalSymbol(icode::var_info& varDescription, const std::string& name)
    {
        GlobalVariable* global;
        Type* type = varDescriptionToLLVMType(varDescription);

        global = new GlobalVariable(*LLVMModule, type, false, GlobalVariable::CommonLinkage, nullptr, name);

        global->setInitializer(Constant::getNullValue(type));

        symbolNameGlobalsMap[name] = global;
    }

    void LLVMTranslator::createFunctionParameter(const icode::var_info& varDescription, const std::string& name, Value* arg)
    {
        /* If mutable parameters, stores the pointer as int, else allocate space and stores arg */

        if (!varDescription.check(icode::IS_PTR))
        {
            Value* alloca = builder->CreateAlloca(varDescriptionToLLVMType(varDescription), nullptr, name);
            builder->CreateStore(arg, alloca);
            symbolNamePointersMap[name] = alloca;
        }
        else
        {
            symbolNamePointersMap[name] = arg;
            symbolNamePointerIntMap[name] = builder->CreatePtrToInt(arg, dataTypeToLLVMType(icode::I64));
        }
    }

    void LLVMTranslator::createPointer(const icode::entry& e)
    {
        /* Converts mikuro CREATE_PTR to llvm ir */

        switch (e.op2.optype)
        {
            case icode::TEMP_PTR:
                operandValueMap[e.op1] = operandValueMap[e.op2];
                break;
            case icode::VAR:
            case icode::GBL_VAR:
                operandValueMap[e.op1] =
                  builder->CreatePtrToInt(getLLVMPointer(e.op2), dataTypeToLLVMType(icode::I64));
                break;
            case icode::PTR:
                operandValueMap[e.op1] = symbolNamePointerIntMap[e.op2.name];
                break;
            case icode::RET_VAL:
                operandValueMap[e.op1] = getCurrentRetValuePointer(e.op2);
                break;
            case icode::RET_PTR:
                operandValueMap[e.op1] = currentFunctionReturnValue;
                break;
            default:
                miklog::internal_error(moduleDescription.name);
                throw miklog::internal_bug_error();
        }
    }

    void LLVMTranslator::copy(const icode::entry& e)
    {
        /* Converts mikuro EQUAL to llvm ir */

        Value* sourceValue = getLLVMValue(e.op2);
        setLLVMValue(e.op1, sourceValue);
    }

    void LLVMTranslator::read(const icode::entry& e)
    {
        /* Converts mikuro READ to llvm ir */

        Value* sourcePointer = getLLVMPointer(e.op2);
        Value* sourceValue = builder->CreateLoad(sourcePointer);

        setLLVMValue(e.op1, sourceValue);
    }

    void LLVMTranslator::write(const icode::entry& e)
    {
        /* Converts mikuro WRITE to llvm ir */

        Value* destinationPointer = getLLVMPointer(e.op1);
        Value* sourceValue = getLLVMValue(e.op2);

        builder->CreateStore(sourceValue, destinationPointer);
    }

    Value* LLVMTranslator::ensureI64(Value* value)
    {
        if (value->getType() == dataTypeToLLVMType(icode::I64))
            return value;

        return builder->CreateZExtOrTrunc(value, dataTypeToLLVMType(icode::I64));
    }

    void LLVMTranslator::addressBinaryOperator(const icode::entry& e)
    {
        /* Converts mikuro ADDR_ADD and ADDR_MUL to llvm ir */

        Value* result;
        Value* LHS = getLLVMValue(e.op2);
        Value* RHS = getLLVMValue(e.op3);

        switch (e.opcode)
        {
            case icode::ADDR_ADD:
                result = builder->CreateNUWAdd(LHS, RHS);
                break;
            case icode::ADDR_MUL:
                result = builder->CreateNUWMul(ensureI64(LHS), RHS);
                break;
            default:
                miklog::internal_error(moduleDescription.name);
                throw miklog::internal_bug_error();
        }

        /* Store result llvm in map so it can be used by other llvm trnaslations */
        setLLVMValue(e.op1, result);
    }

    Value* LLVMTranslator::add(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro ADD to llvm ir */

        if (icode::is_sint(dtype))
            return builder->CreateNSWAdd(LHS, RHS);

        if (icode::is_uint(dtype))
            return builder->CreateNUWAdd(LHS, RHS);

        if (icode::is_float(dtype))
            return builder->CreateFAdd(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::subtract(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro SUB to llvm ir */

        if (icode::is_sint(dtype))
            return builder->CreateNSWSub(LHS, RHS);

        if (icode::is_uint(dtype))
            return builder->CreateNUWSub(LHS, RHS);

        if (icode::is_float(dtype))
            return builder->CreateFSub(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::multiply(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro MUL to llvm ir */

        if (icode::is_sint(dtype))
            return builder->CreateNSWMul(LHS, RHS);

        if (icode::is_uint(dtype))
            return builder->CreateNUWMul(LHS, RHS);

        if (icode::is_float(dtype))
            return builder->CreateFMul(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::divide(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro DIV to llvm ir */

        if (icode::is_sint(dtype))
            return builder->CreateSDiv(LHS, RHS);

        if (icode::is_uint(dtype))
            return builder->CreateUDiv(LHS, RHS);

        if (icode::is_float(dtype))
            return builder->CreateFDiv(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::remainder(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro MOD to llvm ir */

        if (icode::is_sint(dtype))
            return builder->CreateSRem(LHS, RHS);

        if (icode::is_uint(dtype))
            return builder->CreateURem(LHS, RHS);

        if (icode::is_float(dtype))
            return builder->CreateFRem(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::rightShift(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro RSH to llvm ir */

        if (icode::is_sint(dtype))
            return builder->CreateAShr(LHS, RHS);

        if (icode::is_uint(dtype))
            return builder->CreateLShr(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::leftShift(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro LSH to llvm ir */

        if (icode::is_int(dtype))
            return builder->CreateShl(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::bitwiseAnd(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro BWA to llvm ir */

        if (icode::is_int(dtype))
            return builder->CreateAnd(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::bitwiseOr(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro BWO to llvm ir */

        if (icode::is_int(dtype))
            return builder->CreateOr(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::bitwiseXor(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro BWX to llvm ir */

        if (icode::is_int(dtype))
            return builder->CreateXor(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    void LLVMTranslator::binaryOperator(const icode::entry& e)
    {
        Value* LHS = getLLVMValue(e.op2);
        Value* RHS = getLLVMValue(e.op3);
        icode::data_type dtype = e.op1.dtype;

        switch (e.opcode)
        {
            case icode::ADD:
                setLLVMValue(e.op1, add(LHS, RHS, dtype));
                break;
            case icode::SUB:
                setLLVMValue(e.op1, subtract(LHS, RHS, dtype));
                break;
            case icode::MUL:
                setLLVMValue(e.op1, multiply(LHS, RHS, dtype));
                break;
            case icode::DIV:
                setLLVMValue(e.op1, divide(LHS, RHS, dtype));
                break;
            case icode::MOD:
                setLLVMValue(e.op1, remainder(LHS, RHS, dtype));
                break;
            case icode::RSH:
                setLLVMValue(e.op1, rightShift(LHS, RHS, dtype));
                break;
            case icode::LSH:
                setLLVMValue(e.op1, leftShift(LHS, RHS, dtype));
                break;
            case icode::BWA:
                setLLVMValue(e.op1, bitwiseAnd(LHS, RHS, dtype));
                break;
            case icode::BWO:
                setLLVMValue(e.op1, bitwiseOr(LHS, RHS, dtype));
                break;
            case icode::BWX:
                setLLVMValue(e.op1, bitwiseXor(LHS, RHS, dtype));
                break;
            default:
                miklog::internal_error(moduleDescription.name);
                throw miklog::internal_bug_error();
        }
    }

    void LLVMTranslator::bitwiseNot(const icode::entry& e)
    {
        Value* result = builder->CreateNot(getLLVMValue(e.op2));

        setLLVMValue(e.op1, result);
    }

    void LLVMTranslator::unaryMinus(const icode::entry& e)
    {
        Value* result;

        if (icode::is_int(e.op2.dtype))
            result = builder->CreateNeg(getLLVMValue(e.op2));
        else
            result = builder->CreateFNeg(getLLVMValue(e.op2));

        setLLVMValue(e.op1, result);
    }

    Value* LLVMTranslator::castToSignedInt(const icode::entry& e, Type* destType)
    {
        if (icode::is_sint(e.op2.dtype))
            return builder->CreateSExtOrTrunc(getLLVMValue(e.op2), destType);

        if (icode::is_uint(e.op2.dtype))
            return builder->CreateZExtOrTrunc(getLLVMValue(e.op2), destType);

        if (icode::is_float(e.op2.dtype))
            return builder->CreateFPToSI(getLLVMValue(e.op2), destType);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::castToUnsignedInt(const icode::entry& e, Type* destType)
    {
        if (icode::is_int(e.op2.dtype))
            return builder->CreateZExtOrTrunc(getLLVMValue(e.op2), destType);

        if (icode::is_float(e.op2.dtype))
            return builder->CreateFPToUI(getLLVMValue(e.op2), destType);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::castToFloat(const icode::entry& e, Type* destType)
    {
        if (icode::is_sint(e.op2.dtype))
            return builder->CreateSIToFP(getLLVMValue(e.op2), destType);

        if (icode::is_uint(e.op2.dtype))
            return builder->CreateUIToFP(getLLVMValue(e.op2), destType);

        if (icode::is_float(e.op2.dtype))
        {
            if (icode::dtype_size[e.op1.dtype] > icode::dtype_size[e.op2.dtype])
                return builder->CreateFPExt(getLLVMValue(e.op2), destType);

            if (icode::dtype_size[e.op1.dtype] < icode::dtype_size[e.op2.dtype])
                return builder->CreateFPTrunc(getLLVMValue(e.op2), destType);

            if (icode::dtype_size[e.op1.dtype] == icode::dtype_size[e.op2.dtype])
                return getLLVMValue(e.op2);
        }

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    void LLVMTranslator::cast(const icode::entry& e)
    {
        Type* destType = dataTypeToLLVMType(e.op1.dtype);
        Value* result;

        if (icode::is_sint(e.op1.dtype))
            result = castToSignedInt(e, destType);
        else if (icode::is_uint(e.op1.dtype))
            result = castToUnsignedInt(e, destType);
        else if (icode::is_float(e.op1.dtype))
            result = castToFloat(e, destType);
        else
        {
            miklog::internal_error(moduleDescription.name);
            throw miklog::internal_bug_error();
        }

        setLLVMValue(e.op1, result);
    }

    Value* LLVMTranslator::eqaul(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro EQ to llvm ir */

        if (icode::is_int(dtype))
            return builder->CreateICmpEQ(LHS, RHS);

        if (icode::is_float(dtype))
            return builder->CreateFCmpUEQ(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::notEqual(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro NEQ to llvm ir */

        if (icode::is_int(dtype))
            return builder->CreateICmpNE(LHS, RHS);

        if (icode::is_float(dtype))
            return builder->CreateFCmpUNE(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::lessThan(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro LT to llvm ir */

        if (icode::is_sint(dtype))
            return builder->CreateICmpSLT(LHS, RHS);

        if (icode::is_uint(dtype))
            return builder->CreateICmpULT(LHS, RHS);

        if (icode::is_float(dtype))
            return builder->CreateFCmpULT(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::lessThanOrEqualTo(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro LTE to llvm ir */

        if (icode::is_sint(dtype))
            return builder->CreateICmpSLE(LHS, RHS);

        if (icode::is_uint(dtype))
            return builder->CreateICmpULE(LHS, RHS);

        if (icode::is_float(dtype))
            return builder->CreateFCmpULE(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::greaterThan(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro GT to llvm ir */

        if (icode::is_sint(dtype))
            return builder->CreateICmpSGT(LHS, RHS);

        if (icode::is_uint(dtype))
            return builder->CreateICmpUGT(LHS, RHS);

        if (icode::is_float(dtype))
            return builder->CreateFCmpUGT(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::greaterThanOrEqualTo(Value* LHS, Value* RHS, const icode::data_type dtype)
    {
        /* Converts mikuro GTE to llvm ir */

        if (icode::is_sint(dtype))
            return builder->CreateICmpSGE(LHS, RHS);

        if (icode::is_uint(dtype))
            return builder->CreateICmpUGE(LHS, RHS);

        if (icode::is_float(dtype))
            return builder->CreateFCmpUGE(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    void LLVMTranslator::compareOperator(const icode::entry& e)
    {
        Value* LHS = getLLVMValue(e.op1);
        Value* RHS = getLLVMValue(e.op2);
        icode::data_type dtype = e.op1.dtype;

        switch (e.opcode)
        {
            case icode::EQ:
                branchFlags.push(eqaul(LHS, RHS, dtype));
                break;
            case icode::NEQ:
                branchFlags.push(notEqual(LHS, RHS, dtype));
                break;
            case icode::LT:
                branchFlags.push(lessThan(LHS, RHS, dtype));
                break;
            case icode::LTE:
                branchFlags.push(lessThanOrEqualTo(LHS, RHS, dtype));
                break;
            case icode::GT:
                branchFlags.push(greaterThan(LHS, RHS, dtype));
                break;
            case icode::GTE:
                branchFlags.push(greaterThanOrEqualTo(LHS, RHS, dtype));
                break;
            default:
                miklog::internal_error(moduleDescription.name);
                throw miklog::internal_bug_error();
        }
    }

    void LLVMTranslator::createLabel(const icode::entry& e, Function* function)
    {
        /* Converts mikuro CREATE_LABEL to llvm basic block  */
        BasicBlock* newBlock = BasicBlock::Create(*context, e.op1.name, function);

        labelToBasicBlockMap[e.op1] = newBlock;

        /* Make sure every block has a terminator */
        if (!prevInstructionGotoOrRet)
            builder->CreateBr(newBlock);

        /* Start inserting ir into new block */
        builder->SetInsertPoint(newBlock);
    }

    void LLVMTranslator::createGotoBackpatch(const icode::entry& e, Function* F, size_t entryIndex)
    {
        /* All branch instructions GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO are backpatched,
            i.e the task is stored in a queue and instructions are created in a second pass */

        if (prevInstructionGotoOrRet)
            return;

        /* Save llvm insertion point corresponding to this entry */
        insertionPoints[entryIndex] = InsertionPoint(builder->GetInsertBlock(), builder->GetInsertPoint()++);

        /* Append to backpath queue */
        backpatchQueue.push_back(EnumeratedEntry(entryIndex, e));

        if (e.opcode == icode::GOTO)
            return;

        /* Create basic block for fall through for IF_TRUE_GOTO, IF_FALSE_GOTO */
        BasicBlock* fallBlock = BasicBlock::Create(*context, "_fall_e" + std::to_string(entryIndex), F);

        fallBlocks[entryIndex] = fallBlock;

        /* Start inserstion in fall through block */
        builder->SetInsertPoint(fallBlock);
    }

    void LLVMTranslator::createBranch(const icode::entry& e, Value* flag, BasicBlock* gotoBlock, BasicBlock* fallBlock)
    {
        /* Convert mikuro GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO int llvm ir  */

        switch (e.opcode)
        {
            case icode::GOTO:
                builder->CreateBr(gotoBlock);
                break;
            case icode::IF_TRUE_GOTO:
                builder->CreateCondBr(flag, gotoBlock, fallBlock);
                break;
            case icode::IF_FALSE_GOTO:
                builder->CreateCondBr(flag, fallBlock, gotoBlock);
                break;
            default:
                miklog::internal_error(moduleDescription.name);
                throw miklog::internal_bug_error();
        }
    }

    void LLVMTranslator::processGotoBackpatches()
    {
        /* Go through backpatch queue */
        for (size_t i = 0; i < backpatchQueue.size(); i++)
        {
            /* Get the entry from backpatch q */
            EnumeratedEntry& enumeratedEntry = backpatchQueue[i];
            size_t entryIndex = enumeratedEntry.first;
            icode::entry& e = enumeratedEntry.second;

            /* Get branch flags and blocks for the goto */
            BasicBlock* gotoBlock = labelToBasicBlockMap[e.op1];
            BasicBlock* fallBlock = fallBlocks[entryIndex];
            Value* flag = branchFlags.front();

            /* Get insertion point corresponding to the entry */
            InsertionPoint insertPoint = insertionPoints[entryIndex];
            builder->SetInsertPoint(insertPoint.first, insertPoint.second);

            /* Create branch */
            createBranch(e, flag, gotoBlock, fallBlock);

            /* Pop from flags queue after processing branch */
            if (e.opcode != icode::GOTO)
                branchFlags.pop();
        }
    }

    Value* LLVMTranslator::getFromatString(icode::data_type dtype)
    {
        if (icode::is_uint(dtype))
            return uintFormatString;

        if (icode::is_sint(dtype))
            return intFormatString;

        if (icode::is_float(dtype))
            return floatFormatString;

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    void LLVMTranslator::callPrintf(Value* format_str, Value* value)
    {
        /* Set up printf arguments*/
        std::vector<Value*> printArgs;
        printArgs.push_back(format_str);
        if (value)
            printArgs.push_back(value);

        /* Call printf */
        builder->CreateCall(LLVMModule->getFunction("printf"), printArgs);
    }

    void LLVMTranslator::print(const icode::entry& e)
    {
        Value* value = getLLVMValue(e.op1);

        /* Cast value to double if float */
        if (icode::is_float(e.op1.dtype))
            value = builder->CreateFPCast(value, Type::getDoubleTy(*context));
        else
            value = builder->CreateSExt(value, Type::getInt32Ty(*context));

        callPrintf(getFromatString(e.op1.dtype), value);
    }

    void LLVMTranslator::printString(const icode::entry& e)
    {
        Value* str_value = getLLVMPointer(e.op1);

        callPrintf(str_value);
    }

    void LLVMTranslator::call(const icode::entry& e)
    {
        Value* result = builder->CreateCall(getLLVMFunction(e.op2.name, e.op3.name), params);

        setLLVMValue(e.op1, result);

        params.clear();
    }

    void LLVMTranslator::ret(const icode::entry& e, icode::data_type dtype)
    {
        if (dtype == icode::VOID)
            builder->CreateRetVoid();
        else
            builder->CreateRet(builder->CreateLoad(currentFunctionReturnValue));
    }

    void LLVMTranslator::pass(const icode::entry& e) { params.push_back(getLLVMValue(e.op1)); }

    void LLVMTranslator::passPointer(const icode::entry& e) { params.push_back(getLLVMPointer(e.op1)); }

    void LLVMTranslator::translateFunctionIcode(const icode::func_desc& funcDesc, Function* F)
    {
        /* Go through icode and generate llvm ir */
        for (size_t i = 0; i < funcDesc.icode_table.size(); i++)
        {
            icode::entry e = funcDesc.icode_table[i];

            switch (e.opcode)
            {
                case icode::EQUAL:
                    copy(e);
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
                    binaryOperator(e);
                    break;
                case icode::NOT:
                    bitwiseNot(e);
                    break;
                case icode::UNARY_MINUS:
                    unaryMinus(e);
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
                    compareOperator(e);
                    break;
                case icode::CREATE_LABEL:
                    createLabel(e, F);
                    break;
                case icode::IF_TRUE_GOTO:
                case icode::IF_FALSE_GOTO:
                case icode::GOTO:
                    createGotoBackpatch(e, F, i);
                    break;
                case icode::CREATE_PTR:
                    createPointer(e);
                    break;
                case icode::ADDR_ADD:
                case icode::ADDR_MUL:
                    addressBinaryOperator(e);
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
                    printString(e);
                    break;
                case icode::NEWLN:
                    callPrintf(newLineString);
                    break;
                case icode::SPACE:
                    callPrintf(spaceString);
                    break;
                case icode::PASS:
                    pass(e);
                    break;
                case icode::PASS_ADDR:
                    passPointer(e);
                    break;
                case icode::CALL:
                    call(e);
                    break;
                case icode::RET:
                    ret(e, funcDesc.func_info.dtype);
                    break;
                case icode::EXIT:
                    break;
                default:
                    miklog::internal_error(moduleDescription.name);
                    throw miklog::internal_bug_error();
            }

            prevInstructionGotoOrRet = e.opcode == icode::GOTO || e.opcode == icode::RET;
        }
    }

    void LLVMTranslator::resetState()
    {
        symbolNamePointersMap.clear();
        operandValueMap.clear();
        labelToBasicBlockMap.clear();
        fallBlocks.clear();
        insertionPoints.clear();
        backpatchQueue.clear();
        params.clear();
    }

    void LLVMTranslator::setupFunctionStack(icode::func_desc& funcDescription, Function* F)
    {
        /* Allocate space for local variables */
        for (auto symbol : funcDescription.symbols)
            if (!symbol.second.check(icode::IS_PARAM))
                createLocalSymbol(symbol.second, symbol.first);

        /* Assign passed function args */
        unsigned int i = 0;
        for (auto& arg : F->args())
        {
            std::string& symbol_name = funcDescription.params[i];
            arg.setName(symbol_name);
            createFunctionParameter(funcDescription.symbols[symbol_name], symbol_name, &arg);
            i++;
        }
    }

    void LLVMTranslator::generateFunction(icode::func_desc& funcDescription, const std::string& name)
    {
        resetState();

        /* Create function */
        Function* function = getLLVMFunction(name, moduleDescription.name);

        /* Set insertion point to function body */
        BasicBlock* functionBlock = BasicBlock::Create(*context, "entry", function);
        builder->SetInsertPoint(functionBlock);

        /* Allocate stack space for local variables */
        setupFunctionStack(funcDescription, function);

        /* Set ret ptr */
        if (funcDescription.func_info.dtype != icode::VOID)
            currentFunctionReturnValue =
              builder->CreateAlloca(varDescriptionToLLVMType(funcDescription.func_info), nullptr, name + ".retval");

        /* Convert mikuro function ir to llvm ir */
        translateFunctionIcode(funcDescription, function);

        /* Process goto backpathing */
        processGotoBackpatches();

        verifyFunction(*function);
    }

    void LLVMTranslator::generateGlobals()
    {
        for (auto symbol : moduleDescription.globals)
            createGlobalSymbol(symbol.second, symbol.first);
    }

    void LLVMTranslator::setupPrintf()
    {
        /* Declare printf function */
        std::vector<Type*> args;
        args.push_back(Type::getInt8PtrTy(*context));
        FunctionType* printf_type = FunctionType::get(builder->getInt32Ty(), args, true);
        Function::Create(printf_type, Function::ExternalLinkage, "printf", LLVMModule.get());

        /* Setup global format strings */
        intFormatString = builder->CreateGlobalString("%d", "intFormatString", 0U, LLVMModule.get());
        uintFormatString = builder->CreateGlobalString("%u", "uintFormatString", 0U, LLVMModule.get());
        floatFormatString = builder->CreateGlobalString("%f", "floatFormatString", 0U, LLVMModule.get());
        newLineString = builder->CreateGlobalString("\n", "newln", 0U, LLVMModule.get());
        spaceString = builder->CreateGlobalString(" ", "space", 0U, LLVMModule.get());
    }

    LLVMTranslator::LLVMTranslator(icode::module_desc& modDescription, icode::module_desc_map& modulesMap)
      : moduleDescription(modDescription)
      , externalModulesRef(modulesMap)
    {
        /* Setup LLVM context, module and builder */
        context = std::make_unique<LLVMContext>();
        LLVMModule = std::make_unique<Module>(modDescription.name, *context);
        builder = std::make_unique<IRBuilder<>>(*context);
        prevInstructionGotoOrRet = false;

        /* Generate global variables */
        generateGlobals();

        /* Declare that printf exists and has signature int (i8*, ...) */
        setupPrintf();

        /* Loop through each function and convert mikuro IR to llvm IR */
        for (auto func : modDescription.functions)
        {
            generateFunction(func.second, func.first);
        }

        /* Initialize the target registry etc */
        InitializeAllTargetInfos();
        InitializeAllTargets();
        InitializeAllTargetMCs();
        InitializeAllAsmParsers();
        InitializeAllAsmPrinters();

        /* Setup LLVM target triple */
        auto targetTriple = sys::getDefaultTargetTriple();
        LLVMModule->setTargetTriple(targetTriple);

        std::string error;
        auto Target = TargetRegistry::lookupTarget(targetTriple, error);

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
        auto targetMachine = Target->createTargetMachine(targetTriple, CPU, features, opt, RM);

        LLVMModule->setDataLayout(targetMachine->createDataLayout());

        /* Setup output object file and pass manager */
        auto filename = modDescription.name + ".o";
        std::error_code EC;
        raw_fd_ostream dest(filename, EC, sys::fs::OF_None);

        if (EC)
        {
            miklog::println("LLVM ERROR: Could not open file: " + EC.message());
            throw miklog::internal_bug_error();
        }

        legacy::PassManager pass;
        auto FileType = CGFT_ObjectFile;

        if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
        {
            miklog::println("LLVM ERROR: LLVM target machine can't emit a file of this type");
            throw miklog::internal_bug_error();
        }

        pass.run(*LLVMModule);

        dest.flush();
    }

    std::string LLVMTranslator::getLLVMModuleString()
    {
        std::string moduleString;
        raw_string_ostream OS(moduleString);
        OS << *LLVMModule;
        OS.flush();

        return moduleString;
    }
}