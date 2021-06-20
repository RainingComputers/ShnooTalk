#ifndef BUILDER_ENTRY_BUILDER
#define BUILDER_ENTRY_BUILDER

#include "../Builder/OperandBuilder.hpp"
#include "../IntermediateRepresentation/All.hpp"

class EntryBuilder
{
    icode::FunctionDescription* workingFunction;
    OperandBuilder& opBuilder;

    icode::Operand ensureNotPointer(icode::Operand op);

    icode::Operand pushEntryAndEnsureNoPointerWrite(icode::Entry entry);

    icode::Operand getCreatePointerDestinationOperand(const icode::Operand& op,
                                                      const std::string& dtypeName,
                                                      icode::ModuleDescription* workingModule);

  public:
    EntryBuilder(OperandBuilder& opBuilder);

    void setWorkingFunction(icode::FunctionDescription* functionDesc);

    void pushEntry(icode::Entry entry);

    icode::Operand createPointer(const icode::Operand& op,
                                 const std::string& dtypeName,
                                 icode::ModuleDescription* workingModule);

    void copy(icode::Operand op1, icode::Operand op2);

    icode::Operand binaryOperator(icode::Instruction instruction,
                                  icode::Operand op1,
                                  icode::Operand op2,
                                  icode::Operand op3);

    icode::Operand unaryOperator(icode::Instruction instruction, icode::Operand op1, icode::Operand op2);

    icode::Operand castOperator(icode::DataType castDataType, icode::Operand op);

    void compareOperator(icode::Instruction instruction, icode::Operand op1, icode::Operand op2);

    icode::Operand addressAddOperator(icode::Operand op2, icode::Operand op3);

    icode::Operand addressMultiplyOperator(icode::Operand op2, icode::Operand op3);

    void label(icode::Operand op);

    void createBranch(icode::Instruction instruction, icode::Operand op);

    void printOperator(icode::Instruction printInstruction, icode::Operand op);

    void inputOperator(icode::Instruction inputInstruction, icode::Operand op, unsigned int size = 0);

    void pass(icode::Instruction passInstruction,
              icode::Operand op,
              const std::string& functionName,
              const icode::FunctionDescription& functionDesc);

    icode::Operand call(const std::string& functionName, const icode::FunctionDescription& functionDesc);

    void noArgumentEntry(icode::Instruction instruction);

    bool doesFunctionTerminate();

    bool terminateFunction();
};

#endif