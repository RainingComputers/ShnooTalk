#ifndef BUILDER_OPERAND_BUILDER
#define BUILDER_OPERAND_BUILDER

#include "../IntermediateRepresentation/All.hpp"

class OperandBuilder
{
    unsigned int idCounter;

  public:
    OperandBuilder();

    unsigned int getId();

    icode::Operand createTempOperand(icode::DataType dtype);

    icode::Operand createPointerOperand(icode::DataType dtype);

    icode::Operand createStringDataOperand(const std::string& name, unsigned int size);

    icode::Operand createVarOperand(icode::DataType dtype,
                                    const std::string& symbol,
                                    bool global = false,
                                    bool ptr = false);

    icode::Operand createRetPointerOperand(icode::DataType dtype);

    icode::Operand createCalleeRetValOperand(icode::DataType dtype);

    icode::Operand createFloatLiteralOperand(icode::DataType dtype, float literal);

    icode::Operand createIntLiteralOperand(icode::DataType dtype, int literal);

    icode::Operand createLiteralAddressOperand(unsigned int address);

    icode::Operand createLabelOperand(const std::string& label);

    icode::Operand createModuleOperand(const std::string& module);

    icode::Operand operandFromTypeDescription(const icode::TypeDescription& typeDescription, const std::string& name);

    icode::Operand operandFromIntLiteral(int value);

    icode::Operand operandFromFloatLiteral(float value);
};

#endif
