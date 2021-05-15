#include "ibuild.hpp"

#include "log.hpp"

namespace ibuild
{
    ir_builder::ir_builder(icode::ModuleDescription& module_desc)
      : module(module_desc)
    {
        id_counter = 0;
    }

    unsigned int ir_builder::id()
    {
        return id_counter++;
    }

    void ir_builder::set_func_desc(icode::FunctionDescription* func_desc)
    {
        current_func_desc = func_desc;
    }

    void ir_builder::push_ir(icode::Entry entry)
    {
        /* Push an ir entry to the current function's icode table */

        (*current_func_desc).icodeTable.push_back(entry);
    }

    icode::Operand ir_builder::create_ptr(const icode::Operand& op)
    {
        /* Converts op to TEMP_PTR using the CREATE_PTR instruction */

        icode::DataType ptr_dtype;
        std::string ptr_dtype_name;

        /* If the op is a struct, the ptr dtype will be the dtype of the first field of the struct
            else it would be the same as op */
        if (op.dtype == icode::STRUCT)
        {
            icode::VariableDescription first_field_info = module.structures[op.dtypeName].structFields.begin()->second;
            ptr_dtype = first_field_info.dtype;
            ptr_dtype_name = first_field_info.dtypeName;
        }
        else
        {
            ptr_dtype = op.dtype;
            ptr_dtype_name = op.dtypeName;
        }

        /* Converted TEMP_PTR */
        icode::Operand ptr_op = icode::createPointerOperand(ptr_dtype, ptr_dtype_name, id());

        /* Construct CREATE_PTR incstruction */
        icode::Entry create_ptr_entry;
        create_ptr_entry.op1 = ptr_op;
        create_ptr_entry.op2 = op;
        create_ptr_entry.opcode = icode::CREATE_PTR;
        push_ir(create_ptr_entry);

        return ptr_op;
    }

    void ir_builder::copy(icode::Operand op1, icode::Operand op2)
    {
        /* If op2 is a literal, change generic dtypes like icode::INT and icode::FLOAT
            to correct specific dtype */
        if (op2.operandType == icode::LITERAL)
            op2.dtype = op1.dtype;

        /* Copy one operand value to another, use READ and WRITE instruction
            if pointers are involved */

        if (op1.isPointer() && op2.isPointer())
        {
            icode::Operand temp = icode::createTempOperand(op2.dtype, op2.dtypeName, id());
            copy(temp, op2);
            copy(op1, temp);
        }
        else
        {
            icode::Entry copy_entry;
            copy_entry.op1 = op1;
            copy_entry.op2 = op2;

            if (op1.isPointer() && !op2.isPointer())
                copy_entry.opcode = icode::WRITE;
            else if (!op1.isPointer() && op2.isPointer())
                copy_entry.opcode = icode::READ;
            else if (!op1.isPointer() && !op2.isPointer())
                copy_entry.opcode = icode::EQUAL;

            push_ir(copy_entry);
        }
    }

    icode::Operand ir_builder::ensure_not_ptr(icode::Operand op)
    {
        /* Make sure the operand is not a pointer, if it is a pointer,
            converts it to a temp using the READ instruction */

        if (op.isPointer())
        {
            icode::Entry read_entry;
            icode::Operand temp = icode::createTempOperand(op.dtype, op.dtypeName, id());
            copy(temp, op);
            return temp;
        }

        return op;
    }

    icode::Operand ir_builder::push_ir_ensure_no_write_ptr(icode::Entry entry)
    {
        /* Push an ir entry to the current function's icode table,
            but ensures entry.op1 is not a pointer */

        if (!entry.op1.isPointer())
        {
            push_ir(entry);
            return entry.op1;
        }

        /* If entry.op1 is a pointer, repleace it with a temp and
            write that temp to the pointer */

        /* Create corresponding TEMP to TEMP_PTR  */
        icode::Operand ptr_op = entry.op1;
        icode::Operand temp = icode::createTempOperand(ptr_op.dtype, ptr_op.dtypeName, id());

        /* Replace TEMP_PTR with TEMP */
        icode::Entry mod_entry = entry;
        mod_entry.op1 = temp;
        push_ir(mod_entry);

        /* Create WRITE instruction to write the TEMP to TEMP_PTR */
        icode::Entry write_entry;
        write_entry.op1 = ptr_op;
        write_entry.op2 = temp;
        write_entry.opcode = icode::WRITE;
        push_ir(write_entry);

        return temp;
    }

    icode::Operand ir_builder::binop(icode::Instruction instr,
                                     icode::Operand op1,
                                     icode::Operand op2,
                                     icode::Operand op3)
    {
        /* Construct icode instruction for binary operator instructions,
            ADD, SUB, MUL, DIV, MOD, RSH, LSH, BWA, BWO, BWX */

        icode::Entry entry;
        entry.opcode = instr;
        entry.op1 = op1;
        entry.op2 = ensure_not_ptr(op2);
        entry.op3 = ensure_not_ptr(op3);

        return push_ir_ensure_no_write_ptr(entry);
    }

    icode::Operand ir_builder::uniop(icode::Instruction instr, icode::Operand op1, icode::Operand op2)
    {
        /* Construct icode for unary operator instructions,
            UNARY_MINUS and NOT  */

        icode::Entry entry;
        entry.opcode = instr;
        entry.op1 = op1;
        entry.op2 = ensure_not_ptr(op2);

        return push_ir_ensure_no_write_ptr(entry);
    }

