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

    icode::Operand getCreatePointerDestinationOperand(const icode::Operand& op,
                                                      const std::string& dtypeName,
                                                      icode::ModuleDescription* workingModule);

    icode::Operand ensureNotPointer(icode::Operand op);

    icode::Operand pushEntryAndEnsureNoPointerWrite(icode::Entry entry);

    bool doesFunctionTerminate();

  public:
    FunctionBuilder(icode::StringModulesMap& modulesMap,
                    OperandBuilder& opBuilder,
                    UnitBuilder& unitBuilder,
                    Console& console);

    void setWorkingFunction(icode::FunctionDescription* functionDesc);

    void pushEntry(icode::Entry entry);

    icode::Operand createPointer(const icode::Operand& op,
                                 const std::string& dtypeName,
                                 icode::ModuleDescription* workingModule);

    icode::Operand getPointerOperand(const Unit& unit);

    void operandCopy(icode::Operand op1, icode::Operand op2);

    void memCopy(icode::Operand op1, icode::Operand op2, int numBytes);

    void unitCopy(const Unit& dest, const Unit& src);

    Unit binaryOperator(icode::Instruction instruction, const Unit& LHS, const Unit& RHS);

    Unit unaryOperator(icode::Instruction instruction, const Unit& unaryOperatorTerm);

    Unit castOperator(const Unit& unitToCast, icode::DataType destinationDataType);

    void compareOperator(icode::Instruction instruction, const Unit& LHS, const Unit& RHS);

    icode::Operand addressAddOperator(icode::Operand op2, icode::Operand op3);

    icode::Operand addressMultiplyOperator(icode::Operand op2, icode::Operand op3);

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

    bool terminateFunction();
};

#endif