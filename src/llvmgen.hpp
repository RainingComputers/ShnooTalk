#ifndef LLVMGEN_HPP
#define LLVMGEN_HPP

#include <map>
#include <queue>
#include <string>

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
    typedef std::pair<llvm::BasicBlock*, llvm::BasicBlock::iterator> InsertionPoint;
    typedef std::pair<size_t, icode::entry> EnumeratedEntry;

    icode::target_desc getTargetDescription();

    class LLVMTranslator
    {
        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::Module> LLVMModule;
        std::unique_ptr<llvm::IRBuilder<>> builder;

        std::map<std::string, llvm::Value*> symbolNamePointersMap;
        std::map<std::string, llvm::Value*> symbolNamePointerIntMap;
        std::map<std::string, llvm::GlobalVariable*> symbolNameGlobalsMap;
        std::map<icode::operand, llvm::Value*> operandValueMap;

        std::map<icode::operand, llvm::BasicBlock*> labelToBasicBlockMap;
        std::map<size_t, llvm::BasicBlock*> fallBlocks;
        std::queue<llvm::Value*> branchFlags;
        std::map<size_t, InsertionPoint> insertionPoints;
        std::vector<EnumeratedEntry> backpatchQueue;

        std::vector<llvm::Value*> params;

        llvm::Value* uintFormatString;
        llvm::Value* intFormatString;
        llvm::Value* floatFormatString;
        llvm::Value* newLineString;
        llvm::Value* spaceString;

        llvm::Value* currentFunctionReturnValue;

        bool prevInstructionGotoOrRet;

        icode::module_desc& moduleDescription;
        icode::module_desc_map& externalModulesRef;

        llvm::Type* dataTypeToLLVMType(const icode::data_type dtype);
        llvm::Type* dataTypeToLLVMPointerType(const icode::data_type dtype);
        llvm::Type* varDescriptionToLLVMType(const icode::var_info& varDesc);
        llvm::FunctionType* funcDescriptionToLLVMType(icode::func_desc& funcDescription);

        llvm::Value* getLLVMConstant(const icode::operand& op);
        llvm::Function* getLLVMFunction(const std::string& functionName, const std::string& moduleName);
        llvm::Value* getCurrentRetValuePointer(const icode::operand& op);
        llvm::Value* getLLVMPointer(const icode::operand& op);
        llvm::Value* getLLVMValue(const icode::operand& op);
        void setLLVMValue(const icode::operand& op, llvm::Value* value);

        void createLocalSymbol(const icode::var_info& varDescription, const std::string& name);
        void createGlobalSymbol(icode::var_info& varDescription, const std::string& name);
        void createFunctionParameter(const icode::var_info& varDescription, const std::string& name, llvm::Value* arg);

        void createPointer(const icode::entry& e);
        void copy(const icode::entry& e);
        void read(const icode::entry& e);
        void write(const icode::entry& e);
        llvm::Value* ensureI64(llvm::Value* value);
        void addressBinaryOperator(const icode::entry& e);

        llvm::Value* add(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* subtract(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* multiply(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* divide(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* remainder(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* rightShift(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* leftShift(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* bitwiseAnd(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* bitwiseOr(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* bitwiseXor(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        void binaryOperator(const icode::entry& e);

        void bitwiseNot(const icode::entry& e);
        void unaryMinus(const icode::entry& e);

        llvm::Value* castToSignedInt(const icode::entry& e, llvm::Type* destType);
        llvm::Value* castToUnsignedInt(const icode::entry& e, llvm::Type* destType);
        llvm::Value* castToFloat(const icode::entry& e, llvm::Type* destType);
        void cast(const icode::entry& e);

        llvm::Value* eqaul(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* notEqual(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* lessThan(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* lessThanOrEqualTo(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* greaterThan(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        llvm::Value* greaterThanOrEqualTo(llvm::Value* LHS, llvm::Value* RHS, const icode::data_type dtype);
        void compareOperator(const icode::entry& e);

        void createLabel(const icode::entry& e, llvm::Function* function);
        void createGotoBackpatch(const icode::entry& e, llvm::Function* F, size_t entryIndex);
        void createBranch(const icode::entry& e,
                          llvm::Value* flag,
                          llvm::BasicBlock* gotoBlock,
                          llvm::BasicBlock* fallBlock);
        void processGotoBackpatches();

        llvm::Value* getFromatString(icode::data_type dtype);
        void callPrintf(llvm::Value* format_str, llvm::Value* value = nullptr);
        void print(const icode::entry& e);
        void printString(const icode::entry& e);

        void call(const icode::entry& e);
        void ret(const icode::entry& e, icode::data_type dtype);
        void pass(const icode::entry& e);
        void passPointer(const icode::entry& e);

        void translateFunctionIcode(const icode::func_desc& funcDescription, llvm::Function* F);

        void resetState();

        void setupFunctionStack(icode::func_desc& funcDescription, llvm::Function* F);

        void generateFunction(icode::func_desc& funcDescription, const std::string& name);

        void generateGlobals();

        void setupPrintf();

      public:
        std::string getLLVMModuleString();
        LLVMTranslator(icode::module_desc& modDesc, icode::module_desc_map& modulesMap);
    };

}

#endif