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

    icode::Operand autoCast(const icode::Operand& op, icode::DataType destinationDataType);

    icode::Operand ensureNotPointer(icode::Operand op);

    void operandCopy(icode::Operand op1, icode::Operand op2);

    void memCopy(icode::Operand op1, icode::Operand op2, int numBytes);

    icode::Operand getCreatePointerDestOperand(const icode::TypeDescription& type);

    icode::Operand createPointer(const Unit& unit);

    Unit createTempArray(const icode::TypeDescription& type, unsigned int bytes);

    void unitListCopy(const Unit& dest, const Unit& src);

    icode::Operand addressAddOperator(icode::Operand op2, icode::Operand op3);

    icode::Operand addressAddOperatorPtrPtr(icode::Operand op2, icode::Operand op3);

    icode::Operand addressMultiplyOperator(icode::Operand op2, icode::Operand op3);

    std::string getMangledCalleeName(const Token& calleeNameToken, const icode::FunctionDescription& callee);

    icode::Operand createPointerForPassAddress(const Unit& actualParam, const Unit& formalParam);

    bool doesFunctionTerminate();

public:
    FunctionBuilder(icode::StringModulesMap& modulesMap,
                    OperandBuilder& opBuilder,
                    UnitBuilder& unitBuilder,
                    Console& console);

    void setWorkingFunction(icode::FunctionDescription* functionDesc);

    Unit createTemp(icode::DataType dtype);

    void unitCopy(const Unit& dest, const Unit& src);

    void unitPointerAssign(const Unit& to, const Unit& src);

    Unit binaryOperator(icode::Instruction instruction, const Unit& LHS, const Unit& RHS);

    Unit unaryOperator(icode::Instruction instruction, const Unit& unaryOperatorTerm);

    Unit castOperator(const Unit& unitToCast, icode::DataType destinationDataType);

    Unit addrOperator(const Unit& unit);

    Unit pointerCastOperator(const Unit& unitToCast, icode::TypeDescription destinationType);

    void compareOperator(icode::Instruction instruction, const Unit& LHS, const Unit& RHS);

    Unit getStructField(const Token& fieldName, const Unit& unit);

    Unit getIndexedElement(const Unit& unit, const std::vector<Unit>& indices);

    icode::Operand createLabel(const Token& tok, bool isTrueLabel, std::string prefix);

    void insertLabel(icode::Operand op);

    void createBranch(icode::Instruction instruction, icode::Operand op);

    void createPrint(const Unit& unit);

    void createInput(const Unit& unit);

    Unit createLocal(const Token nameToken, icode::TypeDescription& typeDescription);

    void passParameterPreMangled(const std::string& mangledCalleeName,
                       icode::FunctionDescription callee,
                       const Unit& formalParam,
                       const Unit& actualParam);

    void passParameter(const Token& calleeNameToken,
                       icode::FunctionDescription callee,
                       const Unit& formalParam,
                       const Unit& actualParam);

    Unit callFunctionPreMangled(const std::string& mangledCalleeName, const icode::FunctionDescription& callee);

    Unit callFunction(const Token& calleeNameToken, const icode::FunctionDescription& callee);

    void noArgumentEntry(icode::Instruction instruction);

    Unit getReturnValueUnit();

    void terminateFunction(const Token& nameToken);
};

#endif