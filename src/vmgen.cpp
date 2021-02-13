#include "vmgen.hpp"

namespace uhllvmgen
{
    icode::target_desc target_desc()
    {
        /* Target descroption for uhllvm */
        icode::target_desc uhlltarget;

        uhlltarget.dtype_strings_map = { { "int", icode::VM_INT },
                                         { "bool", icode::VM_INT },
                                         { "float", icode::VM_FLOAT },
                                         { "uint", icode::VM_UINT } };

        /* true and false defines */
        icode::def true_def;
        true_def.dtype = icode::INT;
        true_def.val.integer = 1;

        icode::def false_def;
        false_def.dtype = icode::INT;
        false_def.val.integer = 0;

        uhlltarget.defines = { { "true", true_def }, { "false", false_def } };

        /* default int or word */
        uhlltarget.default_int = icode::INT;

        return uhlltarget;
    }

    void uhllvm_generator::to_gblmode()
    {
        if (!gblmode)
        {
            vm.create_instr(vm::GBLMODE);
            gblmode = true;
        }
    }

    void uhllvm_generator::to_fpmode()
    {
        if (gblmode)
        {
            vm.create_instr(vm::FPMODE);
            gblmode = false;
        }
    }

    unsigned int uhllvm_generator::get_frame_addr(const icode::operand& op)
    {
        unsigned int frame_addr;

        /*
            Get the address in which the temp/var is stored,
            switch to gblmode or fpmode if necessary to access variable
        */

        if (op.optype == icode::RET_PTR)
        {
            frame_addr = 1;

            to_fpmode();
        }
        else if (op.optype == icode::TEMP || op.optype == icode::TEMP_PTR)
        {
            auto pair = temps.find(op);

            if (pair == temps.end())
            {
                frame_addr = curr_frame_size;
                temps[op] = frame_addr;

                if (op.optype == icode::TEMP)
                    curr_frame_size += op.val.size;
                else
                    curr_frame_size += 1;
            }
            else
            {
                frame_addr = pair->second;
            }

            to_fpmode();
        }
        else if (op.optype == icode::GBL_VAR || op.optype == icode::STR_DATA)
        {
            frame_addr = (*globals)[op.name];
            to_gblmode();
        }
        else
        {
            frame_addr = (*locals)[op.name];
            to_fpmode();
        }

        return frame_addr;
    }

    void uhllvm_generator::save()
    {
        /* If acc is none, dont save */
        if (acc.optype == icode::NONE || !acc_mod)
            return;

        /* Write accumulator to address */
        unsigned int frame_addr = get_frame_addr(acc);

        if (icode::is_ptr(acc.optype) && !acc_is_addr)
            vm.create_instr_ui(vm::STPTR, frame_addr);
        else if (acc_is_addr && !icode::is_ptr(acc.optype))
            return;
        else
            vm.create_instr_ui(vm::ST, frame_addr);
    }

    void uhllvm_generator::save_acc_val()
    {
        if (acc.optype == icode::STR_DATA)
            return;

        if (icode::is_ptr(acc.optype) || acc_in_ir || acc_is_live)
            save();
    }

    void uhllvm_generator::load(const icode::operand& op, bool mod)
    {
        /* If the accumulator is not going to be modified and
            it has the operand already, return */
        if (acc == op && !acc_is_addr && !mod)
            return;

        /* Save accumulator if req */
        save_acc_val();
        acc_mod = mod;

        /* If accumulator already has the operand return */
        if (acc == op && !acc_is_addr)
            return;

        if (op.optype == icode::LITERAL)
        {
            switch (op.dtype)
            {
                case icode::INT:
                    vm.create_instr_i(vm::LDIMi, op.val.integer);
                    break;
                case icode::FLOAT:
                    vm.create_instr_f(vm::LDIMf, op.val.floating);
                    break;
                default:
                    log::internal_error(mod_name);
                    throw log::internal_bug_error();
            }
        }
        /* Else load accumulator from address */
        else
        {
            unsigned int frame_addr = get_frame_addr(op);

            if (icode::is_ptr(op.optype))
                vm.create_instr_ui(vm::LDPTR, frame_addr);
            else
                vm.create_instr_ui(vm::LD, frame_addr);
        }

        acc = op;
        acc_is_addr = false;
    }

