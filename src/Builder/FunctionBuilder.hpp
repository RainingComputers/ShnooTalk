#ifndef BUILDER_ENTRY_BUILDER
#define BUILDER_ENTRY_BUILDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"
#include "../Token/Token.hpp"
#include "OperandBuilder.hpp"
#include "Unit.hpp"
#include "UnitBuilder.hpp"

class FunctionBuilder
{

    icode::StringModulesMap& modulesMap;
    OperandBuilder& opBuilder;
    UnitBuilder& unitBuilder;
    Console& console;

    icode::FunctionDescription* workingFunction;

    void pushEntry(icode::Entry entry);

    icode::Operand getCreatePointerDestinationOperand(const Unit& unit);

    icode::Operand createPointer(const Unit& unit);

    icode::Operand autoCast(const icode::Operand& op, icode::DataType destinationDataType);

    void operandCopy(icode::Operand op1, icode::Operand op2);

    void memCopy(icode::Operand op1, icode::Operand op2, int numBytes);

    void unitListCopy(const Unit& dest, const Unit& src);

    icode::Operand ensureNotPointer(icode::Operand op);

    icode::Operand pushEntryAndEnsureNoPointerWrite(icode::Entry entry);

    icode::Operand addressAddOperator(icode::Operand op2, icode::Operand op3);

    icode::Operand addressMultiplyOperator(icode::Operand op2, icode::Operand op3);

    bool doesFunctionTerminate();

  public:
    FunctionBuilder(icode::StringModulesMap& modulesMap,
                    OperandBuilder& opBuilder,
                    UnitBuilder& unitBuilder,
                    Console& console);

    void setWorkingFunction(icode::FunctionDescription* functionDesc);

    void unitCopy(const Unit& dest, const Unit& src);

    Unit binaryOperator(icode::Instruction instruction, const Unit& LHS, const Unit& RHS);

    Unit unaryOperator(icode::Instruction instruction, const Unit& unaryOperatorTerm);

    Unit castOperator(const Unit& unitToCast, icode::DataType destinationDataType);

    void compareOperator(icode::Instruction instruction, const Unit& LHS, const Unit& RHS);

    Unit getStructField(const Token& fieldName, const Unit& unit);

    Unit getIndexedElement(const Unit& unit, const std::vector<Unit>& indices);

    icode::Operand createLabel(const Token& tok, bool isTrueLabel, std::string prefix);

    void insertLabel(icode::Operand op);

    void createBranch(icode::Instruction instruction, icode::Operand op);

    void createPrint(const Unit& unit);

    void createInput(const Unit& unit);

    Unit createLocal(const Token nameToken, icode::TypeDescription& typeDescription);

    void passParameter(const Token& calleeNameToken,
                       icode::FunctionDescription callee,
                       const Unit& formalParam,
                       const Unit& actualParam);

    Unit callFunction(const Token& calleeNameToken, icode::FunctionDescription callee);

    void noArgumentEntry(icode::Instruction instruction);

    Unit getReturnPointerUnit();

    void terminateFunction(const Token& nameToken);
};

#endif