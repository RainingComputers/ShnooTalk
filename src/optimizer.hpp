#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <map>
#include <vector>

#include "icode.hpp"

namespace opt
{
    /* Typedefs and structs used to store next use info for variables and struct */

    const int NO_NEXT_USE = -1;

    struct use_info
    {
        bool live;
        int next_use;

        use_info();
    };

    typedef std::map<icode::operand, use_info> entry_use_info;
    typedef std::vector<entry_use_info> icode_use_info;
    typedef std::map<std::string, std::map<std::string, icode_use_info>> use_map;
    typedef std::map<std::string, unsigned int> addr_info;
    typedef std::map<std::string, std::map<std::string, addr_info>> local_address_map;
    typedef std::map<std::string, addr_info> global_address_map;

    /* Helper struct to construct next use info */

    struct use_map_symtable
    {
        int exit;
        std::map<icode::operand, use_info> symbol_table;
        use_info get(const icode::operand& key);

        use_map_symtable(int no_entries);
        void set(const icode::operand& key, bool live, int next_use);
        void clear();

        void set_use_read(const icode::operand& op, icode_use_info& func_use_info, int i);
        void
        set_use_write(const icode::operand& op, icode_use_info& func_use_info, int i);
        void
        set_use_read_ptr(const icode::operand& op, icode_use_info& func_use_info, int i);
    };

    /* Optimizer class */

    class optimizer
    {
        icode::target_desc& target;

        void construct_use_info(const std::string& module_name,
                                const std::string& func_name,
                                icode::func_desc& func);

        /* Functions that assign address to variables */
        void construct_addr_map_local(const std::string& module_name,
                                      const std::string& func_name,
                                      icode::func_desc& func);

        void construct_addr_map_global(const icode::module_desc& mod);

      public:
        use_map prog_use_map;
        local_address_map prog_local_addr_map;
        global_address_map prog_global_addr_map;
        std::map<std::string, std::map<std::string, unsigned int>> local_vars_size;
        unsigned int global_dat_size;

        optimizer(icode::target_desc& target_desc, icode::module_desc_map& modules);
    };
}

#endif