    void uhllvm_generator::load_addr(const icode::operand& op)
    {
        /* If accumulator already has the operand return */
        if (acc == op && acc_is_addr)
            return;

        /* Save accumulator if req */
        if ((icode::is_ptr(acc.optype) && !acc_is_addr) || acc_is_live)
            save();

        if (op.optype == icode::ADDR || op.optype == icode::LITERAL)
        {
            vm.create_instr_ui(vm::LDIMui, op.val.address);
        }
        else
        {
            unsigned int frame_addr = get_frame_addr(op);

            if (icode::is_ptr(op.optype))
                vm.create_instr_ui(vm::LD, frame_addr);
            else if (icode::is_ltrl(op.optype))
                vm.create_instr_ui(vm::LDIMui, frame_addr);
            else
                vm.create_instr_ui(vm::LDEA, frame_addr);
        }

        acc = op;
        acc_is_addr = true;
        acc_mod = true;
    }

    void uhllvm_generator::eq(const icode::entry& e)
    {
        /* Load accumulator */
        load(e.op2);

        /* Accumulator now has that variable */
        acc = e.op1;
    }

    void uhllvm_generator::addrop(const icode::entry& e)
    {
        icode::operand addrop;

        /* Commutative operation, order does not matter
            Load non literal or existing addr into accumulator preferably */
        if (e.opcode == icode::ADDR_MUL)
        {
            load(e.op2);
            addrop = e.op3;
            acc_is_addr = true;
        }
        else
        {
            if (icode::is_ltrl(e.op3.optype) || (acc == e.op2 && acc_is_addr))
            {
                load_addr(e.op2);
                addrop = e.op3;
            }
            else
            {
                load_addr(e.op3);
                addrop = e.op2;
            }
        }

        /* Get addres from addrop */
        unsigned int imm_op;

        if (addrop.optype == icode::ADDR || addrop.optype == icode::LITERAL)
            imm_op = addrop.val.address;
        else
            imm_op = get_frame_addr(addrop);

        /* Generate instruction */
        switch (addrop.optype)
        {
            case icode::VAR:
            case icode::ADDR:
            case icode::GBL_VAR:
            case icode::LITERAL:
            {
                switch (e.opcode)
                {
                    case icode::ADDR_ADD:
                        if (addrop.optype != icode::VAR)
                            vm.create_instr_ui(vm::ADDIMui, imm_op);
                        else
                            vm.create_instr_ui(vm::ADDEA, imm_op);
                        break;
                    case icode::ADDR_MUL:
                        vm.create_instr_ui(vm::MULIMui, imm_op);
                        break;
                    default:
                        log::internal_error(mod_name);
                        throw log::internal_bug_error();
                }

                break;
            }
            case icode::TEMP_PTR:
            case icode::RET_PTR:
            case icode::PTR:
            {
                switch (e.opcode)
                {
                    case icode::ADDR_ADD:
                        vm.create_instr_ui(vm::ADDui, imm_op);
                        break;
                    case icode::ADDR_MUL:
                        vm.create_instr_ui(vm::MULui, imm_op);
                        break;
                    default:
                        log::internal_error(mod_name);
                        throw log::internal_bug_error();
                }

                break;
            }
            default:
            {
                log::internal_error(mod_name);
                throw log::internal_bug_error();
            }
        }

        acc = e.op1;
    }

