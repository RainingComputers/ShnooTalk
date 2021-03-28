#include "ibuild.hpp"

namespace ibuild
{
    ir_builder::ir_builder() { id_counter = 0; }

    unsigned int ir_builder::id() { return id_counter++; }

    void ir_builder::set_func_desc(icode::func_desc* func_desc)
    {
        current_func_desc = func_desc;
    }

    void ir_builder::push_ir(icode::entry entry)
    {
        /* Push an ir entry to the current function's icode table */

        (*current_func_desc).icode_table.push_back(entry);
    }

    void ir_builder::copy(icode::operand op1, icode::operand op2)
    {
        /* If op2 is a literal, change  generic dtypes like icode::INT and icode::FLOAT
            to correct specific dtype */
        if (op2.optype == icode::LITERAL)
            op2.dtype = op1.dtype;

        /* Copy one operand value to another, use READ and WRITE instruction
            if pointers are involved */

        if (icode::is_ptr(op1.optype) && icode::is_ptr(op2.optype))
        {
            icode::operand temp = icode::temp_opr(op2.dtype, id());
            copy(temp, op2);
            copy(op1, temp);
        }
        else
        {
            icode::entry copy_entry;
            copy_entry.op1 = op1;
            copy_entry.op2 = op2;

            if (icode::is_ptr(op1.optype) && !icode::is_ptr(op2.optype))
                copy_entry.opcode = icode::WRITE;
            else if (!icode::is_ptr(op1.optype) && icode::is_ptr(op2.optype))
                copy_entry.opcode = icode::READ;
            else if (!icode::is_ptr(op1.optype) && !icode::is_ptr(op2.optype))
                copy_entry.opcode = icode::EQUAL;

            push_ir(copy_entry);
        }
    }

    icode::operand ir_builder::ensure_not_ptr(icode::operand op)
    {
        /* Make sure the operand is not a pointer, if it is a pointer,
            converts it to a temp using the READ instruction */

        if (icode::is_ptr(op.optype))
        {
            icode::entry read_entry;
            icode::operand temp = icode::temp_opr(op.dtype, id());
            copy(temp, op);
            return temp;
        }
        else
            return op;
    }

    icode::operand ir_builder::push_ir_ensure_no_write_ptr(icode::entry entry)
    {
        /* Push an ir entry to the current function's icode table,
            but ensures entry.op1 is not a pointer */

        if (!icode::is_ptr(entry.op1.optype))
        {
            push_ir(entry);
            return entry.op1;
        }

        /* If entry.op1 is a pointer, repleace it with a temp and
            write that temp to the pointer */

        icode::operand ptr_op = entry.op1;
        icode::operand temp = icode::temp_opr(ptr_op.dtype, id());

        icode::entry mod_entry = entry;
        mod_entry.op1 = temp;
        push_ir(mod_entry);

        icode::entry write_entry;
        write_entry.op1 = ptr_op;
        write_entry.op2 = temp;
        write_entry.opcode = icode::WRITE;
        push_ir(write_entry);

        return temp;
    }

    icode::operand ir_builder::binop(icode::instruction instr,
                                     icode::operand op1,
                                     icode::operand op2,
                                     icode::operand op3)
    {
        icode::entry entry;
        entry.opcode = instr;
        entry.op1 = op1;
        entry.op2 = ensure_not_ptr(op2);
        entry.op3 = ensure_not_ptr(op3);

        return push_ir_ensure_no_write_ptr(entry);
    }

    icode::operand
    ir_builder::uniop(icode::instruction instr, icode::operand op1, icode::operand op2)
    {
        icode::entry entry;
        entry.opcode = instr;
        entry.op1 = op1;
        entry.op2 = ensure_not_ptr(op2);

        return push_ir_ensure_no_write_ptr(entry);
    }

    icode::operand ir_builder::cast(icode::data_type cast_dtype, icode::operand op)
    {
        icode::entry entry;
        entry.opcode = icode::CAST;
        entry.op1 = icode::temp_opr(cast_dtype, id());
        entry.op2 = ensure_not_ptr(op);
        entry.op3 = icode::dtype_opr(cast_dtype, id());

        return push_ir_ensure_no_write_ptr(entry);
    }

    void
    ir_builder::cmpop(icode::instruction instr, icode::operand op1, icode::operand op2)
    {
        icode::entry entry;
        entry.opcode = instr;
        entry.op1 = ensure_not_ptr(op1);
        entry.op2 = ensure_not_ptr(op2);

        push_ir(entry);
    }

    icode::operand
    ir_builder::addrop(icode::instruction instr, icode::operand op2, icode::operand op3)
    {
        icode::entry entry;
        entry.op1 = icode::temp_ptr_opr(op2.dtype, id());
        entry.op2 = op2;
        entry.op3 = op3;
        entry.opcode = instr;
        push_ir(entry);

        return entry.op1;
    }

    void ir_builder::label(icode::operand op)
    {
        icode::entry label_entry;
        label_entry.op1 = op;
        label_entry.opcode = icode::CREATE_LABEL;
        push_ir(label_entry);
    }

    void ir_builder::goto_label(icode::instruction instr, icode::operand op)
    {
        icode::entry goto_entry;
        goto_entry.op1 = op;
        goto_entry.opcode = instr;
        push_ir(goto_entry);
    }

    void ir_builder::printop(icode::instruction printop, icode::operand op)
    {
        icode::entry print_entry;

        if (printop == icode::PRINT)
            print_entry.op1 = ensure_not_ptr(op);
        else
            print_entry.op1 = op;

        print_entry.opcode = printop;
        push_ir(print_entry);
    }

    void
    ir_builder::inputop(icode::instruction instr, icode::operand op, unsigned int size)
    {
        icode::entry input_entry;
        input_entry.op1 = op;
        input_entry.op2 = icode::literal_opr(icode::INT, (int)size, id());
        input_entry.opcode = instr;
        push_ir(input_entry);
    }

    void ir_builder::pass(icode::instruction pass_instr,
                          icode::operand op,
                          const std::string& func_name,
                          const icode::func_desc& func_desc)
    {
        icode::data_type func_dtype = func_desc.func_info.dtype;
        icode::entry entry;

        if (pass_instr == icode::PASS)
            entry.op1 = ensure_not_ptr(op);
        else
            entry.op1 = op;

        entry.op2 = icode::var_opr(func_dtype, func_name, id());
        entry.op3 = icode::module_opr(func_desc.module_name, id());
        entry.opcode = pass_instr;
        push_ir(entry);
    }

    icode::operand
    ir_builder::call(const std::string& func_name, const icode::func_desc& func_desc)
    {
        icode::data_type func_dtype = func_desc.func_info.dtype;

        icode::entry call_entry;
        call_entry.op1 = icode::temp_opr(func_dtype, id());
        call_entry.op2 = icode::var_opr(func_dtype, func_name, id());
        call_entry.op3 = icode::module_opr(func_desc.module_name, id());
        call_entry.opcode = icode::CALL;

        push_ir(call_entry);

        return call_entry.op1;
    }

    void ir_builder::opir(icode::instruction instr)
    {
        icode::entry entry;
        entry.opcode = instr;
        push_ir(entry);
    }
}