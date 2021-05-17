#ifndef IBUILD_HPP
#define IBUILD_HPP

#include "../IntermediateRepresentation/All.hpp"

namespace ibuild
{
    class IRBuilder
    {
        unsigned int idCounter;

        icode::FunctionDescription* functionDescriptionPointer;
        icode::ModuleDescription& module;

        icode::Operand ensureNotPointer(icode::Operand op);

        icode::Operand pushEntryAndEnsureNoPointerWrite(icode::Entry entry);

        icode::Operand getCreatePointerDestinationOperand(const icode::Operand& op);

      public:
        unsigned int id();

        void setFunctionDescription(icode::FunctionDescription* functionDesc);

        void pushEntry(icode::Entry entry);

        icode::Operand createPointer(const icode::Operand& op);

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

        icode::Operand call(const std::string& functoinName, const icode::FunctionDescription& functionDesc);

        void noArgumentEntry(icode::Instruction instruction);

        IRBuilder(icode::ModuleDescription& moduleDesc);
    };
}

#endif