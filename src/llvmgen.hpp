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
    typedef std::pair<size_t, icode::Entry> EnumeratedEntry;

    icode::TargetDescription getTargetDescription();

    class LLVMTranslator
    {
        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::Module> LLVMModule;
        std::unique_ptr<llvm::IRBuilder<>> builder;

        std::map<std::string, llvm::Value*> symbolNamePointersMap;
        std::map<std::string, llvm::Value*> symbolNamePointerIntMap;
        std::map<std::string, llvm::GlobalVariable*> symbolNameGlobalsMap;
        std::map<icode::Operand, llvm::Value*> operandValueMap;

        std::map<icode::Operand, llvm::BasicBlock*> labelToBasicBlockMap;
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

        llvm::Value* currentFunctionReturnPointer;

        bool prevInstructionGotoOrRet;

        icode::ModuleDescription& moduleDescription;
        icode::StringModulesMap& externalModulesRef;

        llvm::Type* dataTypeToLLVMType(const icode::DataType dtype);
        llvm::Type* dataTypeToLLVMPointerType(const icode::DataType dtype);
        llvm::Type* variableDescriptionToLLVMType(const icode::VariableDescription& variableDesc);
        llvm::FunctionType* funcDescriptionToLLVMType(icode::FunctionDescription& functionDesc);

        llvm::Value* getLLVMConstant(const icode::Operand& op);
        llvm::Function* getLLVMFunction(const std::string& functionName, const std::string& moduleName);
        llvm::Value* getCalleeRetValuePointer(const icode::Operand& op);
        llvm::Value* getLLVMPointer(const icode::Operand& op);
        llvm::Value* getLLVMValue(const icode::Operand& op);
        void setLLVMValue(const icode::Operand& op, llvm::Value* value);

        void createLocalSymbol(const icode::VariableDescription& variableDesc, const std::string& name);
        void createGlobalSymbol(icode::VariableDescription& variableDesc, const std::string& name);
        void createFunctionParameter(const icode::VariableDescription& variableDesc,
                                     const std::string& name,
                                     llvm::Value* arg);

        void createPointer(const icode::Entry& e);
        void copy(const icode::Entry& e);
        void read(const icode::Entry& e);
        void write(const icode::Entry& e);
        llvm::Value* ensureI64(llvm::Value* value);
        void addressBinaryOperator(const icode::Entry& e);

        llvm::Value* add(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* subtract(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* multiply(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* divide(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* remainder(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* rightShift(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* leftShift(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* bitwiseAnd(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* bitwiseOr(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* bitwiseXor(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        void binaryOperator(const icode::Entry& e);

        void bitwiseNot(const icode::Entry& e);
        void unaryMinus(const icode::Entry& e);

        llvm::Value* castToSignedInt(const icode::Entry& e, llvm::Type* destType);
        llvm::Value* castToUnsignedInt(const icode::Entry& e, llvm::Type* destType);
        llvm::Value* castToFloatFromInt(const icode::Entry& e, llvm::Type* destType);
        llvm::Value* castToFloatFromFloat(const icode::Entry& e, llvm::Type* destType);
        llvm::Value* castToFloat(const icode::Entry& e, llvm::Type* destType);
        void cast(const icode::Entry& e);

        llvm::Value* equal(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* notEqual(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* lessThan(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* lessThanOrEqualTo(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* greaterThan(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        llvm::Value* greaterThanOrEqualTo(llvm::Value* LHS, llvm::Value* RHS, const icode::DataType dtype);
        void compareOperator(const icode::Entry& e);

        void createLabel(const icode::Entry& e, llvm::Function* function);
        void createGotoBackpatch(const icode::Entry& e, llvm::Function* F, size_t entryIndex);
        void createBranch(const icode::Entry& e,
                          llvm::Value* flag,
                          llvm::BasicBlock* gotoBlock,
                          llvm::BasicBlock* fallBlock);
        void processGotoBackpatches();

        llvm::Value* getFromatString(icode::DataType dtype);
        void callPrintf(llvm::Value* formatString, llvm::Value* value = nullptr);
        void print(const icode::Entry& e);
        void printString(const icode::Entry& e);

        void call(const icode::Entry& e);
        void ret(const icode::Entry& e, icode::DataType dtype);
        void pass(const icode::Entry& e);
        void passPointer(const icode::Entry& e);

        void translateFunctionIcode(const icode::FunctionDescription& functionDesc, llvm::Function* function);

        void resetState();

        void setupFunctionStack(icode::FunctionDescription& functionDesc, llvm::Function* function);

        void generateFunction(icode::FunctionDescription& functionDesc, const std::string& name);

        void setupPrintf();

        void setupContextAndModule();

        void generateModule();

        void initializeTargetRegistry();
        llvm::TargetMachine* setupTargetTripleAndDataLayout();
        void setupPassManagerAndCreateObject(llvm::TargetMachine* targetMachine);

      public:
        std::string getLLVMModuleString();
        LLVMTranslator(icode::ModuleDescription& modDesc, icode::StringModulesMap& modulesMap);
    };

}

#endif