    void uhllvm_generator::binop(const icode::entry& e)
    {
        /* Determine which operand to add to acc */
        icode::operand addop;

        if (e.opcode == icode::ADD || e.opcode == icode::MUL)
        {
            /* Commutative operation, order does not matter
                Load non literal or existing into accumulator preferably */
            if (icode::is_ltrl(e.op3.optype) || icode::is_ptr(e.op2.optype) ||
                (acc == e.op1 && !acc_is_addr))
            {
                load(e.op2);
                addop = e.op3;
            }
            else
            {
                load(e.op3);
                addop = e.op2;
            }
        }
        else
        {
            /* Non commutative operation */
            load(e.op2);
            addop = e.op3;
        }

        /* Accumulator will hold the result */
        acc = e.op1;

        /* Generate instruction */
        if (!icode::is_ltrl(addop.optype))
        {
            unsigned int frame_addr = get_frame_addr(addop);

            vm::opcode opc;
            switch (e.opcode)
            {
                case icode::ADD:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            opc = vm::ADDi;
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            opc = vm::ADDf;
                            break;
                        case icode::VM_UINT:
                            opc = vm::ADDui;
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::SUB:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            opc = vm::SUBi;
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            opc = vm::SUBf;
                            break;
                        case icode::VM_UINT:
                            opc = vm::SUBui;
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::MUL:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            opc = vm::MULi;
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            opc = vm::MULf;
                            break;
                        case icode::VM_UINT:
                            opc = vm::MULui;
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::DIV:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            opc = vm::DIVi;
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            opc = vm::DIVf;
                            break;
                        case icode::VM_UINT:
                            opc = vm::DIVui;
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::MOD:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            opc = vm::MODi;
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            opc = vm::MODf;
                            break;
                        case icode::VM_UINT:
                            opc = vm::MODui;
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::RSH:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            opc = vm::RSHi;
                            break;
                        case icode::VM_UINT:
                            opc = vm::RSH;
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::LSH:
                    opc = vm::LSH;
                    break;
                case icode::BWA:
                    opc = vm::BWA;
                    break;
                case icode::BWO:
                    opc = vm::BWO;
                    break;
                case icode::BWX:
                    opc = vm::BWX;
                    break;
                default:
                    log::internal_error(mod_name);
                    throw log::internal_bug_error();
            }

            vm.create_instr_ui(opc, frame_addr);
        }
        else
        {
            switch (e.opcode)
            {
                case icode::ADD:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            vm.create_instr_i(vm::ADDIMi, addop.val.integer);
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            vm.create_instr_f(vm::ADDIMf, addop.val.floating);
                            break;
                        case icode::VM_UINT:
                            vm.create_instr_ui(vm::ADDIMui, addop.val.integer);
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::SUB:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            vm.create_instr_i(vm::SUBIMi, addop.val.integer);
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            vm.create_instr_f(vm::SUBIMf, addop.val.floating);
                            break;
                        case icode::VM_UINT:
                            vm.create_instr_ui(vm::SUBIMui, addop.val.integer);
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::MUL:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            vm.create_instr_i(vm::MULIMi, addop.val.integer);
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            vm.create_instr_f(vm::MULIMf, addop.val.floating);
                            break;
                        case icode::VM_UINT:
                            vm.create_instr_ui(vm::MULIMui, addop.val.integer);
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::DIV:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            vm.create_instr_i(vm::DIVIMi, addop.val.integer);
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            vm.create_instr_f(vm::DIVIMf, addop.val.floating);
                            break;
                        case icode::VM_UINT:
                            vm.create_instr_ui(vm::DIVIMui, addop.val.integer);
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::MOD:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            vm.create_instr_i(vm::MODIMi, addop.val.integer);
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            vm.create_instr_f(vm::MODIMf, addop.val.floating);
                            break;
                        case icode::VM_UINT:
                            vm.create_instr_ui(vm::MODIMui, addop.val.integer);
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::RSH:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            vm.create_instr_i(vm::RSHIMi, addop.val.integer);
                            break;
                        case icode::VM_UINT:
                            vm.create_instr_ui(vm::RSHIM, addop.val.integer);
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }
                    break;
                }
                case icode::LSH:
                    vm.create_instr_ui(vm::LSHIM, addop.val.integer);
                    break;
                case icode::BWA:
                    vm.create_instr_ui(vm::BWAIM, addop.val.integer);
                    break;
                case icode::BWO:
                    vm.create_instr_ui(vm::BWOIM, addop.val.integer);
                    break;
                case icode::BWX:
                    vm.create_instr_ui(vm::BWXIM, addop.val.integer);
                    break;
                default:
                    log::internal_error(mod_name);
                    throw log::internal_bug_error();
            }
        }
    }

    void uhllvm_generator::cmpop(const icode::entry& e)
    {
        /* Determine which operand to add to acc */
        icode::operand addop;

        if (e.opcode == icode::EQ || e.opcode == icode::NEQ)
        {
            /* Commutative operation, order does not matter
                Load non literal into accumulator preferably */
            if (icode::is_ltrl(e.op2.optype) || icode::is_ptr(e.op1.optype))
            {
                load(e.op1, false);
                addop = e.op2;
            }
            else
            {
                load(e.op2, false);
                addop = e.op1;
            }
        }
        else
        {
            /* Non commutative operation */
            load(e.op1, false);
            addop = e.op2;
        }

        /* Flag to tell weather the next goto should be inverted */
        compliment_goto = false;

        /* Generate instruction */
        if (!icode::is_ltrl(addop.optype))
        {
            unsigned int frame_addr = get_frame_addr(addop);

            vm::opcode opc;
            switch (e.opcode)
            {
                case icode::NEQ:
                case icode::EQ:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            opc = vm::EQi;
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            opc = vm::EQf;
                            break;
                        case icode::VM_UINT:
                            opc = vm::EQui;
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }

                    if (e.opcode == icode::NEQ)
                        compliment_goto = true;

                    break;
                }
                case icode::LTE:
                case icode::GT:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            opc = vm::GTi;
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            opc = vm::GTf;
                            break;
                        case icode::VM_UINT:
                            opc = vm::GTui;
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }

                    if (e.opcode == icode::LTE)
                        compliment_goto = true;

                    break;
                }
                case icode::GTE:
                case icode::LT:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            opc = vm::LTi;
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            opc = vm::LTf;
                            break;
                        case icode::VM_UINT:
                            opc = vm::LTui;
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }

                    if (e.opcode == icode::GTE)
                        compliment_goto = true;

                    break;
                }
                default:
                {
                    log::internal_error(mod_name);
                    throw log::internal_bug_error();
                }
            }

            vm.create_instr_ui(opc, frame_addr);
        }
        else
        {
            switch (e.opcode)
            {
                case icode::NEQ:
                case icode::EQ:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            vm.create_instr_i(vm::EQIMi, addop.val.integer);
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            vm.create_instr_i(vm::EQIMf, addop.val.floating);
                            break;
                        case icode::VM_UINT:
                            vm.create_instr_i(vm::EQIMui, addop.val.integer);
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }

                    if (e.opcode == icode::NEQ)
                        compliment_goto = true;

                    break;
                }
                case icode::LTE:
                case icode::GT:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            vm.create_instr_i(vm::GTIMi, addop.val.integer);
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            vm.create_instr_i(vm::GTIMf, addop.val.floating);
                            break;
                        case icode::VM_UINT:
                            vm.create_instr_i(vm::GTIMui, addop.val.integer);
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }

                    if (e.opcode == icode::LTE)
                        compliment_goto = true;

                    break;
                }
                case icode::GTE:
                case icode::LT:
                {
                    switch (acc.dtype)
                    {
                        case icode::INT:
                        case icode::VM_INT:
                            vm.create_instr_i(vm::LTIMi, addop.val.integer);
                            break;
                        case icode::FLOAT:
                        case icode::VM_FLOAT:
                            vm.create_instr_i(vm::LTIMf, addop.val.floating);
                            break;
                        case icode::VM_UINT:
                            vm.create_instr_i(vm::LTIMui, addop.val.integer);
                            break;
                        default:
                            log::internal_error(mod_name);
                            throw log::internal_bug_error();
                    }

                    if (e.opcode == icode::GTE)
                        compliment_goto = true;

                    break;
                }
                default:
                {
                    log::internal_error(mod_name);
                    throw log::internal_bug_error();
                }
            }
        }
    }

    void uhllvm_generator::uniop(const icode::entry& e)
    {
        /* Load accumulator */
        load(e.op2);

        /* Generate instruction */
        switch (e.opcode)
        {
            case icode::NOT:
                vm.create_instr(vm::NOT);
                break;
            case icode::UNARY_MINUS:
            {
                switch (acc.dtype)
                {
                    case icode::INT:
                    case icode::VM_INT:
                        vm.create_instr(vm::NEGi);
                        break;
                    case icode::FLOAT:
                    case icode::VM_FLOAT:
                        vm.create_instr(vm::NEGf);
                        break;
                    case icode::VM_UINT:
                        vm.create_instr(vm::NEGui);
                        break;
                    default:
                        log::internal_error(mod_name);
                        throw log::internal_bug_error();
                }

                break;
            }
            case icode::CAST:
            {
                switch (acc.dtype)
                {
                    case icode::INT:
                    case icode::VM_INT:
                    {
                        switch (e.op3.dtype)
                        {
                            case icode::FLOAT:
                            case icode::VM_FLOAT:
                                vm.create_instr(vm::CASTif);
                                break;
                            case icode::VM_UINT:
                                vm.create_instr(vm::CASTiui);
                                break;
                            default:
                                log::internal_error(mod_name);
                                throw log::internal_bug_error();
                        }

                        break;
                    }
                    case icode::FLOAT:
                    case icode::VM_FLOAT:
                    {
                        switch (e.op3.dtype)
                        {
                            case icode::INT:
                            case icode::VM_INT:
                                vm.create_instr(vm::CASTfi);
                                break;
                            case icode::VM_UINT:
                                vm.create_instr(vm::CASTfui);
                                break;
                            default:
                                log::internal_error(mod_name);
                                throw log::internal_bug_error();
                        }

                        break;
                    }
                    case icode::VM_UINT:
                    {
                        switch (e.op3.dtype)
                        {
                            case icode::INT:
                            case icode::VM_INT:
                                vm.create_instr(vm::CASTuii);
                                break;
                            case icode::FLOAT:
                            case icode::VM_FLOAT:
                                vm.create_instr(vm::CASTuif);
                                break;
                            default:
                                log::internal_error(mod_name);
                                throw log::internal_bug_error();
                        }

                        break;
                    }
                    default:
                    {
                        log::internal_error(mod_name);
                        throw log::internal_bug_error();
                    }
                }

                break;
            }
            default:
            {
                log::internal_error(mod_name);
                throw log::internal_bug_error();
            }
        }

        /* Accumulator will have result */
        acc = e.op1;
    }

    void uhllvm_generator::print(const icode::entry& e)
    {
        /* Generate instruction */
        load(e.op1, false);

        if (icode::is_uint(e.op1.dtype))
            vm.create_instr(vm::PRINTui);
        else if (icode::is_int(e.op1.dtype))
            vm.create_instr(vm::PRINTi);
        else if (icode::is_float(e.op1.dtype))
            vm.create_instr(vm::PRINTf);
    }

    void uhllvm_generator::print_str(const icode::entry& e)
    {
        if (e.op1.optype == icode::STR_DATA)
        {
            unsigned int str_addr = (*globals)[e.op1.name];
            vm.create_instr_ui(vm::PRINTdat, str_addr);
        }
        else if (icode::is_ptr(e.op1.optype))
        {
            /* If acc has address, save it to memory */
            save_acc_val();

            unsigned int frame_addr = get_frame_addr(e.op1);
            vm.create_instr_ui(vm::PRINTptr, frame_addr);
        }
        else
        {
            unsigned int frame_addr = get_frame_addr(e.op1);
            vm.create_instr_ui(vm::PRINTstr, frame_addr);
        }
    }

    void uhllvm_generator::input(const icode::entry& e)
    {
        save_acc_val();

        if (e.opcode == icode::INPUT)
        {
            switch (e.op1.dtype)
            {
                case icode::INT:
                case icode::VM_INT:
                    vm.create_instr(vm::INPUTi);
                    break;
                case icode::FLOAT:
                case icode::VM_FLOAT:
                    vm.create_instr(vm::INPUTf);
                    break;
                case icode::VM_UINT:
                    vm.create_instr(vm::INPUTui);
                    break;
                default:
                    log::internal_error(mod_name);
                    throw log::internal_bug_error();
            }

            /* Accumulator has op1 */
            acc = e.op1;
            acc_is_addr = false;
            acc_mod = true;
        }
        else if (e.opcode == icode::INPUT_STR)
        {
            unsigned int frame_addr = get_frame_addr(e.op1);
            unsigned int limit = e.op2.val.integer;

            vm.create_instr_ui(vm::INPLIM, limit);

            if (!icode::is_ptr(e.op1.optype))
                vm.create_instr_ui(vm::INPUTstr, frame_addr);
            else
                vm.create_instr_ui(vm::INPUTptr, frame_addr);
        }
    }

    std::string uhllvm_generator::entry_to_label(const icode::entry& e)
    {
        if (e.opcode != icode::CALL)
            return e.op1.name + "_" + mod_name;
        else
            return e.op2.name + "_" + e.op3.name;
    }

    void uhllvm_generator::create_label(const std::string& label)
    {
        /* Save accumulator, end/start of a basic block */
        save_acc_val();

        /* You don't know what the accumulator will have b
            becuse this is a jump target */
        acc.optype = icode::NONE;

        /* Create pair in label addr map */
        unsigned int label_addr = vm.get_curr_addr();
        label_addr_map[label] = label_addr;

        /* Check the back patch queue */
        for (unsigned int i : label_backpatch_q[label])
            vm.update_addr(i, label_addr);
    }

    unsigned int uhllvm_generator::get_label_addr(const std::string& label)
    {
        /* Check if it exist in the label address map */
        auto pair = label_addr_map.find(label);

        if (pair != label_addr_map.end())
            return pair->second;
        else
        {
            /* If the label has not been created yet, return 0 for now,
                add to backpatching queue. */
            label_backpatch_q[label].push_back(vm.get_curr_addr());
            return 0;
        }
    }

    void uhllvm_generator::jmp(const icode::entry& e)
    {
        /* Save accumulator before jumping */
        save_acc_val();

        /* Get the address of the label */
        std::string label = entry_to_label(e);
        unsigned int label_addr = get_label_addr(label);

        switch (e.opcode)
        {
            case icode::GOTO:
                vm.create_instr_ui(vm::JMP, label_addr);
                break;
            case icode::IF_TRUE_GOTO:
                if (!compliment_goto)
                    vm.create_instr_ui(vm::JMPF, label_addr);
                else
                    vm.create_instr_ui(vm::JMPNF, label_addr);
                break;
            case icode::IF_FALSE_GOTO:
                if (!compliment_goto)
                    vm.create_instr_ui(vm::JMPNF, label_addr);
                else
                    vm.create_instr_ui(vm::JMPF, label_addr);
                break;
            default:
                log::internal_error(mod_name);
                throw log::internal_bug_error();
        }
    }

    void uhllvm_generator::call(const icode::entry& e)
    {
        /* Save accumulator before jumping */
        save_acc_val();

        /* Setup return pointer */
        if (e.op1.dtype != icode::VOID)
        {
            vm.create_instr_ui(vm::LDEA, get_frame_addr(e.op1));
            fsize_backpatch_q.push_back(vm.get_curr_addr());
            vm.create_instr_ui(vm::ST, 1);
        }

        /* Setup return address */
        vm.create_instr_ui(vm::LDIMui, vm.get_curr_addr() + 4);
        fsize_backpatch_q.push_back(vm.get_curr_addr());
        vm.create_instr_ui(vm::ST, 0);

        /* Update frame pointer,
            add to fsetup back patch queue, frame size is
            unknown at this point, will be known after this function is
            completly generated  */
        fsize_backpatch_q.push_back(vm.get_curr_addr());
        vm.create_instr_ui(vm::ADDFP, 0);

        /* Get the address of the function and jump to */
        std::string callee_label = entry_to_label(e);
        unsigned int calle_addr = get_label_addr(callee_label);
        vm.create_instr_ui(vm::CALL, calle_addr);

        /* Restore frame pointer */
        fsize_backpatch_q.push_back(vm.get_curr_addr());
        vm.create_instr_ui(vm::SUBFP, 0);

        /* Invalidate accumulator, you don't know what callee did to it */
        acc.optype = icode::NONE;
    }

    void uhllvm_generator::pass(const icode::entry& e, int param_count)
    {
        /* Load accumulator */
        if (e.opcode == icode::PASS)
            load(e.op1, false);
        else
            load_addr(e.op1);

        to_fpmode();

        /* Get frame address, add to fsetup backpatch q
            so funcation frame size can be added later */
        fsize_backpatch_q.push_back(vm.get_curr_addr());

        std::string param_name =
          modules[e.op3.name].functions[e.op2.name].params[param_count];

        unsigned int callee_param_addr =
          opt.prog_local_addr_map[e.op3.name][e.op2.name][param_name];

        /* Copy parameters */
        vm.create_instr_ui(vm::ST, callee_param_addr);
    }

    void uhllvm_generator::update_acc_live_info(int entry_idx)
    {
        auto use_info_pair = opt.prog_use_map[mod_name][func_name][entry_idx].find(acc);
        auto use_info_end = opt.prog_use_map[mod_name][func_name][entry_idx].end();

        if (use_info_pair != use_info_end)
            acc_is_live = use_info_pair->second.live;
    }

    void uhllvm_generator::gen_func(const std::vector<icode::entry>& icode)
    {
        temps.clear();
        fsize_backpatch_q.clear();
        curr_frame_size = opt.local_vars_size[mod_name][func_name];

        locals = &opt.prog_local_addr_map[mod_name][func_name];

        /* Accumulator does not hold anything at start */
        acc_is_live = false;
        acc_mod = false;
        acc_is_addr = false;
        acc.optype = icode::NONE;

        /* Create function label */
        std::string label = func_name + "_" + mod_name;
        create_label(label);

        /* Loop through each icode entry and generate VM instruction */
        int param_count = -1;
        for (size_t i = 0; i < icode.size(); i++)
        {
            /* Get new entry and use info */
            icode::entry e = icode[i];
            update_acc_live_info(i);

            /* Check if value held by accumulator is use by current
             * instruction/entry
             */
            if (acc == e.op1 || acc == e.op2 || acc == e.op3)
                acc_in_ir = true;
            else
                acc_in_ir = false;

            /* Keep track which parameter is being passed */
            if (e.opcode == icode::PASS || e.opcode == icode::PASS_ADDR)
                param_count++;
            else if (e.opcode == icode::CALL)
                param_count = -1;

            /* Generate instruction */
            switch (e.opcode)
            {
                case icode::ADD:
                case icode::SUB:
                case icode::MUL:
                case icode::DIV:
                case icode::MOD:
                case icode::LSH:
                case icode::RSH:
                case icode::BWA:
                case icode::BWO:
                case icode::BWX:
                    binop(e);
                    break;
                case icode::EQ:
                case icode::NEQ:
                case icode::LT:
                case icode::LTE:
                case icode::GT:
                case icode::GTE:
                    cmpop(e);
                    break;
                case icode::ADDR_ADD:
                case icode::ADDR_MUL:
                    addrop(e);
                    break;
                case icode::NOT:
                case icode::UNARY_MINUS:
                case icode::CAST:
                    uniop(e);
                    break;
                case icode::EQUAL:
                case icode::READ:
                case icode::WRITE:
                    eq(e);
                    break;
                case icode::PRINT:
                    print(e);
                    break;
                case icode::PRINT_STR:
                    print_str(e);
                    break;
                case icode::INPUT:
                case icode::INPUT_STR:
                    input(e);
                    break;
                case icode::NEWLN:
                    vm.create_instr(vm::NEWLN);
                    break;
                case icode::SPACE:
                    vm.create_instr(vm::SPACE);
                    break;
                case icode::CREATE_LABEL:
                {
                    std::string label = entry_to_label(e);
                    create_label(label);
                    break;
                }
                case icode::GOTO:
                case icode::IF_TRUE_GOTO:
                case icode::IF_FALSE_GOTO:
                    jmp(e);
                    break;
                case icode::CALL:
                    call(e);
                    break;
                case icode::PASS:
                case icode::PASS_ADDR:
                    pass(e, param_count);
                    break;
                case icode::RET:
                    save_acc_val();
                    to_fpmode();
                    vm.create_instr(vm::RET);
                    break;
                case icode::EXIT:
                    vm.create_instr(vm::EXIT);
                    break;
                default:
                    log::internal_error(mod_name);
                    throw log::internal_bug_error();
            }

            /* Get use info */
            update_acc_live_info(i);
        }

        /* Process fsetup backpatch queue with correct frame size */
        for (unsigned int i : fsize_backpatch_q)
            vm.update_addr(i, curr_frame_size);
    }

    uhllvm_generator::uhllvm_generator(vm::uhllvm& target_vm,
                                       icode::module_desc_map& modules_map,
                                       opt::optimizer& optimizer)
      : vm(target_vm)
      , opt(optimizer)
      , modules(modules_map)
    {
        /* Load frame pointer to after global data */
        vm.create_instr_ui(vm::LDFP, opt.global_dat_size);

        /* Jump to main, the address will be modified when main
            funtion is found */
        vm.create_instr_ui(vm::CALL, 0);

        /* Flag to avoid having multiple main functions */
        bool found_main = false;

        /* Loop through each function and compile */
        for (auto mod : modules)
        {
            mod_name = mod.first;

            /* Setup module global data */
            globals = &opt.prog_global_addr_map[mod_name];

            /* Write string data */
            for (auto str_dat : mod.second.str_data)
            {
                unsigned int str_addr = (*globals)[str_dat.first];
                vm.org(str_addr);
                vm.create_str_dat(str_dat.second);
            }

            /* Compile module functions */
            for (auto func : mod.second.functions)
            {
                func_name = func.first;

                /* VM boots in fp mode initially, and switched to fp mode on
                    function call */
                gblmode = false;

                /* If main function, modify first jump instruction to
                    jump to this address */
                if (func.first == "main")
                {
                    if (found_main)
                    {
                        log::error(mod.first, "Duplicate main function found");
                        throw log::compile_error();
                    }

                    found_main = true;
                    vm.instr_mem[1].opr.ui = vm.get_curr_addr();
                }

                gen_func(func.second.icode_table);
            }
        }

        if (!found_main)
        {
            log::println("ERROR\nNo entry point or main function found");
            throw log::compile_error();
        }
    }
} // namespace uhllvmgen
