#ifndef ICODE_HPP
#define ICODE_HPP

#include <algorithm>
#include <map>
#include <string>
#include <vector>

/*
    This header contains structs and enum that are
    used to represent intermediate code.
*/

namespace icode
{
    /*
        Pre build data types and their sizes in number of words.
        Depending on the target, one word may or may not be a byte.
    */

    enum data_type
    {
        I8,
        UI8,
        I16,
        UI16,
        I32,
        UI32,
        I64,
        UI64,
        F32,
        F64,
        VM_INT,
        VM_UINT,
        VM_FLOAT,
        INT,
        FLOAT,
        STRUCT,
        VOID
    };

    const int dtype_size[] = { 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 1, 1, 1, 0, 0, 0, 0 };

    extern std::string data_type_strs[];

    /*
        The following structs are used to describe a
        variables, structs, function parameters, defs
    */

    enum var_prop
    {
        IS_MUT,
        IS_PTR,
        IS_PARAM,
    };

    struct var_info
    {
        icode::data_type dtype;
        std::string dtype_name;
        std::string module_name;
        unsigned int dtype_size;
        unsigned int offset;
        unsigned int size;
        std::vector<unsigned int> dimensions;
        unsigned int scope_id;

        unsigned int properties;

        var_info();

        void set_prop(var_prop prop);
        void clear_prop(var_prop prop);
        bool check(var_prop prop) const;
    };

    struct struct_desc
    {
        std::map<std::string, var_info> fields;
        unsigned int size;
        std::string module_name;

        bool field_exists(const std::string&);
    };

    struct def
    {
        union
        {
            int integer;
            float floating;
        } val;

        data_type dtype;
    };

    /*
        Icode operand types and struct
    */

    enum operand_type
    {
        TEMP,
        TEMP_PTR,
        ADDR,
        STR_DATA,
        VAR,
        GBL_VAR,
        PTR,
        RET_PTR,
        RET_VAL,
        LITERAL,
        LABEL,
        MODULE,
        NONE,
    };

    struct operand
    {
        unsigned int temp_id;

        union
        {
            int integer;
            float floating;
            unsigned int address;
            unsigned int size;
        } val;

        std::string name;
        data_type dtype;
        std::string dtype_name;
        operand_type optype;

        bool operator<(const operand& other) const;
        bool operator==(const operand& other) const;
        bool operator!=(const operand& other) const;
        void update_dtype(const var_info& var);
    };

    /*
        List of icode instructions and icode entry
    */

    enum instruction
    {
        PASS,
        PASS_ADDR,
        CALL,
        RET,
        ADDR_ADD,
        ADDR_MUL,
        EQUAL,
        READ,
        WRITE,
        CREATE_PTR,
        CAST,
        UNARY_MINUS,
        NOT,
        MUL,
        DIV,
        MOD,
        ADD,
        SUB,
        RSH,
        LSH,
        LT,
        LTE,
        GT,
        GTE,
        EQ,
        NEQ,
        BWA,
        BWX,
        BWO,
        GOTO,
        IF_TRUE_GOTO,
        IF_FALSE_GOTO,
        CREATE_LABEL,
        PRINT,
        PRINT_STR,
        SPACE,
        NEWLN,
        INPUT,
        INPUT_STR,
        EXIT
    };

    struct entry
    {
        instruction opcode;
        operand op1;
        operand op2;
        operand op3;

        entry();
    };

    /*
        This struct describes a function,
        it contains return type info, list of parameters and if they are mutable,
        list of local symbols, and icode for the function
    */

    struct func_desc
    {
        var_info func_info;
        std::vector<std::string> params;
        std::map<std::string, var_info> symbols;
        std::vector<icode::entry> icode_table;
        std::string module_name;

        bool symbol_exists(const std::string&);
        bool get_symbol(const std::string&, icode::var_info&);
    };

    /*
        This struct describes the target.
        The map should map default data types like "int", "float"
        or "i8" strings to data_type enum.
    */

    struct target_desc
    {
        std::map<std::string, data_type> dtype_strings_map;
        std::map<std::string, def> defines;
        data_type default_int;
        data_type str_int;

        bool get_def(const std::string&, def&);
    };

    /*
        This struct describes a uhll module, or one file
    */

    struct module_desc
    {
        std::string name;
        std::vector<std::string> uses;
        std::map<std::string, struct_desc> structures;
        std::map<std::string, func_desc> functions;
        std::map<std::string, int> enumerations;
        std::map<std::string, def> defines;
        std::map<std::string, var_info> globals;
        std::map<std::string, std::string> str_data;

        bool use_exists(const std::string&);
        bool get_struct(const std::string&, struct_desc&);
        bool get_func(const std::string&, func_desc&);
        bool get_enum(const std::string&, int&);
        bool get_def(const std::string&, def&);
        bool get_global(const std::string&, var_info&);
        bool symbol_exists(const std::string&, target_desc& target);
    };

    typedef std::map<std::string, module_desc> module_desc_map;

    /*
        Helper functions for type checking and other data type operations.
    */

    bool is_sint(data_type);
    bool is_uint(data_type);
    bool is_int(data_type);
    bool is_float(data_type);
    bool type_eq(var_info var1, var_info var2);
    var_info var_from_dtype(data_type dtype, target_desc& target);
    data_type from_dtype_str(const std::string& dtype_name, target_desc& target);

    /*
        Helper functions for optimizer
    */
    bool is_ltrl(operand_type optype);
    bool is_ptr(operand_type optype);

    /*
        Helper functions for generating icode operands
    */

    operand temp_opr(data_type dtype, const std::string& dtype_name, unsigned int id);
    operand temp_ptr_opr(data_type dtype, const std::string& dtype_name, unsigned int id);
    operand str_dat_opr(const std::string& name, unsigned int size, unsigned int id);
    operand var_opr(data_type dtype,
                    const std::string& dtype_name,
                    const std::string& symbol,
                    unsigned int id,
                    bool global = false,
                    bool ptr = false);
    operand ret_ptr_opr(data_type dtype, const std::string& dtype_name, unsigned int id);
    operand ret_val_opr(data_type dtype, const std::string& dtype_name, unsigned int id);
    operand literal_opr(data_type dtype, float literal, unsigned int id);
    operand literal_opr(data_type dtype, int literal, unsigned int id);
    operand addr_opr(unsigned int address, unsigned int id);
    operand gbl_addr_opr(unsigned int address, unsigned int id);
    operand fp_addr_opr(unsigned int address, unsigned int id);
    operand label_opr(const std::string& label, unsigned int id);
    operand module_opr(const std::string& module, unsigned int id);
}

#endif