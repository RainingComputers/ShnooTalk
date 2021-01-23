#ifndef VMGEN_HPP
#define VMGEN_HPP

#include "icode.hpp"
#include "log.hpp"
#include "optimizer.hpp"
#include "vm.hpp"

namespace uhllvmgen
{
    icode::target_desc target_desc();

    class uhllvm_generator
    {
        /* References to external objects */
        vm::uhllvm& vm;
        opt::optimizer& opt;
        icode::module_desc_map& modules;

        std::string mod_name;
        std::string func_name;

        /* Used for register/address allocation */
        icode::operand acc;
        bool acc_is_addr;
        bool acc_is_live;
        bool acc_in_ir;
        bool acc_mod;
        bool compliment_goto;
        bool gblmode;

        /* Function frame information */
        unsigned int curr_frame_size;
        std::map<icode::operand, unsigned int> temps;
        opt::addr_info* locals;
        opt::addr_info* globals;

        /* Label and back patching information */
        std::map<std::string, unsigned int> label_addr_map;
        std::map<std::string, std::vector<unsigned int>> label_backpatch_q;
        std::vector<unsigned int> fsize_backpatch_q;

        void to_gblmode();

        void to_fpmode();

        unsigned int get_frame_addr(const icode::operand& op);

        void save();

        void save_acc_val();

        void load(const icode::operand& op, bool mod = true);

        void load_addr(const icode::operand& op);

        void eq(const icode::entry& e);

        void addrop(const icode::entry& e);

        void binop(const icode::entry& e);

        void uniop(const icode::entry& e);

        void cmpop(const icode::entry& e);

        void print(const icode::entry& e);

        void print_str(const icode::entry& e);

        void input(const icode::entry& e);

        std::string entry_to_label(const icode::entry& e);

        void create_label(const std::string& label);

        unsigned int get_label_addr(const std::string& label);

        void jmp(const icode::entry& e);

        void call(const icode::entry& e);

        void pass(const icode::entry& e, int param_count);

        void update_acc_live_info(int entry_idx);

        void gen_func(const std::vector<icode::entry>& icode);

      public:
        uhllvm_generator(vm::uhllvm& target_vm,
                         icode::module_desc_map& modules_map,
                         opt::optimizer& optimizer);
    };
}

#endif