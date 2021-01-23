#include "vm.hpp"

namespace vm
{
    uhllvm::uhllvm()
    {
        fp = 0;
        pc = 0;
        inplim = 0;

        flag = false;
        gblmode = false;
        power = true;
        pcinc = true;
    }

    word& vm::vm_data_mem::operator[](std::size_t addr)
    {
        /* Allocate more memory if required */
        if (addr >= mem.size())
            mem.resize(addr + 1);

        return mem[addr];
    }

    void uhllvm::step()
    {
        instruction instr = instr_mem[pc];
        pcinc = true;
        unsigned int addr;
        unsigned int ptraddr;

        /* Calculate addresses for global or frame pointer relative mode */
        if (gblmode)
            addr = instr.opr.ui;
        else
            addr = instr.opr.ui + fp;

        /* Decode and excute instruction */
        switch (instr.opc)
        {
            case GBLMODE:
                gblmode = true;
                break;
            case FPMODE:
                gblmode = false;
                break;
            case LDFP:
                fp = instr.opr.ui;
                break;
            case ADDFP:
                fp += instr.opr.ui;
                break;
            case SUBFP:
                fp -= instr.opr.ui;
                break;
            case LDEA:
                acc.ui = addr;
                break;
            case ADDEA:
                acc.ui += (instr.opr.ui + fp);
                break;
            case LD:
                acc = data_mem[addr];
                break;
            case LDPTR:
                ptraddr = data_mem[addr].ui;
                acc = data_mem[ptraddr];
                break;
            case LDIMf:
            case LDIMi:
            case LDIMui:
                acc = instr.opr;
                break;
            case ST:
                data_mem[addr] = acc;
                break;
            case STPTR:
                ptraddr = data_mem[addr].ui;
                data_mem[ptraddr] = acc;
                break;
            case CASTfi:
                acc.i = (int)acc.f;
                break;
            case CASTfui:
                acc.ui = (unsigned int)acc.f;
                break;
            case CASTif:
                acc.f = (float)acc.i;
                break;
            case CASTiui:
                acc.ui = (unsigned int)acc.i;
                break;
            case CASTuii:
                acc.i = (int)acc.ui;
                break;
            case CASTuif:
                acc.f = (float)acc.ui;
                break;
            case NOT:
                acc.ui = ~acc.ui;
                break;
            case BWA:
                acc.ui &= data_mem[addr].ui;
                break;
            case BWX:
                acc.ui ^= data_mem[addr].ui;
                break;
            case BWO:
                acc.ui |= data_mem[addr].ui;
                break;
            case RSH:
                acc.ui >>= data_mem[addr].ui;
                break;
            case RSHi:
                acc.i >>= data_mem[addr].i;
                break;
            case LSH:
                acc.ui <<= data_mem[addr].ui;
                break;
            case BWAIM:
                acc.ui &= instr.opr.ui;
                break;
            case BWXIM:
                acc.ui ^= instr.opr.ui;
                break;
            case BWOIM:
                acc.ui |= instr.opr.ui;
                break;
            case RSHIM:
                acc.ui >>= instr.opr.ui;
                break;
            case RSHIMi:
                acc.i >>= instr.opr.ui;
                break;
            case LSHIM:
                acc.ui <<= instr.opr.ui;
                break;
            case NEGf:
                acc.f = -acc.f;
                break;
            case NEGi:
                acc.i = -acc.i;
                break;
            case NEGui:
                acc.ui = -acc.ui;
                break;
            case MULi:
                acc.i *= data_mem[addr].i;
                break;
            case DIVi:
                acc.i /= data_mem[addr].i;
                break;
            case MODi:
                acc.i %= data_mem[addr].i;
                break;
            case ADDi:
                acc.i += data_mem[addr].i;
                break;
            case SUBi:
                acc.i -= data_mem[addr].i;
                break;
            case LTi:
                flag = acc.i < data_mem[addr].i;
                break;
            case GTi:
                flag = acc.i > data_mem[addr].i;
                break;
            case EQi:
                flag = acc.i == data_mem[addr].i;
                break;
            case MULf:
                acc.f *= data_mem[addr].f;
                break;
            case DIVf:
                acc.f /= data_mem[addr].f;
                break;
            case MODf:
                acc.f = (int)acc.f % (int)data_mem[addr].f;
                break;
            case ADDf:
                acc.f += data_mem[addr].f;
                break;
            case SUBf:
                acc.f -= data_mem[addr].f;
                break;
            case LTf:
                flag = acc.f < data_mem[addr].f;
                break;
            case GTf:
                flag = acc.f > data_mem[addr].f;
                break;
            case EQf:
                flag = acc.f == data_mem[addr].f;
                break;
            case MULui:
                acc.ui *= data_mem[addr].ui;
                break;
            case DIVui:
                acc.ui /= data_mem[addr].ui;
                break;
            case MODui:
                acc.ui %= data_mem[addr].ui;
                break;
            case ADDui:
                acc.ui += data_mem[addr].ui;
                break;
            case SUBui:
                acc.ui -= data_mem[addr].ui;
                break;
            case LTui:
                flag = acc.ui < data_mem[addr].ui;
                break;
            case GTui:
                flag = acc.ui > data_mem[addr].ui;
                break;
            case EQui:
                flag = acc.ui == data_mem[addr].ui;
                break;
            case MULIMi:
                acc.i *= instr.opr.i;
                break;
            case DIVIMi:
                acc.i /= instr.opr.i;
                break;
            case MODIMi:
                acc.i %= instr.opr.i;
                break;
            case ADDIMi:
                acc.i += instr.opr.i;
                break;
            case SUBIMi:
                acc.i -= instr.opr.i;
                break;
            case LTIMi:
                flag = acc.i < instr.opr.i;
                break;
            case GTIMi:
                flag = acc.i > instr.opr.i;
                break;
            case EQIMi:
                flag = acc.i == instr.opr.i;
                break;
            case MULIMf:
                acc.f *= instr.opr.f;
                break;
            case DIVIMf:
                acc.f /= instr.opr.f;
                break;
            case MODIMf:
                acc.f = (int)acc.f % (int)instr.opr.f;
                break;
            case ADDIMf:
                acc.f += instr.opr.f;
                break;
            case SUBIMf:
                acc.f -= instr.opr.f;
                break;
            case LTIMf:
                flag = acc.f < instr.opr.f;
                break;
            case GTIMf:
                flag = acc.f > instr.opr.f;
                break;
            case EQIMf:
                flag = acc.f == instr.opr.f;
                break;
            case MULIMui:
                acc.ui *= instr.opr.ui;
                break;
            case DIVIMui:
                acc.ui /= instr.opr.ui;
                break;
            case MODIMui:
                acc.ui %= instr.opr.ui;
                break;
            case ADDIMui:
                acc.ui += instr.opr.ui;
                break;
            case SUBIMui:
                acc.ui -= instr.opr.ui;
                break;
            case LTIMui:
                flag = acc.ui < instr.opr.ui;
                break;
            case GTIMui:
                flag = acc.ui > instr.opr.ui;
                break;
            case EQIMui:
                flag = acc.ui == instr.opr.ui;
                break;
            case CALL:
                gblmode = false;
            case JMP:
                pc = instr.opr.ui;
                pcinc = false;
                break;
            case RET:
                pc = data_mem[addr].ui;
                pcinc = false;
                break;
            case JMPF:
                if (flag)
                {
                    pc = instr.opr.ui;
                    pcinc = false;
                }
                break;
            case JMPNF:
                if (!flag)
                {
                    pc = instr.opr.ui;
                    pcinc = false;
                }
                break;

            /* Print instructions */
            case PRINTf:
                std::cout << acc.f;
                break;
            case PRINTi:
                std::cout << acc.i;
                break;
            case PRINTui:
                std::cout << acc.ui;
                break;
            case PRINTdat:
            case PRINTptr:
            case PRINTstr:
            {
                unsigned int i;

                switch (instr.opc)
                {
                    case PRINTdat:
                        i = instr.opr.ui;
                        break;
                    case PRINTptr:
                        i = data_mem[addr].ui;
                        break;
                    default:
                        i = addr;
                        break;
                }

                while (true)
                {
                    if (data_mem[i].i == 0)
                        break;
                    std::cout << (char)data_mem[i].i;

                    i++;
                }

                break;
            }
            case NEWLN:
                std::cout << std::endl;
                break;
            case SPACE:
                std::cout << " ";
                break;

            /* Input instructions */
            case INPUTf:
                std::cin >> acc.f;
                break;
            case INPUTi:
                std::cin >> acc.i;
                break;
            case INPUTui:
                std::cin >> acc.ui;
                break;
            case INPLIM:
                inplim = instr.opr.ui;
                break;
            case INPUTptr:
            case INPUTstr:
            {
                std::string input;
                std::getline(std::cin, input);

                unsigned int str_addr;

                if (instr.opc == INPUTstr)
                    str_addr = addr;
                else
                    str_addr = data_mem[addr].ui;

                unsigned int i;
                for (i = 0; i < inplim - 1 && i < input.length(); i++)
                    data_mem[i + str_addr].i = (int)input[i];

                /* Null terminate the string */
                data_mem[i + str_addr].ui = 0;

                break;
            }
            case EXIT:
                power = false;
                break;
            default:
                std::cout << "VM ERROR: Unknown instruction" << std::endl;
                throw vm_error();
                break;
        }

        if (pcinc)
            pc++;
    }

