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

    enum DataType
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

    /*
        The following structs are used to describe a
        variables, structs, function parameters, defs
    */

    enum VariableProperty
    {
        IS_MUT,
        IS_PTR,
        IS_PARAM,
    };

    struct VariableDescription
    {
        icode::DataType dtype;
        std::string dtypeName;
        std::string moduleName;
        unsigned int dtypeSize;
        unsigned int offset;
        unsigned int size;
        std::vector<unsigned int> dimensions;
        unsigned int scopeId;

        unsigned int properties;

        VariableDescription();

        void setProperty(VariableProperty prop);
        void clearProperty(VariableProperty prop);
        bool checkProperty(VariableProperty prop) const;
    };

    struct StructDescription
    {
        std::map<std::string, VariableDescription> structFields;
        unsigned int size;
        std::string moduleName;

        bool fieldExists(const std::string&);
    };

    struct Define
    {
        union
        {
            int integer;
            float floating;
        } val;

        DataType dtype;
    };

    /*
        Icode operand types and struct
    */

    enum OperandType
    {
        TEMP,
        TEMP_PTR,
        ADDR,
        STR_DATA,
        VAR,
        GBL_VAR,
        PTR,
        RET_PTR,
        CALLEE_RET_VAL,
        LITERAL,
        LABEL,
        MODULE,
        NONE,
    };

    struct Operand
    {
        unsigned int operandId;

        union
        {
            int integer;
            float floating;
            unsigned int address;
            unsigned int size;
        } val;

        std::string name;
        DataType dtype;
        std::string dtypeName;
        OperandType operandType;

        bool operator<(const Operand& other) const;
        bool operator==(const Operand& other) const;
        void update_dtype(const VariableDescription& var);
        bool isPointer();
    };

    /*
        List of icode instructions and icode entry
    */

    enum Instruction
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

    struct Entry
    {
        Instruction opcode;
        Operand op1;
        Operand op2;
        Operand op3;

        Entry();
    };

    /*
        This struct describes a function,
        it contains return type info, list of parameters and if they are mutable,
        list of local symbols, and icode for the function
    */

    struct FunctionDescription
    {
        VariableDescription functionReturnDescription;
        std::vector<std::string> parameters;
        std::map<std::string, VariableDescription> symbols;
        std::vector<icode::Entry> icodeTable;
        std::string moduleName;

        bool symbolExists(const std::string&);
        bool getSymbol(const std::string&, icode::VariableDescription&);
    };

    /*
        This struct describes the target.
        The map should map default data types like "int", "float"
        or "i8" strings to data_type enum.
    */

    struct TargetDescription
    {
        std::map<std::string, DataType> dataTypeNames;
        std::map<std::string, Define> defines;
        DataType characterInt;

        bool get_def(const std::string&, Define&);
    };

    /*
        This struct describes a uhll module, or one file
    */

    struct ModuleDescription
    {
        std::string name;
        std::vector<std::string> uses;
        std::map<std::string, StructDescription> structures;
        std::map<std::string, FunctionDescription> functions;
        std::map<std::string, int> enumerations;
        std::map<std::string, Define> defines;
        std::map<std::string, VariableDescription> globals;
        std::map<std::string, std::string> str_data;

        bool use_exists(const std::string&);
        bool get_struct(const std::string&, StructDescription&);
        bool get_func(const std::string&, FunctionDescription&);
        bool get_enum(const std::string&, int&);
        bool get_def(const std::string&, Define&);
        bool get_global(const std::string&, VariableDescription&);
        bool symbol_exists(const std::string&, TargetDescription& target);
    };

    typedef std::map<std::string, ModuleDescription> StringModulesMap;

    /*
        Helper functions for type checking and other data type operations.
    */

    bool isSignedInteger(DataType);
    bool isUnsignedInteger(DataType);
    bool isInteger(DataType);
    bool isFloat(DataType);
    bool isSameType(VariableDescription var1, VariableDescription var2);
    VariableDescription variableDescriptionFromDataType(DataType dtype, TargetDescription& target);
    DataType dataTypeFromString(const std::string& dtype_name, TargetDescription& target);
    std::string dataTypeToString(const DataType dtype);
    int getDataTypeSize(const DataType dtype);

    /*
        Helper functions for generating icode operands
    */

    Operand createTempOperand(DataType dtype, const std::string& dtype_name, unsigned int id);
    Operand createPointerOperand(DataType dtype, const std::string& dtype_name, unsigned int id);
    Operand createStringDataOperand(const std::string& name, unsigned int size, unsigned int id);
    Operand createVarOperand(DataType dtype,
                             const std::string& dtype_name,
                             const std::string& symbol,
                             unsigned int id,
                             bool global = false,
                             bool ptr = false);
    Operand createRetPointerOperand(DataType dtype, const std::string& dtype_name, unsigned int id);
    Operand createCalleeRetValOperand(DataType dtype, const std::string& dtype_name, unsigned int id);
    Operand createLiteralOperand(DataType dtype, float literal, unsigned int id);
    Operand createLiteralOperand(DataType dtype, int literal, unsigned int id);
    Operand createLiteralAddressOperand(unsigned int address, unsigned int id);
    Operand createLabelOperand(const std::string& label, unsigned int id);
    Operand createModuleOperand(const std::string& module, unsigned int id);
}

#endif