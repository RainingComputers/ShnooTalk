#include "IRPrinter.hpp"

namespace mikpp
{
    void printToken(const token::Token& symbol)
    {
        /* Prints token and its properties */

        std::cout << "Token(\"" << symbol.toString() << "\", " << tokenTypeToString[symbol.getType()]
                  << ", line=" << symbol.getLine() << ", col=" << symbol.getColumn() << ")";
    }

    void printNode(const node::Node& node, int depth)
    {
        /* Recursively prints tree, used to print AST */

        static std::vector<bool> isLastChild;

        std::cout << "Node(" << nodeTypeToString[node.type] << ", ";
        printToken(node.tok);
        std::cout << ")" << std::endl;

        isLastChild.push_back(false);

        for (size_t i = 0; i < node.children.size(); i++)
        {
            for (size_t j = 1; j <= depth; j++)
                if (j == depth)
                    if (i == node.children.size() - 1)
                        std::cout << "└"
                                  << "──";
                    else
                        std::cout << "├"
                                  << "──";
                else if (isLastChild[j - 1])
                    std::cout << " "
                              << "  ";
                else
                    std::cout << "│"
                              << "  ";

            if (i == node.children.size() - 1)
                isLastChild[depth - 1] = true;

            printNode(node.children[i], depth + 1);
        }

        isLastChild.pop_back();
    }

    void printOperand(const icode::Operand& op)
    {
        switch (op.operandType)
        {
            case icode::TEMP:
                std::cout << "Temp(id_" << op.operandId << ":" << op.dtypeName;
                break;
            case icode::TEMP_PTR:
                std::cout << "TempPtr(id_" << op.operandId << ":" << op.dtypeName;
                break;
            case icode::STR_DATA:
                std::cout << "StrDat(name=" << op.name << " size=" << op.val.size;
                break;
            case icode::ADDR:
                std::cout << "Addr(" << op.val.address;
                break;
            case icode::VAR:
                std::cout << "Var(" << op.name << ":" << op.dtypeName;
                break;
            case icode::GBL_VAR:
                std::cout << "GblVar(" << op.name << ":" << op.dtypeName;
                break;
            case icode::PTR:
                std::cout << "Ptr(" << op.name << ":" << op.dtypeName;
                break;
            case icode::RET_PTR:
                std::cout << "RetPtr(id_" << op.operandId << ":" << op.dtypeName;
                break;
            case icode::CALLEE_RET_VAL:
                std::cout << "CalleeRetVal(id_" << op.operandId << ":" << op.dtypeName;
                break;
            case icode::LITERAL:
            {
                if (icode::isUnsignedInteger(op.dtype))
                    std::cout << "Ltrl(" << op.val.size << ":" << icode::dataTypeToString(op.dtype);
                else if (icode::isInteger(op.dtype))
                    std::cout << "Ltrl(" << op.val.integer << ":" << icode::dataTypeToString(op.dtype);
                else
                    std::cout << "Ltrl(" << op.val.floating << ":" << icode::dataTypeToString(op.dtype);

                break;
            }
            case icode::LABEL:
                std::cout << "Label(" << op.name;
                break;
            case icode::MODULE:
                std::cout << "Module(" << op.name;
                break;
            case icode::NONE:
                return;
                break;
        }

        std::cout << ")";
    }

    void printEntry(const icode::Entry& entry)
    {
        std::cout << instructionToString[entry.opcode] << " ";
        printOperand(entry.op1);
        std::cout << " ";
        printOperand(entry.op2);
        std::cout << " ";
        printOperand(entry.op3);
    }

    void printVariableDescription(const icode::VariableDescription& variableDesc)
    {
        std::cout << "Var(";
        std::cout << "dtype="
                  << "\"" << variableDesc.dtypeName << "\":";
        std::cout << icode::dataTypeToString(variableDesc.dtype);
        std::cout << " dtypesize=" << variableDesc.dtypeSize;
        std::cout << " mod="
                  << "\"" << variableDesc.moduleName << "\"";
        std::cout << " offset=" << variableDesc.offset;
        std::cout << " size=" << variableDesc.size;

        std::cout << " dim=[";
        for (unsigned int i : variableDesc.dimensions)
            std::cout << i << ",";
        std::cout << "]";

        std::cout << ")";
    }

    void printStructDescription(const icode::StructDescription& structDesc, int ilvl)
    {
        std::cout << "Struct(" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Fields={" << std::endl;
        for (auto field : structDesc.structFields)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << field.first << ":";
            printVariableDescription(field.second);
            std::cout << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Size=" << structDesc.size << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Module=" << structDesc.size << std::endl;

        std::cout << std::string(ilvl, ' ') << ")" << std::endl;
    }

    void printDef(const icode::Define& definition)
    {
        std::cout << "Def(";

        if (definition.dtype == icode::INT)
            std::cout << definition.val.integer;
        else
            std::cout << definition.val.floating;

        std::cout << ":" << icode::dataTypeToString(definition.dtype) << ")";
    }

    void printFunctionDescription(const icode::FunctionDescription& functionDesc, int ilvl)
    {
        std::cout << "Func(" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Params=[";
        for (std::string param : functionDesc.parameters)
            std::cout << param << ", ";
        std::cout << "]" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Retinfo=";
        printVariableDescription(functionDesc.functionReturnDescription);
        std::cout << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Symbols={" << std::endl;
        for (auto symbol : functionDesc.symbols)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << symbol.first << ":";
            printVariableDescription(symbol.second);
            std::cout << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "icode=[" << std::endl;
        for (icode::Entry i : functionDesc.icodeTable)
        {
            std::cout << std::string(ilvl + 6, ' ');
            printEntry(i);
            std::cout << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "]" << std::endl;

        std::cout << std::string(ilvl, ' ') << ")" << std::endl;
    }

    void printModuleDescription(const icode::ModuleDescription& moduleDesc, int ilvl)
    {
        std::cout << "Module(" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Name=";
        std::cout << "\"" + moduleDesc.name + "\"" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Uses=[";
        for (std::string use : moduleDesc.uses)
            std::cout << use << ", ";
        std::cout << "]" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Enums={" << std::endl;
        for (auto enumeration : moduleDesc.enumerations)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << enumeration.first << "=";
            std::cout << enumeration.second << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Defs={" << std::endl;
        for (auto definition : moduleDesc.defines)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << definition.first << ":";
            printDef(definition.second);
            std::cout << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Globals={" << std::endl;
        for (auto symbol : moduleDesc.globals)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << symbol.first << ":";
            printVariableDescription(symbol.second);
            std::cout << std::endl;
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Sructs={" << std::endl;
        for (auto structure : moduleDesc.structures)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << structure.first << ":";
            printStructDescription(structure.second, ilvl + 6);
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << std::string(ilvl + 3, ' ') << "Functs={" << std::endl;
        for (auto function : moduleDesc.functions)
        {
            std::cout << std::string(ilvl + 6, ' ');
            std::cout << function.first << ":";
            printFunctionDescription(function.second, ilvl + 6);
        }
        std::cout << std::string(ilvl + 3, ' ') << "}" << std::endl;

        std::cout << ")" << std::endl;
    }

} // namespace mikpp