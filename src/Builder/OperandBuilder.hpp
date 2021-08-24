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

    icode::Operand createStringDataOperand(const std::string& name, unsigned long size);

    icode::Operand createVarOperand(icode::DataType dtype,
                                    const std::string& name,
                                    icode::OperandType type = icode::VAR);

    icode::Operand createRetPointerOperand(icode::DataType dtype);

    icode::Operand createCalleeRetValOperand(icode::DataType dtype);

    icode::Operand createFloatLiteralOperand(icode::DataType dtype, double literal);

    icode::Operand createIntLiteralOperand(icode::DataType dtype, long literal);

    icode::Operand createBytesOperand(unsigned long address);

    icode::Operand createLabelOperand(const std::string& label);

    icode::Operand createModuleOperand(const std::string& module);

    icode::Operand operandFromTypeDescription(const icode::TypeDescription& typeDescription, const std::string& name);
};

#endif