    icode::Operand ir_builder::cast(icode::DataType cast_dtype, icode::Operand op)
    {
        /* Construct icode for CAST */

        icode::Entry entry;
        entry.opcode = icode::CAST;
        entry.op1 = icode::createTempOperand(cast_dtype, icode::dataTypeToString(cast_dtype), id());
        entry.op2 = ensure_not_ptr(op);

        return push_ir_ensure_no_write_ptr(entry);
    }

    void ir_builder::cmpop(icode::Instruction instr, icode::Operand op1, icode::Operand op2)
    {
        /* If op2 is a literal, change generic dtypes like icode::INT and icode::FLOAT
            to correct specific dtype */
        if (op2.operandType == icode::LITERAL)
            op2.dtype = op1.dtype;

        /* Construct icode for comparator operator instructions,
            EQ, NEQ, LT, LTE, GT, GTE  */

        icode::Entry entry;
        entry.opcode = instr;
        entry.op1 = ensure_not_ptr(op1);
        entry.op2 = ensure_not_ptr(op2);

        push_ir(entry);
    }

    icode::Operand ir_builder::addr_add(icode::Operand op2, icode::Operand op3)
    {
        /* Construct icode for ADDR_ADD */

        icode::Entry entry;
        entry.op1 = icode::createPointerOperand(op2.dtype, op2.dtypeName, id());
        entry.op2 = op2;
        entry.op3 = op3;
        entry.opcode = icode::ADDR_ADD;
        push_ir(entry);

        return entry.op1;
    }

    icode::Operand ir_builder::addr_mul(icode::Operand op2, icode::Operand op3)
    {
        /* Construct icode for ADDR_MUL */

        icode::Entry entry;
        entry.op1 = icode::createPointerOperand(icode::INT, icode::dataTypeToString(icode::I8), id());
        entry.op2 = op2;
        entry.op3 = op3;
        entry.opcode = icode::ADDR_MUL;
        push_ir(entry);

        return entry.op1;
    }

    void ir_builder::label(icode::Operand op)
    {
        /* Construct CREATE_LABEL */

        icode::Entry label_entry;
        label_entry.op1 = op;
        label_entry.opcode = icode::CREATE_LABEL;
        push_ir(label_entry);
    }

    void ir_builder::goto_label(icode::Instruction instr, icode::Operand op)
    {
        /* Construct icode for GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO */

        icode::Entry goto_entry;
        goto_entry.op1 = op;
        goto_entry.opcode = instr;
        push_ir(goto_entry);
    }

    void ir_builder::printop(icode::Instruction printop, icode::Operand op)
    {
        /* Construct icode for PRINT, PRINT_STR */

        icode::Entry print_entry;

        if (printop == icode::PRINT)
            print_entry.op1 = ensure_not_ptr(op);
        else
            print_entry.op1 = op;

        print_entry.opcode = printop;
        push_ir(print_entry);
    }

    void ir_builder::inputop(icode::Instruction instr, icode::Operand op, unsigned int size)
    {
        /* Construct icode for INPUT, INPUT_STR */

        icode::Entry input_entry;
        input_entry.op1 = op;
        input_entry.op2 = icode::createLiteralOperand(icode::INT, (int)size, id());
        input_entry.opcode = instr;
        push_ir(input_entry);
    }

    void ir_builder::pass(icode::Instruction pass_instr,
                          icode::Operand op,
                          const std::string& func_name,
                          const icode::FunctionDescription& func_desc)
    {
        /* Construct icode for PASS and PASS_ADDR instructions */

        icode::DataType func_dtype = func_desc.functionReturnDescription.dtype;
        std::string func_dtype_name = func_desc.functionReturnDescription.dtypeName;
        icode::Entry entry;

        if (pass_instr == icode::PASS)
            entry.op1 = ensure_not_ptr(op);
        else
            entry.op1 = op;

        entry.op2 = icode::createVarOperand(func_dtype, func_dtype_name, func_name, id());
        entry.op3 = icode::createModuleOperand(func_desc.moduleName, id());
        entry.opcode = pass_instr;
        push_ir(entry);
    }

    icode::Operand ir_builder::call(const std::string& func_name, const icode::FunctionDescription& func_desc)
    {
        /* Construct icode for CALL instruction */

        icode::DataType func_dtype = func_desc.functionReturnDescription.dtype;
        std::string func_dtype_name = func_desc.functionReturnDescription.dtypeName;

        icode::Entry call_entry;
        call_entry.op1 = icode::createCalleeRetValOperand(func_dtype, func_dtype_name, id());
        call_entry.op2 = icode::createVarOperand(func_dtype, func_dtype_name, func_name, id());
        call_entry.op3 = icode::createModuleOperand(func_desc.moduleName, id());
        call_entry.opcode = icode::CALL;

        push_ir(call_entry);

        return call_entry.op1;
    }

    void ir_builder::opir(icode::Instruction instr)
    {
        /* Construct icode for instructions with no arguments,
            RET, SPACE, NEWLN, EXIT */

        icode::Entry entry;
        entry.opcode = instr;
        push_ir(entry);
    }
}