#ifndef IBUILD_HPP
#define IBUILD_HPP

#include "IntermediateRepresentation/All.hpp"

namespace ibuild
{
    class ir_builder
    {
        unsigned int id_counter;

        icode::FunctionDescription* current_func_desc;
        icode::ModuleDescription& module;

      public:
        unsigned int id();

        void set_func_desc(icode::FunctionDescription* func_desc);

        void push_ir(icode::Entry entry);

        icode::Operand create_ptr(const icode::Operand& op);

        void copy(icode::Operand op1, icode::Operand op2);

        icode::Operand ensure_not_ptr(icode::Operand op);

        icode::Operand push_ir_ensure_no_write_ptr(icode::Entry entry);

        icode::Operand binop(icode::Instruction instr, icode::Operand op1, icode::Operand op2, icode::Operand op3);

        icode::Operand uniop(icode::Instruction instr, icode::Operand op1, icode::Operand op2);

        icode::Operand cast(icode::DataType cast_dtype, icode::Operand op);

        void cmpop(icode::Instruction instr, icode::Operand op1, icode::Operand op2);

        icode::Operand addr_add(icode::Operand op2, icode::Operand op3);

        icode::Operand addr_mul(icode::Operand op2, icode::Operand op3);

        void label(icode::Operand op);

        void goto_label(icode::Instruction instr, icode::Operand op);

        void printop(icode::Instruction printop, icode::Operand op);

        void inputop(icode::Instruction instr, icode::Operand op, unsigned int size = 0);

        void pass(icode::Instruction pass_instr,
                  icode::Operand op,
                  const std::string& func_name,
                  const icode::FunctionDescription& func_desc);

        icode::Operand call(const std::string& func_name, const icode::FunctionDescription& func_desc);

        void opir(icode::Instruction instr);

        ir_builder(icode::ModuleDescription& module_desc);
    };
}

#endif