    void uhllvm::run()
    {
        while (power)
            step();
    }

    /* Helper functions for "programming" the VM */

    void uhllvm::create_instr(opcode opc)
    {
        vm::instruction new_i;
        new_i.opc = opc;
        new_i.opr.ui = 0;

        instr_mem.push_back(new_i);
    }

    void uhllvm::create_instr_f(opcode opc, float op)
    {
        vm::instruction new_i;
        new_i.opc = opc;
        new_i.opr.f = op;

        instr_mem.push_back(new_i);
    }

    void uhllvm::create_instr_i(opcode opc, int op)
    {
        vm::instruction new_i;
        new_i.opc = opc;
        new_i.opr.i = op;

        instr_mem.push_back(new_i);
    }

    void uhllvm::create_instr_ui(opcode opc, unsigned int op)
    {
        vm::instruction new_i;
        new_i.opc = opc;
        new_i.opr.ui = op;

        instr_mem.push_back(new_i);
    }

    void uhllvm::create_str_dat(const std::vector<int>& string)
    {
        for (char c : string)
        {
            vm::word char_word;
            char_word.i = c;

            data_mem.mem.push_back(char_word);
        }
    }

    void uhllvm::org(unsigned int addr) { data_mem.mem.resize(addr); }

    unsigned int uhllvm::get_curr_addr() { return instr_mem.size(); }

    void uhllvm::update_addr(unsigned int addr, unsigned int op)
    {
        instr_mem[addr].opr.ui += op;
    }

} // namespace vm
