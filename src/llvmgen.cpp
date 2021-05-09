#include "llvmgen.hpp"

using namespace llvm;
using namespace sys;

namespace llvmgen
{
    icode::TargetDescription getTargetDescription()
    {
        /* Target description for mikuro-llvm */
        icode::TargetDescription target;

        target.dataTypeNames = { { "byte", icode::I8 },     { "ubyte", icode::UI8 },  { "short", icode::I16 },
                                     { "ushort", icode::UI16 }, { "int", icode::I32 },    { "uint", icode::UI32 },
                                     { "long", icode::I64 },    { "ulong", icode::UI64 }, { "float", icode::F32 },
                                     { "double", icode::F64 },  { "char", icode::UI8 },   { "bool", icode::UI8 } };

        /* true and false defines */
        icode::Define trueDef;
        trueDef.dtype = icode::INT;
        trueDef.val.integer = 1;

        icode::Define falseDef;
        falseDef.dtype = icode::INT;
        falseDef.val.integer = 0;

        target.defines = { { "true", trueDef }, { "false", falseDef } };

        /* Default int or word */
        target.characterInt = icode::UI8;

        return target;
    }

    Type* LLVMTranslator::dataTypeToLLVMType(icode::DataType dtype)
    {
        /* Converts mikuro ir icode::dataType to llvm type */

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

    Type* LLVMTranslator::dataTypeToLLVMPointerType(icode::DataType dtype)
    {
        /* Converts mikuro ir icode::dataType to llvm pointer type */

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

    Type* LLVMTranslator::variableDescriptionToLLVMType(const icode::VariableDescription& variableDesc)
    {
        if (variableDesc.checkProperty(icode::IS_PTR))
            return dataTypeToLLVMPointerType(variableDesc.dtype);

        if (variableDesc.dimensions.size() > 0 || variableDesc.dtype == icode::STRUCT)
            return ArrayType::get(Type::getInt8Ty(*context), variableDesc.size);

        return dataTypeToLLVMType(variableDesc.dtype);
    }

    FunctionType* LLVMTranslator::funcDescriptionToLLVMType(icode::FunctionDescription& functionDesc)
    {
        std::vector<Type*> parameterTypes;

        /* Set the types vector */
        for (std::string paramString : functionDesc.parameters)
        {
            Type* type = variableDescriptionToLLVMType(functionDesc.symbols[paramString]);
            parameterTypes.push_back(type);
        }

        /* Setup llvm function */
        FunctionType* FT = FunctionType::get(variableDescriptionToLLVMType(functionDesc.functionReturnDescription),
                                             parameterTypes,
                                             false);

        return FT;
    }

    Value* LLVMTranslator::getLLVMConstant(const icode::Operand& op)
    {
        /* Convetrs mikuro icode::LITERAL operand type to llvm value  */

        if (op.operandType == icode::ADDR)
            return ConstantInt::get(Type::getInt64Ty(*context), op.val.integer);

        if (icode::isInteger(op.dtype))
            return ConstantInt::get(dataTypeToLLVMType(op.dtype), op.val.integer);

        if (icode::isFloat(op.dtype))
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
        std::string fullFunctionName = getFullFunctionName(functionName, moduleName);

        if (auto* F = LLVMModule->getFunction(fullFunctionName))
            return F;

        FunctionType* functionType = funcDescriptionToLLVMType(externalModulesRef[moduleName].functions[functionName]);
        return Function::Create(functionType, Function::ExternalLinkage, fullFunctionName, *LLVMModule);
    }

    Value* LLVMTranslator::getCalleeRetValuePointer(const icode::Operand& op)
    {
        Value* returnValue = operandValueMap[op];
        Value* returnValuePointer = builder->CreateAlloca(returnValue->getType());
        builder->CreateStore(returnValue, returnValuePointer);

        return returnValuePointer;
    }

    Value* LLVMTranslator::getLLVMPointer(const icode::Operand& op)
    {
        /* Returns llvm value allocated by symbol_alloca */

        switch (op.operandType)
        {
            case icode::PTR:
            case icode::VAR:
                return symbolNamePointersMap[op.name];
            case icode::GBL_VAR:
                return symbolNameGlobalsMap[op.name];
            case icode::STR_DATA:
                return builder->CreateGlobalStringPtr(moduleDescription.str_data[op.name]);
            case icode::RET_PTR:
                return currentFunctionReturnPointer;
            case icode::TEMP_PTR:
                return builder->CreateIntToPtr(getLLVMValue(op), dataTypeToLLVMPointerType(op.dtype));
            case icode::CALLEE_RET_VAL:
                return getCalleeRetValuePointer(op);
            default:
                break;
        }

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::getLLVMValue(const icode::Operand& op)
    {
        /* Convertes mikuro icode::operand to llvm value */

        switch (op.operandType)
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
            case icode::CALLEE_RET_VAL:
                return operandValueMap[op];
            case icode::RET_PTR:
                return builder->CreatePtrToInt(currentFunctionReturnPointer, dataTypeToLLVMType(icode::I64));
            case icode::PTR:
                return symbolNamePointerIntMap[op.name];
            default:
                miklog::internal_error(moduleDescription.name);
                throw miklog::internal_bug_error();
        }
    }

    void LLVMTranslator::setLLVMValue(const icode::Operand& op, Value* value)
    {
        /* Assigns llvm value to a mikuro icode::operand */

        switch (op.operandType)
        {
            case icode::TEMP:
            case icode::TEMP_PTR:
            case icode::CALLEE_RET_VAL:
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

    void LLVMTranslator::createLocalSymbol(const icode::VariableDescription& variableDesc, const std::string& name)
    {
        /* Creates llvm alloca instruction for icode::var_info symbols in symbol tables */

        symbolNamePointersMap[name] =
          builder->CreateAlloca(variableDescriptionToLLVMType(variableDesc), nullptr, name);
    }

    void LLVMTranslator::createGlobalSymbol(icode::VariableDescription& varDescription, const std::string& name)
    {
        GlobalVariable* global;
        Type* type = variableDescriptionToLLVMType(varDescription);

        global = new GlobalVariable(*LLVMModule, type, false, GlobalVariable::CommonLinkage, nullptr, name);

        global->setInitializer(Constant::getNullValue(type));

        symbolNameGlobalsMap[name] = global;
    }

    void LLVMTranslator::createFunctionParameter(const icode::VariableDescription& variableDesc,
                                                 const std::string& name,
                                                 Value* arg)
    {
        /* If mutable parameters, stores the pointer as int, else allocate space and stores arg */

        if (!variableDesc.checkProperty(icode::IS_PTR))
        {
            Value* alloca = builder->CreateAlloca(variableDescriptionToLLVMType(variableDesc), nullptr, name);
            builder->CreateStore(arg, alloca);
            symbolNamePointersMap[name] = alloca;
        }
        else
        {
            symbolNamePointersMap[name] = arg;
            symbolNamePointerIntMap[name] = builder->CreatePtrToInt(arg, dataTypeToLLVMType(icode::I64));
        }
    }

    void LLVMTranslator::createPointer(const icode::Entry& e)
    {
        /* Converts mikuro CREATE_PTR to llvm ir */

        switch (e.op2.operandType)
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
            case icode::CALLEE_RET_VAL:
                operandValueMap[e.op1] = getCalleeRetValuePointer(e.op2);
                break;
            case icode::RET_PTR:
                operandValueMap[e.op1] = currentFunctionReturnPointer;
                break;
            default:
                miklog::internal_error(moduleDescription.name);
                throw miklog::internal_bug_error();
        }
    }

    void LLVMTranslator::copy(const icode::Entry& e)
    {
        /* Converts mikuro EQUAL to llvm ir */

        Value* sourceValue = getLLVMValue(e.op2);
        setLLVMValue(e.op1, sourceValue);
    }

    void LLVMTranslator::read(const icode::Entry& e)
    {
        /* Converts mikuro READ to llvm ir */

        Value* sourcePointer = getLLVMPointer(e.op2);
        Value* sourceValue = builder->CreateLoad(sourcePointer);

        setLLVMValue(e.op1, sourceValue);
    }

    void LLVMTranslator::write(const icode::Entry& e)
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

    void LLVMTranslator::addressBinaryOperator(const icode::Entry& e)
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

    Value* LLVMTranslator::add(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro ADD to llvm ir */

        if (icode::isSignedInteger(dtype))
            return builder->CreateNSWAdd(LHS, RHS);

        if (icode::isUnsignedInteger(dtype))
            return builder->CreateNUWAdd(LHS, RHS);

        if (icode::isFloat(dtype))
            return builder->CreateFAdd(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::subtract(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro SUB to llvm ir */

        if (icode::isSignedInteger(dtype))
            return builder->CreateNSWSub(LHS, RHS);

        if (icode::isUnsignedInteger(dtype))
            return builder->CreateNUWSub(LHS, RHS);

        if (icode::isFloat(dtype))
            return builder->CreateFSub(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::multiply(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro MUL to llvm ir */

        if (icode::isSignedInteger(dtype))
            return builder->CreateNSWMul(LHS, RHS);

        if (icode::isUnsignedInteger(dtype))
            return builder->CreateNUWMul(LHS, RHS);

        if (icode::isFloat(dtype))
            return builder->CreateFMul(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::divide(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro DIV to llvm ir */

        if (icode::isSignedInteger(dtype))
            return builder->CreateSDiv(LHS, RHS);

        if (icode::isUnsignedInteger(dtype))
            return builder->CreateUDiv(LHS, RHS);

        if (icode::isFloat(dtype))
            return builder->CreateFDiv(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::remainder(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro MOD to llvm ir */

        if (icode::isSignedInteger(dtype))
            return builder->CreateSRem(LHS, RHS);

        if (icode::isUnsignedInteger(dtype))
            return builder->CreateURem(LHS, RHS);

        if (icode::isFloat(dtype))
            return builder->CreateFRem(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::rightShift(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro RSH to llvm ir */

        if (icode::isSignedInteger(dtype))
            return builder->CreateAShr(LHS, RHS);

        if (icode::isUnsignedInteger(dtype))
            return builder->CreateLShr(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::leftShift(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro LSH to llvm ir */

        if (icode::isInteger(dtype))
            return builder->CreateShl(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::bitwiseAnd(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro BWA to llvm ir */

        if (icode::isInteger(dtype))
            return builder->CreateAnd(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::bitwiseOr(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro BWO to llvm ir */

        if (icode::isInteger(dtype))
            return builder->CreateOr(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::bitwiseXor(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro BWX to llvm ir */

        if (icode::isInteger(dtype))
            return builder->CreateXor(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    void LLVMTranslator::binaryOperator(const icode::Entry& e)
    {
        Value* LHS = getLLVMValue(e.op2);
        Value* RHS = getLLVMValue(e.op3);
        icode::DataType dtype = e.op1.dtype;

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

    void LLVMTranslator::bitwiseNot(const icode::Entry& e)
    {
        Value* result = builder->CreateNot(getLLVMValue(e.op2));

        setLLVMValue(e.op1, result);
    }

    void LLVMTranslator::unaryMinus(const icode::Entry& e)
    {
        Value* result;

        if (icode::isInteger(e.op2.dtype))
            result = builder->CreateNeg(getLLVMValue(e.op2));
        else
            result = builder->CreateFNeg(getLLVMValue(e.op2));

        setLLVMValue(e.op1, result);
    }

    Value* LLVMTranslator::castToSignedInt(const icode::Entry& e, Type* destType)
    {
        if (icode::isSignedInteger(e.op2.dtype))
            return builder->CreateSExtOrTrunc(getLLVMValue(e.op2), destType);

        if (icode::isUnsignedInteger(e.op2.dtype))
            return builder->CreateZExtOrTrunc(getLLVMValue(e.op2), destType);

        if (icode::isFloat(e.op2.dtype))
            return builder->CreateFPToSI(getLLVMValue(e.op2), destType);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::castToUnsignedInt(const icode::Entry& e, Type* destType)
    {
        if (icode::isInteger(e.op2.dtype))
            return builder->CreateZExtOrTrunc(getLLVMValue(e.op2), destType);

        if (icode::isFloat(e.op2.dtype))
            return builder->CreateFPToUI(getLLVMValue(e.op2), destType);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::castToFloatFromInt(const icode::Entry& e, Type* destType)
    {
        if (icode::isSignedInteger(e.op2.dtype))
            return builder->CreateSIToFP(getLLVMValue(e.op2), destType);

        if (icode::isUnsignedInteger(e.op2.dtype))
            return builder->CreateUIToFP(getLLVMValue(e.op2), destType);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::castToFloatFromFloat(const icode::Entry& e, Type* destType)
    {
        if (icode::getDataTypeSize(e.op1.dtype) > icode::getDataTypeSize(e.op2.dtype))
            return builder->CreateFPExt(getLLVMValue(e.op2), destType);

        if (icode::getDataTypeSize(e.op1.dtype) < icode::getDataTypeSize(e.op2.dtype))
            return builder->CreateFPTrunc(getLLVMValue(e.op2), destType);

        if (icode::getDataTypeSize(e.op1.dtype) == icode::getDataTypeSize(e.op2.dtype))
            return getLLVMValue(e.op2);
    
        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::castToFloat(const icode::Entry& e, Type* destType)
    {
        if (icode::isInteger(e.op2.dtype))
            return castToFloatFromInt(e, destType);

        if (icode::isFloat(e.op2.dtype))
            return castToFloatFromFloat(e, destType);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    void LLVMTranslator::cast(const icode::Entry& e)
    {
        Type* destType = dataTypeToLLVMType(e.op1.dtype);
        Value* result;

        if (icode::isSignedInteger(e.op1.dtype))
            result = castToSignedInt(e, destType);
        else if (icode::isUnsignedInteger(e.op1.dtype))
            result = castToUnsignedInt(e, destType);
        else if (icode::isFloat(e.op1.dtype))
            result = castToFloat(e, destType);
        else
        {
            miklog::internal_error(moduleDescription.name);
            throw miklog::internal_bug_error();
        }

        setLLVMValue(e.op1, result);
    }

    Value* LLVMTranslator::equal(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro EQ to llvm ir */

        if (icode::isInteger(dtype))
            return builder->CreateICmpEQ(LHS, RHS);

        if (icode::isFloat(dtype))
            return builder->CreateFCmpUEQ(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::notEqual(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro NEQ to llvm ir */

        if (icode::isInteger(dtype))
            return builder->CreateICmpNE(LHS, RHS);

        if (icode::isFloat(dtype))
            return builder->CreateFCmpUNE(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::lessThan(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro LT to llvm ir */

        if (icode::isSignedInteger(dtype))
            return builder->CreateICmpSLT(LHS, RHS);

        if (icode::isUnsignedInteger(dtype))
            return builder->CreateICmpULT(LHS, RHS);

        if (icode::isFloat(dtype))
            return builder->CreateFCmpULT(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::lessThanOrEqualTo(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro LTE to llvm ir */

        if (icode::isSignedInteger(dtype))
            return builder->CreateICmpSLE(LHS, RHS);

        if (icode::isUnsignedInteger(dtype))
            return builder->CreateICmpULE(LHS, RHS);

        if (icode::isFloat(dtype))
            return builder->CreateFCmpULE(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::greaterThan(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro GT to llvm ir */

        if (icode::isSignedInteger(dtype))
            return builder->CreateICmpSGT(LHS, RHS);

        if (icode::isUnsignedInteger(dtype))
            return builder->CreateICmpUGT(LHS, RHS);

        if (icode::isFloat(dtype))
            return builder->CreateFCmpUGT(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    Value* LLVMTranslator::greaterThanOrEqualTo(Value* LHS, Value* RHS, const icode::DataType dtype)
    {
        /* Converts mikuro GTE to llvm ir */

        if (icode::isSignedInteger(dtype))
            return builder->CreateICmpSGE(LHS, RHS);

        if (icode::isUnsignedInteger(dtype))
            return builder->CreateICmpUGE(LHS, RHS);

        if (icode::isFloat(dtype))
            return builder->CreateFCmpUGE(LHS, RHS);

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    void LLVMTranslator::compareOperator(const icode::Entry& e)
    {
        Value* LHS = getLLVMValue(e.op1);
        Value* RHS = getLLVMValue(e.op2);
        icode::DataType dtype = e.op1.dtype;

        switch (e.opcode)
        {
            case icode::EQ:
                branchFlags.push(equal(LHS, RHS, dtype));
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

    void LLVMTranslator::createLabel(const icode::Entry& e, Function* function)
    {
        /* Converts mikuro CREATE_LABEL to llvm basic block  */
        BasicBlock* newBlock = BasicBlock::Create(*context, e.op1.name, function);

        labelToBasicBlockMap[e.op1] = newBlock;

        /* Make sure every block has a terminator */
        if (!prevInstructionGotoOrRet)
            builder->CreateBr(newBlock);

        builder->SetInsertPoint(newBlock);
    }

    void LLVMTranslator::createGotoBackpatch(const icode::Entry& e, Function* F, size_t entryIndex)
    {
        /* All branch instructions GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO are backpatched,
            i.e the task is stored in a queue and instructions are created in a second pass */

        /* To prevent duplicate block terminators */
        if (prevInstructionGotoOrRet)
            return;

        /* Save llvm insertion point corresponding to this entry */
        insertionPoints[entryIndex] = InsertionPoint(builder->GetInsertBlock(), builder->GetInsertPoint()++);

        /* Append to backpatch queue */
        backpatchQueue.push_back(EnumeratedEntry(entryIndex, e));

        if (e.opcode == icode::GOTO)
            return;

        /* Create basic block for fall through for IF_TRUE_GOTO, IF_FALSE_GOTO */
        BasicBlock* fallBlock = BasicBlock::Create(*context, "_fall_e" + std::to_string(entryIndex), F);

        fallBlocks[entryIndex] = fallBlock;

        /* Start inserstion in fall through block */
        builder->SetInsertPoint(fallBlock);
    }

    void LLVMTranslator::createBranch(const icode::Entry& e, Value* flag, BasicBlock* gotoBlock, BasicBlock* fallBlock)
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
            icode::Entry& e = enumeratedEntry.second;

            /* Get branch flags and blocks for the goto */
            BasicBlock* gotoBlock = labelToBasicBlockMap[e.op1];
            BasicBlock* fallBlock = fallBlocks[entryIndex];
            Value* flag = branchFlags.front();

            /* Get insertion point corresponding to the entry */
            InsertionPoint insertPoint = insertionPoints[entryIndex];
            builder->SetInsertPoint(insertPoint.first, insertPoint.second);

            /* Create branch in the ^ insertion point */
            createBranch(e, flag, gotoBlock, fallBlock);

            /* Pop from flags queue after processing branch */
            if (e.opcode != icode::GOTO)
                branchFlags.pop();
        }
    }

    Value* LLVMTranslator::getFromatString(icode::DataType dtype)
    {
        if (icode::isUnsignedInteger(dtype))
            return uintFormatString;

        if (icode::isSignedInteger(dtype))
            return intFormatString;

        if (icode::isFloat(dtype))
            return floatFormatString;

        miklog::internal_error(moduleDescription.name);
        throw miklog::internal_bug_error();
    }

    void LLVMTranslator::callPrintf(Value* formatString, Value* value)
    {
        /* Set up printf arguments*/
        std::vector<Value*> printArgs;
        printArgs.push_back(formatString);
        if (value)
            printArgs.push_back(value);

        /* Call printf */
        builder->CreateCall(LLVMModule->getFunction("printf"), printArgs);
    }

    void LLVMTranslator::print(const icode::Entry& e)
    {
        Value* value = getLLVMValue(e.op1);

        /* Cast value to double if float */
        if (icode::isFloat(e.op1.dtype))
            value = builder->CreateFPCast(value, Type::getDoubleTy(*context));
        else
            value = builder->CreateSExt(value, Type::getInt32Ty(*context));

        callPrintf(getFromatString(e.op1.dtype), value);
    }

    void LLVMTranslator::printString(const icode::Entry& e)
    {
        Value* str_value = getLLVMPointer(e.op1);

        callPrintf(str_value);
    }

    void LLVMTranslator::call(const icode::Entry& e)
    {
        Value* result = builder->CreateCall(getLLVMFunction(e.op2.name, e.op3.name), params);

        setLLVMValue(e.op1, result);

        params.clear();
    }

    void LLVMTranslator::ret(const icode::Entry& e, icode::DataType dtype)
    {
        if (dtype == icode::VOID)
            builder->CreateRetVoid();
        else
            builder->CreateRet(builder->CreateLoad(currentFunctionReturnPointer));
    }

    void LLVMTranslator::pass(const icode::Entry& e) { params.push_back(getLLVMValue(e.op1)); }

    void LLVMTranslator::passPointer(const icode::Entry& e) { params.push_back(getLLVMPointer(e.op1)); }

    void LLVMTranslator::translateFunctionIcode(const icode::FunctionDescription& functionDesc, Function* F)
    {
        /* Go through icode and generate llvm ir */
        for (size_t i = 0; i < functionDesc.icodeTable.size(); i++)
        {
            icode::Entry e = functionDesc.icodeTable[i];

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
                    ret(e, functionDesc.functionReturnDescription.dtype);
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

    void LLVMTranslator::setupFunctionStack(const icode::FunctionDescription& functionDesc, Function* F)
    {
        /* Allocate space for local variables */
        for (auto symbol : functionDesc.symbols)
            if (!symbol.second.checkProperty(icode::IS_PARAM))
                createLocalSymbol(symbol.second, symbol.first);

        /* Assign passed function args */
        unsigned int argumentCounter = 0;
        for (auto& arg : F->args())
        {
            const std::string& argumentName = functionDesc.parameters[argumentCounter];
            arg.setName(argumentName);
            createFunctionParameter(functionDesc.symbols.at(argumentName), argumentName, &arg);
            argumentCounter++;
        }
    }

    void LLVMTranslator::generateFunction(const icode::FunctionDescription& functionDesc, const std::string& name)
    {
        resetState();

        Function* function = getLLVMFunction(name, moduleDescription.name);

        /* Set insertion point to function body */
        BasicBlock* functionBlock = BasicBlock::Create(*context, "entry", function);
        builder->SetInsertPoint(functionBlock);

        setupFunctionStack(functionDesc, function);

        /* Set ret ptr */
        if (functionDesc.functionReturnDescription.dtype != icode::VOID)
            currentFunctionReturnPointer =
              builder->CreateAlloca(variableDescriptionToLLVMType(functionDesc.functionReturnDescription),
                                    nullptr,
                                    name + ".retPointer");

        /* Convert mikuro function ir to llvm ir */
        translateFunctionIcode(functionDesc, function);

        /* Process goto backpacthing */
        processGotoBackpatches();

        verifyFunction(*function);
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

    void LLVMTranslator::setupContextAndModule()
    {
        context = std::make_unique<LLVMContext>();
        LLVMModule = std::make_unique<Module>(moduleDescription.name, *context);
        builder = std::make_unique<IRBuilder<>>(*context);
        prevInstructionGotoOrRet = false;
    }

    void LLVMTranslator::generateModule()
    {
        for (auto symbol : moduleDescription.globals)
            createGlobalSymbol(symbol.second, symbol.first);

        setupPrintf();

        for (auto func : moduleDescription.functions)
            generateFunction(func.second, func.first);
    }

    void LLVMTranslator::initializeTargetRegistry()
    {
        InitializeAllTargetInfos();
        InitializeAllTargets();
        InitializeAllTargetMCs();
        InitializeAllAsmParsers();
        InitializeAllAsmPrinters();
    }

    TargetMachine* LLVMTranslator::setupTargetTripleAndDataLayout()
    {
        auto targetTriple = sys::getDefaultTargetTriple();
        LLVMModule->setTargetTriple(targetTriple);

        std::string error;
        auto Target = TargetRegistry::lookupTarget(targetTriple, error);

        if (!Target)
        {
            miklog::println(error);
            throw miklog::internal_bug_error();
        }

        std::string CPU = "generic";
        std::string features = "";

        TargetOptions opt;
        auto RM = Optional<Reloc::Model>();
        auto targetMachine = Target->createTargetMachine(targetTriple, CPU, features, opt, RM);

        LLVMModule->setDataLayout(targetMachine->createDataLayout());

        return targetMachine;
    }

    void LLVMTranslator::setupPassManagerAndCreateObject(TargetMachine* targetMachine)
    {
        auto filename = moduleDescription.name + ".o";
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

    LLVMTranslator::LLVMTranslator(icode::ModuleDescription& moduleDesc, icode::StringModulesMap& modulesMap)
      : moduleDescription(moduleDesc)
      , externalModulesRef(modulesMap)
    {
        setupContextAndModule();
        setupPrintf();
        generateModule();
        initializeTargetRegistry();
        TargetMachine* targetMachine = setupTargetTripleAndDataLayout();
        setupPassManagerAndCreateObject(targetMachine);
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