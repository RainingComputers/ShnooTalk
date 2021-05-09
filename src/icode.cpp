#include "icode.hpp"

namespace icode
{
    static std::string dataTypeStringsArray[] = { "I8",  "UI8",    "I16",     "UI16",     "I32", "UI32",  "I64",    "UI64", "F32",
                                     "F64", "VM_INT", "VM_UINT", "VM_FLOAT", "INT", "FLOAT", "STRUCT", "VOID" };

    Entry::Entry()
    {
        op1.operandType = NONE;
        op2.operandType = NONE;
        op3.operandType = NONE;
    }

    bool Operand::operator<(const Operand& other) const
    {
        if (*this == other)
            return false;

        return operandId < other.operandId;
    }

    bool Operand::operator==(const Operand& other) const
    {
        if (operandType != other.operandType)
            return false;

        switch (operandType)
        {
            case TEMP:
            case TEMP_PTR:
            case RET_PTR:
            case CALLEE_RET_VAL:
                return operandId == other.operandId;
            case VAR:
            case GBL_VAR:
            case PTR:
            case STR_DATA:
            case LABEL:
            case MODULE:
                return name == other.name;
            case ADDR:
            case LITERAL:
                return val.size == other.val.size;
            case NONE:
                return false;
        }

        return false;
    }

    void Operand::update_dtype(const VariableDescription& var)
    {
        dtype = var.dtype;
        dtypeName = var.dtypeName;
    }

    bool Operand::isPointer()
    {
        return operandType == icode::PTR || operandType == icode::TEMP_PTR || operandType == icode::RET_PTR;
    }


    /*
        Helper functions for type checking and other data type operations.
    */

    bool isSignedInteger(DataType dtype)
    {
        return (dtype == I8 || dtype == I16 || dtype == I32 || dtype == I64 || dtype == VM_INT || dtype == INT);
    }

    bool isUnsignedInteger(DataType dtype)
    {
        return (dtype == UI8 || dtype == UI16 || dtype == UI32 || dtype == UI64 || dtype == VM_UINT);
    }

    bool isInteger(DataType dtype) { return (isSignedInteger(dtype) || isUnsignedInteger(dtype)); }

    bool isFloat(DataType dtype) { return (dtype == F32 || dtype == F64 || dtype == FLOAT || dtype == VM_FLOAT); }

    bool dtype_eq(DataType dtype1, DataType dtype2)
    {
        return dtype1 == dtype2 || (dtype1 == INT && isInteger(dtype2)) || (isInteger(dtype1) && dtype2 == INT) ||
               (dtype1 == FLOAT && isFloat(dtype2)) || (isFloat(dtype1) && dtype2 == FLOAT);
    }

    bool isSameType(VariableDescription var1, VariableDescription var2)
    {
        if (var1.dtype == STRUCT || var2.dtype == STRUCT)
            return (var1.dtypeName == var2.dtypeName && var1.dimensions == var2.dimensions &&
                    var1.moduleName == var2.moduleName);

        return (dtype_eq(var1.dtype, var2.dtype) && var1.dimensions == var2.dimensions);
    }

    VariableDescription variableDescriptionFromDataType(DataType dtype, TargetDescription& target)
    {
        VariableDescription var;

        var.dtype = dtype;

        if (dtype == INT)
            var.dtypeName = "int";
        else if (dtype == FLOAT)
            var.dtypeName = "float";
        else if (dtype == VOID)
            var.dtypeName = "void";
        else
        {
            for (auto pair : target.dataTypeNames)
                if (pair.second == dtype)
                    var.dtypeName = pair.first;
        }

        var.dtypeSize = getDataTypeSize(dtype);
        var.size = var.dtypeSize;
        var.offset = 0;
        var.scopeId = 0;

        return var;
    }

    DataType dataTypeFromString(const std::string& dtype_name, TargetDescription& target)
    {
        if (target.dataTypeNames.find(dtype_name) != target.dataTypeNames.end())
            return target.dataTypeNames[dtype_name];

        return STRUCT;
    }

    std::string dataTypeToString(const DataType dtype)
    {
        return dataTypeStringsArray[dtype];
    }

    int getDataTypeSize(const DataType dtype)
    {
        const int dataTypeSizesArray[] = { 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 1, 1, 1, 0, 0, 0, 0 };
        return dataTypeSizesArray[dtype];
    }

    /*
        Helper functions to get and set variable properties
    */

    VariableDescription::VariableDescription() { properties = 0; }

    void VariableDescription::setProperty(VariableProperty prop) { properties |= (1 << prop); }

    void VariableDescription::clearProperty(VariableProperty prop) { properties &= ~(1 << prop); }

    bool VariableDescription::checkProperty(VariableProperty prop) const { return properties & (1 << prop); }

