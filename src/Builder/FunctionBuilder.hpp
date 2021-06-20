#ifndef FUNCTION_BUILDER
#define FUNCTION_BUILDER

#include "../Console/Console.hpp"
#include "../IntermediateRepresentation/All.hpp"
#include "EntryBuilder.hpp"
#include "UnitBuilder.hpp"

class FunctionBuilder
{
    icode::StringModulesMap& modulesMap;

    Console& console;
    OperandBuilder& opBuilder;
    EntryBuilder& entryBuilder;

    icode::Operand getPointerOperand(const Unit& unit);

  public:
    FunctionBuilder(icode::StringModulesMap& modulesMap,
                    Console& console,
                    OperandBuilder& opBuilder,
                    EntryBuilder& entryBuilder);

    Unit getStructField(const Token& fieldName, const Unit& unit);

    Unit getIndexedElement(const Unit& unit, const std::vector<Unit>& indices);

    Unit binaryOperator(icode::Instruction instruction, const Unit& LHS, const Unit& RHS);

    Unit castOperator(const Unit& unitToCast, icode::DataType destinationDataType);

    Unit unaryOperator(icode::Instruction instruction, const Unit& unaryOperatorTerm);

    icode::Operand createLabel(const Token& tok, bool isTrueLabel, std::string prefix);

    void insertLabel(const icode::Operand& label);

    void createIfTrueGoto(const icode::Operand& label);

    void createIfFalseGoto(const icode::Operand& label);

    void createGoto(const icode::Operand& label);

    void compareOperator(icode::Instruction instruction, const Unit& LHS, const Unit& RHS);

    void passParameter(const Token& calleeNameToken,
                       icode::FunctionDescription callee,
                       const Unit& formalParam,
                       const Unit& actualParam);

    Unit callFunction(const Token& calleeNameToken, icode::FunctionDescription callee);

    icode::Operand creatStringOperand(const Token& str_token, icode::DataType dtype);

    Unit createString(const Token& str_token);

    bool terminateFunction();

    void createInput(const Unit& unit);
};

#endif