#ifndef VM_HPP
#define VM_HPP

#include <iostream>
#include <vector>

namespace vm
{
    struct vm_error
    {
    };

    enum opcode
    {
        /* uhllvm Instructions */
        GBLMODE,
        FPMODE,
        LDFP,
        ADDFP,
        SUBFP,
        LDEA,
        ADDEA,
        LD,
        LDPTR,
        LDIMf,
        LDIMi,
        LDIMui,
        ST,
        STPTR,
        CASTfi,
        CASTfui,
        CASTif,
        CASTiui,
        CASTuii,
        CASTuif,
        NOT,
        BWA,
        BWX,
        BWO,
        RSH,
        RSHi,
        LSH,
        BWAIM,
        BWXIM,
        BWOIM,
        RSHIM,
        RSHIMi,
        LSHIM,
        NEGf,
        NEGi,
        NEGui,
        MULi,
        DIVi,
        MODi,
        ADDi,
        SUBi,
        LTi,
        GTi,
        EQi,
        MULf,
        DIVf,
        MODf,
        ADDf,
        SUBf,
        LTf,
        GTf,
        EQf,
        MULui,
        DIVui,
        MODui,
        ADDui,
        SUBui,
        LTui,
        GTui,
        EQui,
        MULIMi,
        DIVIMi,
        MODIMi,
        ADDIMi,
        SUBIMi,
        LTIMi,
        GTIMi,
        EQIMi,
        MULIMf,
        DIVIMf,
        MODIMf,
        ADDIMf,
        SUBIMf,
        LTIMf,
        GTIMf,
        EQIMf,
        MULIMui,
        DIVIMui,
        MODIMui,
        ADDIMui,
        SUBIMui,
        LTIMui,
        GTIMui,
        EQIMui,
        JMP,
        CALL,
        JMPF,
        JMPNF,
        RET,

        /* Opcode for printing data */
        PRINTf,
        PRINTi,
        PRINTui,
        PRINTstr,
        PRINTdat,
        PRINTptr,
        NEWLN,
        SPACE,

        /* Opcode for getting input */
        INPUTf,
        INPUTi,
        INPUTui,
        INPUTstr,
        INPUTptr,
        INPLIM,

        /* Exit VM */
        EXIT
    };

    union word
    {
        float f;
        int i;
        unsigned int ui;
    };

    struct instruction
    {
        opcode opc;
        word opr;
    };

    struct vm_data_mem
    {
        std::vector<word> mem;

        word& operator[](std::size_t);
    };

    class uhllvm
    {
        /* VM registers */
        word acc;
        unsigned int fp;
        unsigned int pc;
        unsigned int inplim;

        /* VM flags */
        bool flag;
        bool pcinc;
        bool gblmode;
        bool power;

      public:
        /* VM memory */
        vm_data_mem data_mem;
        std::vector<instruction> instr_mem;

        uhllvm();
        void step();
        void run();

        /* Helper functions for "programming" the VM */
        void create_instr(opcode opc);
        void create_instr_f(opcode opc, float op);
        void create_instr_i(opcode opc, int op);
        void create_instr_ui(opcode opc, unsigned int op);
        void create_str_dat(const std::vector<int>& string);
        void org(unsigned int addr);

        /* Helper functions for handling labels and backpatching */
        unsigned int get_curr_addr();
        void update_addr(unsigned int addr, unsigned int op);
    };
}

#endif