    /*
        Helper functions to check for existence of symbols
        and fetch them
    */

    template<class K, class V>
    bool get_elem(std::map<K, V>& map, const K& key, V& val)
    {
        auto pair = map.find(key);

        if (pair != map.end())
        {
            val = pair->second;
            return true;
        }

        return false;
    }

    bool StructDescription::fieldExists(const std::string& name)
    {
        return structFields.find(name) != structFields.end();
    }

    bool FunctionDescription::symbolExists(const std::string& name) { return symbols.find(name) != symbols.end(); }

    bool FunctionDescription::getSymbol(const std::string& name, VariableDescription& val)
    {
        return get_elem<std::string, VariableDescription>(symbols, name, val);
    }

    bool ModuleDescription::use_exists(const std::string& name)
    {
        return std::find(uses.begin(), uses.end(), name) != uses.end();
    }

    bool ModuleDescription::get_struct(const std::string& name, StructDescription& val)
    {
        return get_elem<std::string, StructDescription>(structures, name, val);
    }

    bool ModuleDescription::get_func(const std::string& name, FunctionDescription& val)
    {
        return get_elem<std::string, FunctionDescription>(functions, name, val);
    }

    bool ModuleDescription::get_enum(const std::string& name, int& val)
    {
        return get_elem<std::string, int>(enumerations, name, val);
    }

    bool ModuleDescription::get_def(const std::string& name, Define& val)
    {
        return get_elem<std::string, Define>(defines, name, val);
    }

    bool ModuleDescription::get_global(const std::string& name, VariableDescription& val)
    {
        return get_elem<std::string, VariableDescription>(globals, name, val);
    }

    bool ModuleDescription::symbol_exists(const std::string& name, TargetDescription& target)
    {
        return structures.find(name) != structures.end() || functions.find(name) != functions.end() ||
               use_exists(name) || dataTypeFromString(name, target) != STRUCT ||
               enumerations.find(name) != enumerations.end() || globals.find(name) != globals.end() ||
               defines.find(name) != defines.end() || target.defines.find(name) != target.defines.end();
    }

    bool TargetDescription::get_def(const std::string& name, Define& val)
    {
        return get_elem<std::string, Define>(defines, name, val);
    }

    /*
        Helper functions for generating icode operands
    */

    Operand createTempOperand(DataType dtype, const std::string& dtype_name, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.dtype = dtype;
        temp.dtypeName = dtype_name;
        temp.operandType = TEMP;

        return temp;
    }

    Operand createPointerOperand(DataType dtype, const std::string& dtype_name, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.dtype = dtype;
        temp.dtypeName = dtype_name;
        temp.operandType = TEMP_PTR;

        return temp;
    }

    Operand createStringDataOperand(const std::string& name, unsigned int size, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.name = name;
        temp.dtype = icode::UI8;
        temp.operandType = STR_DATA;
        temp.val.size = size;

        return temp;
    }

    Operand createLiteralAddressOperand(unsigned int address, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.val.address = address;
        temp.operandType = ADDR;

        return temp;
    }

    Operand createVarOperand(DataType dtype,
                             const std::string& dtype_name,
                             const std::string& symbol,
                             unsigned int id,
                             bool global,
                             bool ptr)
    {
        Operand temp;
        temp.operandId = id;
        temp.name = symbol;
        temp.dtype = dtype;
        temp.dtypeName = dtype_name;

        if (global)
            temp.operandType = GBL_VAR;
        else if (ptr)
            temp.operandType = PTR;
        else
            temp.operandType = VAR;

        return temp;
    }

    Operand createRetPointerOperand(DataType dtype, const std::string& dtype_name, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.dtype = dtype;
        temp.dtypeName = dtype_name;
        temp.operandType = RET_PTR;

        return temp;
    }

    Operand createCalleeRetValOperand(DataType dtype, const std::string& dtype_name, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.dtype = dtype;
        temp.dtypeName = dtype_name;
        temp.operandType = CALLEE_RET_VAL;

        return temp;
    }

    Operand createLiteralOperand(DataType dtype, float literal, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.val.floating = literal;
        temp.dtype = dtype;
        temp.operandType = LITERAL;

        return temp;
    }

    Operand createLiteralOperand(DataType dtype, int literal, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.val.integer = literal;
        temp.dtype = dtype;
        temp.operandType = LITERAL;

        return temp;
    }

    Operand createLabelOperand(const std::string& label, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.name = label;
        temp.operandType = LABEL;

        return temp;
    }

    Operand createModuleOperand(const std::string& module, unsigned int id)
    {
        Operand temp;
        temp.operandId = id;
        temp.name = module;
        temp.operandType = MODULE;

        return temp;
    }
} // namespace icode
