#ifndef IBUILD_HPP
#define IBUILD_HPP

#include "icode.hpp"

namespace ibuild
{
    class ir_builder
    {
        unsigned int id_counter;

        icode::func_desc* current_func_desc;
        icode::module_desc& module;

      public:
        unsigned int id();

        void set_func_desc(icode::func_desc* func_desc);

        void push_ir(icode::entry entry);

        icode::operand create_ptr(const icode::operand& op);

        void copy(icode::operand op1, icode::operand op2);

        icode::operand ensure_not_ptr(icode::operand op);

        icode::operand push_ir_ensure_no_write_ptr(icode::entry entry);

        icode::operand binop(icode::instruction instr,
                             icode::operand op1,
                             icode::operand op2,
                             icode::operand op3);

        icode::operand
        uniop(icode::instruction instr, icode::operand op1, icode::operand op2);

        icode::operand cast(icode::data_type cast_dtype, icode::operand op);

        void cmpop(icode::instruction instr, icode::operand op1, icode::operand op2);

        icode::operand
        addrop(icode::instruction instr, icode::operand op2, icode::operand op3);

        void label(icode::operand op);

        void goto_label(icode::instruction instr, icode::operand op);

        void printop(icode::instruction printop, icode::operand op);

        void inputop(icode::instruction instr, icode::operand op, unsigned int size = 0);

        void pass(icode::instruction pass_instr,
                  icode::operand op,
                  const std::string& func_name,
                  const icode::func_desc& func_desc);

        icode::operand
        call(const std::string& func_name, const icode::func_desc& func_desc);

        void opir(icode::instruction instr);

        ir_builder(icode::module_desc& module_desc);
    };
}

#endif