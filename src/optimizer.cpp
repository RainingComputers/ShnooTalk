#include "optimizer.hpp"

// DEBUG
#include "log.hpp"

namespace opt
{
    use_info::use_info()
    {
        live = false;
        next_use = NO_NEXT_USE;
    }

    use_map_symtable::use_map_symtable(int no_entries) { exit = no_entries; }

    use_info use_map_symtable::get(const icode::operand& key)
    {
        auto pair = symbol_table.find(key);

        if (pair == symbol_table.end())
        {
            /* Initially, all variables are live on exit and temp are dead
                see dragon book, 8.4.2 pg 528  */

            use_info new_val = use_info();

            if (key.optype == icode::VAR || key.optype == icode::GBL_VAR ||
                key.optype == icode::PTR)
            {
                new_val.live = true;
                new_val.next_use = exit;
            }

            symbol_table[key] = new_val;

            return new_val;
        }
        else
            return pair->second;
    }

    void use_map_symtable::set(const icode::operand& key, bool live, int next_use)
    {
        use_info new_val;
        new_val.live = live;
        new_val.next_use = next_use;

        symbol_table[key] = new_val;
    }

    void use_map_symtable::clear() { symbol_table.clear(); }

    void use_map_symtable::set_use_read(const icode::operand& op,
                                        icode_use_info& func_use_info,
                                        int i)
    {
        if (!icode::is_ltrl(op.optype))
        {
            func_use_info[i][op] = get(op);
            set(op, true, i);
        }
    }

    void use_map_symtable::set_use_write(const icode::operand& op,
                                        icode_use_info& func_use_info,
                                        int i)
    {
        func_use_info[i][op] = get(op);
        set(op, false, NO_NEXT_USE);
    }

    void use_map_symtable::set_use_read_ptr(const icode::operand& op,
                                        icode_use_info& func_use_info,
                                        int i)
    {
        if (icode::is_ptr(op.optype))
        {
            func_use_info[i][op] = get(op);
            set(op, true, i);
        }
    }

    void optimizer::construct_use_info(const std::string& module_name,
                                       const std::string& func_name,
                                       icode::func_desc& func)
    {
        /* Initialize use info vector */
        icode_use_info func_use_info;
        func_use_info.resize(func.icode_table.size());

        /* Initialize symbol table */
        use_map_symtable symbol_table(func.icode_table.size());

        /* Loop through icode, and construct next_use info
            see dragon book, 8.4.2 pg 528  */
        for (int i = func.icode_table.size() - 1; i >= 0; i--)
        {
            icode::entry entry = func.icode_table[i];

            switch (entry.opcode)
            {
                case icode::CREATE_LABEL:
                case icode::IF_TRUE_GOTO:
                case icode::IF_FALSE_GOTO:
                case icode::GOTO:
                    symbol_table.clear();
                    break;
                case icode::CALL:
                case icode::NEWLN:
                case icode::RET:
                    break;
                case icode::PASS:
                case icode::PRINT:
                    /* instr op1 */
                    symbol_table.set_use_read(entry.op1, func_use_info, i);
                    break;
                case icode::CAST:
                case icode::UNARY_MINUS:
                case icode::NOT:
                case icode::MUL:
                case icode::DIV:
                case icode::MOD:
                case icode::ADD:
                case icode::SUB:
                case icode::RSH:
                case icode::LSH:
                case icode::BWA:
                case icode::BWX:
                case icode::BWO:
                case icode::EQUAL:
                case icode::READ:
                    /* op1 <- op2 instr op3 */
                    symbol_table.set_use_write(entry.op1, func_use_info, i);
                    symbol_table.set_use_read(entry.op2, func_use_info, i);
                    symbol_table.set_use_read(entry.op3, func_use_info, i);
                    break;
                case icode::ADDR_ADD:
                case icode::ADDR_MUL:
                    symbol_table.set_use_write(entry.op1, func_use_info, i);
                    symbol_table.set_use_read_ptr(entry.op2, func_use_info, i);
                    symbol_table.set_use_read_ptr(entry.op3, func_use_info, i);                
                    break;
                case icode::PASS_ADDR:
                    symbol_table.set_use_read_ptr(entry.op1, func_use_info, i);
                    break;
                case icode::INPUT:
                case icode::WRITE:
                    if(icode::is_ptr(entry.op1.optype)) 
                        symbol_table.set_use_read(entry.op1, func_use_info, i);
                    else
                        symbol_table.set_use_write(entry.op1, func_use_info, i);
                    break;
                case icode::LT:
                case icode::LTE:
                case icode::GT:
                case icode::GTE:
                case icode::EQ:
                case icode::NEQ:
                    /* op1 instr op2 */
                    symbol_table.set_use_read(entry.op1, func_use_info, i);
                    symbol_table.set_use_read(entry.op2, func_use_info, i);
                    break;
                default:
                    break;
            }
        }

        prog_use_map[module_name][func_name] = func_use_info;
    }

    void optimizer::construct_addr_map_global(const icode::module_desc& mod)
    {
        static unsigned int offset = 0;

        /* Loop through all string data */
        for (auto str : mod.str_data)
        {
            prog_global_addr_map[mod.name][str.first] = offset;
            offset += str.second.size() * icode::dtype_size[target.default_int];
        }

        /* Loop through all global variables */
        for (auto global : mod.globals)
        {
            prog_global_addr_map[mod.name][global.first] = offset;
            offset += global.second.size;
        }

        global_dat_size = offset;
    }

    void optimizer::construct_addr_map_local(const std::string& module_name,
                                             const std::string& func_name,
                                             icode::func_desc& func)
    {
        /* Allocate stack memory for return address and return value */
        unsigned int offset = 2 * icode::dtype_size[target.default_int];

        for (auto symbol : func.symbols)
        {
            prog_local_addr_map[module_name][func_name][symbol.first] = offset;

            /* If it is a pointer, the size will be only one word  */
            if (symbol.second.check(icode::IS_PTR))
                offset += icode::dtype_size[target.default_int];
            else
                offset += symbol.second.size;
        }

        local_vars_size[module_name][func_name] = offset;
    }

    optimizer::optimizer(icode::target_desc& target_desc, icode::module_desc_map& modules)
      : target(target_desc)
    {
        for (auto pair : modules)
        {
            /* Assign addresses for global variables */
            construct_addr_map_global(pair.second);

            /* Optimize module */
            for (auto func : pair.second.functions)
            {
                construct_addr_map_local(pair.first, func.first, func.second);
                construct_use_info(pair.first, func.first, func.second);
            }
        }
    }
} // namespace opt
