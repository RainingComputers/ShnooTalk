#include "irgen.hpp"

namespace irgen
{
    ir_generator::ir_generator(icode::target_desc& target_desc,
                               icode::module_desc_map& modules_map,
                               const std::string& file_name,
                               std::ifstream& ifile)

      : target(target_desc)
      , ext_modules_map(modules_map)
      , module(modules_map[file_name])
      , file(ifile)
    {
        current_func_desc = nullptr;
        current_ext_module = &module;

        id_counter = 0;
        scope_id_counter = 0;

        module.name = file_name;
    }

    unsigned int ir_generator::id() { return id_counter++; }

    unsigned int ir_generator::get_scope_id() { return scope_id_stack.back(); }

    void ir_generator::enter_scope() { scope_id_stack.push_back(++scope_id_counter); }

    void ir_generator::exit_scope() { scope_id_stack.pop_back(); }

    void ir_generator::clear_scope()
    {
        scope_id_counter = 0;
        scope_id_stack.clear();
        scope_id_stack.push_back(0);
    }

    bool ir_generator::in_scope(unsigned int scope_id)
    {
        return std::find(scope_id_stack.begin(), scope_id_stack.end(), scope_id) !=
               scope_id_stack.end();
    }

    bool ir_generator::get_def(const std::string& name, icode::def& def)
    {
        if (target.get_def(name, def))
            return true;
        else if ((*current_ext_module).get_def(name, def))
            return true;
        else if (module.get_def(name, def))
            return true;

        return false;
    }

    bool ir_generator::get_func(const std::string& name, icode::func_desc& func)
    {
        if ((*current_ext_module).get_func(name, func))
            return true;
        else if (module.get_func(name, func))
            return true;

        return false;
    }

    bool ir_generator::get_enum(const std::string& name, int& val)
    {
        if (module.get_enum(name, val))
            return true;
        else if ((*current_ext_module).get_enum(name, val))
            return true;

        return false;
    }

    std::pair<token::token, icode::var_info>
    ir_generator::var_from_node(const node::node& root)
    {
        unsigned int i = 1;

        icode::var_info var_info;

        token::token name_token = root.children[0].tok;
        token::token dtype_token;

        icode::module_desc* var_module = &module;

        /* Get token that specifies the type */
        bool is_void = false;
        while (true)
        {
            if (i == root.children.size())
            {
                is_void = true;
                break;
            }

            if (root.children[i].type == node::MODULE)
            {
                /* Check if module exists */
                if (!(*var_module).use_exists(root.children[i].tok.str))
                {
                    log::error_tok(
                      module.name, "Module does not exist", file, root.children[i].tok);
                    throw log::compile_error();
                }

                /* Switch module */
                var_module = &ext_modules_map[root.children[i].tok.str];
            }

            if (root.children[i].type != node::IDENTIFIER)
            {
                i++;
                continue;
            }

            dtype_token = root.children[i].tok;
            i++;
            break;
        }

        icode::data_type dtype;

        if (is_void)
            dtype = icode::VOID;
        else
            dtype = icode::from_dtype_str(dtype_token.str, target);

        /* Check if data type exists and get size */
        unsigned int size;
        if (dtype == icode::STRUCT)
        {
            icode::struct_desc struct_desc;

            if (!(*var_module).get_struct(dtype_token.str, struct_desc))
            {
                log::error_tok(module.name, "Symbol does not exist", file, dtype_token);
                throw log::compile_error();
            }

            size = struct_desc.size;
            var_info.module_name = struct_desc.module_name;
        }
        else
        {
            size = icode::dtype_size[dtype];
            var_info.module_name = (*var_module).name;
        }

        /* Construct var info */
        if (is_void)
            var_info.dtype_name = "void";
        else
            var_info.dtype_name = dtype_token.str;
        var_info.dtype = dtype;
        var_info.offset = 0;
        var_info.dtype_size = size;
        var_info.size = size;
        var_info.scope_id = get_scope_id();

        /* Add external module */
        if (var_info.module_name != module.name &&
            !module.use_exists(var_info.module_name))
            module.uses.push_back(var_info.module_name);

        /* Get array dimensions */
        while (i < root.children.size())
        {
            if (root.children[i].type != node::SUBSCRIPT)
                break;

            std::string literal_str = root.children[i].children[0].tok.str;
            unsigned int dim = std::stoi(literal_str);
            var_info.dimensions.push_back(dim);

            /* Update size if an array */
            var_info.size *= dim;

            i++;
        }

        return std::pair<token::token, icode::var_info>(name_token, var_info);
    }

    void ir_generator::use(const node::node& root)
    {
        for (node::node child : root.children)
        {
            /* Get module name */
            token::token name_token = child.tok;

            bool is_module = pathchk::file_exists(name_token.str + ".uhll");
            bool is_package = pathchk::dir_exists(name_token.str);

            /* Check if file exists */
            if (!(is_module || is_package))
            {
                log::error_tok(
                  module.name, "Module or Package does not exist", file, name_token);
                throw log::compile_error();
            }

            /* Check for conflicts */
            if (is_module && is_package)
            {
                log::error_tok(module.name,
                               "Module and Package exists with same name",
                               file,
                               name_token);
                throw log::compile_error();
            }

            /* Check for multiple imports */
            if (module.use_exists(name_token.str))
            {
                log::error_tok(
                  module.name, "Multiple imports detected", file, name_token);
                throw log::compile_error();
            }

            /* Check for name conflict */
            if (module.symbol_exists(name_token.str, target))
            {
                log::error_tok(
                  module.name, "Name conflict, symbol already exists", file, name_token);
                throw log::compile_error();
            }

            /* Check for self import */
            if (module.name == name_token.str)
            {
                log::error_tok(module.name, "Self import not allowed", file, name_token);
                throw log::compile_error();
            }

            /* Add to icode */
            module.uses.push_back(name_token.str);
        }
    }

    void ir_generator::from(const node::node& root)
    {
        icode::struct_desc struct_desc;
        icode::func_desc func_desc;
        icode::def def;
        int enum_val;

        /* Get ext module */
        if (!module.use_exists(root.children[0].tok.str))
        {
            log::error_tok(
              module.name, "Module not imported", file, root.children[0].tok);
            throw log::compile_error();
        }

        icode::module_desc* ext_module = &ext_modules_map[root.children[0].tok.str];

        for (node::node child : root.children[1].children)
        {
            /* Check if symbol exists */
            if (module.symbol_exists(child.tok.str, target))
            {
                log::error_tok(module.name,
                               "Symbol already defined in current module",
                               file,
                               child.tok);
                throw log::compile_error();
            }

            /* If it is struct */
            if ((*ext_module).get_struct(child.tok.str, struct_desc))
                module.structures[child.tok.str] = struct_desc;
            /* If it a function */
            else if ((*ext_module).get_func(child.tok.str, func_desc))
            {
                log::error_tok(module.name, "Cannot import functions", file, child.tok);
                throw log::compile_error();
            }
            /* If is a def */
            else if ((*ext_module).get_def(child.tok.str, def))
                module.defines[child.tok.str] = def;
            /* If it is a enum */
            else if ((*ext_module).get_enum(child.tok.str, enum_val))
                module.enumerations[child.tok.str] = enum_val;
            /* Check if use exists */
            else if ((*ext_module).use_exists(child.tok.str))
                module.uses.push_back(child.tok.str);
            /* Does not exist */
            else
            {
                log::error_tok(module.name, "Symbol does not exist", file, child.tok);
                throw log::compile_error();
            }
        }
    }

    void ir_generator::enumeration(const node::node& root)
    {
        for (size_t i = 0; i < root.children.size(); i++)
        {
            token::token enum_tok = root.children[i].tok;

            if (module.symbol_exists(enum_tok.str, target))
            {
                log::error_tok(module.name, "Symbol already defined", file, enum_tok);
                throw log::compile_error();
            }

            module.enumerations[enum_tok.str] = i;
        }
    }

    void ir_generator::def(const node::node& root)
    {
        icode::def definition;
        token::token ltrl_token = root.children[1].tok;

        /* Check if the symbol already exists */
        if (module.symbol_exists(root.children[0].tok.str, target))
        {
            log::error_tok(
              module.name, "Symbol already exists", file, root.children[0].tok);
            throw log::compile_error();
        }

        /* Extract literal value */
        if (ltrl_token.type == token::INT_LITERAL)
        {
            definition.val.integer = std::stoi(ltrl_token.str);
            definition.dtype = icode::INT;
        }
        else
        {
            definition.val.floating = (float)std::stof(ltrl_token.str);
            definition.dtype = icode::FLOAT;
        }

        /* Add to definitions */
        module.defines[root.children[0].tok.str] = definition;
    }

    void ir_generator::structure(const node::node& root)
    {
        icode::struct_desc struct_desc;
        struct_desc.size = 0;

        token::token name_token = root.children[0].tok;

        /* Check if symbol exists */
        if (module.symbol_exists(name_token.str, target))
        {
            log::error_tok(module.name, "Symbol already defined", file, name_token);
            throw log::compile_error();
        }

        /* Go through field of structure */
        for (node::node field : root.children[0].children)
        {
            std::pair<token::token, icode::var_info> var = var_from_node(field);

            /* Set mutable for var */
            var.second.set_prop(icode::IS_MUT);

            /* Check if the name is already a field */
            if (struct_desc.field_exists(var.first.str))
            {
                log::error_tok(module.name, "Field already defined", file, var.first);
                throw log::compile_error();
            }
            else if (module.symbol_exists(var.first.str, target))
            {
                log::error_tok(module.name, "Symbol already defined", file, var.first);
                throw log::compile_error();
            }

            /* Update struct size and offset */
            var.second.offset = struct_desc.size;
            struct_desc.size += var.second.size;

            /* Append to feilds map */
            struct_desc.fields[var.first.str] = var.second;

            /* Module name */
            struct_desc.module_name = module.name;
        }

        /* Add strucuture definition to module */
        module.structures[name_token.str] = struct_desc;
    }

    void ir_generator::fn(const node::node& root)
    {
        icode::func_desc func_desc;
        std::string func_name;

        std::pair<token::token, icode::var_info> var = var_from_node(root);
        func_name = var.first.str;
        func_desc.func_info = var.second;

        /* Check if function name symbol already exists */
        if (module.symbol_exists(func_name, target))
        {
            log::error_tok(
              module.name, "Symbol already defined", file, root.children[0].tok);
            throw log::compile_error();
        }

        /* Process function parameters */
        for (size_t i = 1;; i++)
        {
            bool mut;
            if (root.children[i].type == node::PARAM)
                mut = false;
            else if (root.children[i].type == node::MUT_PARAM)
                mut = true;
            else
                break;

            std::pair<token::token, icode::var_info> param_var =
              var_from_node(root.children[i]);

            if (mut)
                param_var.second.set_prop(icode::IS_MUT);

            if (mut || param_var.second.dtype == icode::STRUCT ||
                param_var.second.dimensions.size() != 0)
                param_var.second.set_prop(icode::IS_PTR);

            /* Check if symbol is already defined */
            if (module.symbol_exists(param_var.first.str, target))
            {
                log::error_tok(
                  module.name, "Symbol already defined", file, param_var.first);
                throw log::compile_error();
            }

            /* Append to symbol table */
            func_desc.params.push_back(param_var.first.str);
            func_desc.symbols[param_var.first.str] = param_var.second;
        }

        func_desc.module_name = module.name;

        /* Add function definition to module */
        module.functions[func_name] = func_desc;
    }

    void ir_generator::global_var(const node::node& root)
    {
        std::pair<token::token, icode::var_info> var = var_from_node(root);

        /* Set mut for var */
        var.second.set_prop(icode::IS_MUT);

        /* Check if symbol already exists */
        if (module.symbol_exists(var.first.str, target))
        {
            log::error_tok(module.name, "Symbol already defined", file, var.first);
            throw log::compile_error();
        }

        /* Add to symbol table */
        module.globals[var.first.str] = var.second;
    }

    std::vector<int> ir_generator::str_literal_toint(const token::token& str_token)
    {
        std::vector<int> str_int;

        /* Loop through each character, convert them to ASCII and append
            them to int vector */
        bool backslash = false;
        for (size_t i = 1; i < str_token.str.size() - 1; i++)
        {
            char c = str_token.str[i];
            int character;
            /* Process character */
            if (c == '\\' && !backslash)
            {
                backslash = true;
                continue;
            }
            else if (backslash)
            {
                switch (c)
                {
                    case 'n':
                        character = (int)'\n';
                        break;
                    case 'b':
                        character = (int)'\b';
                        break;
                    case 't':
                        character = (int)'\t';
                        break;
                    case '0':
                        character = 0;
                        break;
                    default:
                        character = (int)c;
                        break;
                }

                backslash = false;
            }
            else
            {
                character = (int)c;
            }

            str_int.push_back(character);
        }

        /* Append null character */
        str_int.push_back(0);

        return str_int;
    }

    op_var_pair
    ir_generator::gen_str_dat(const token::token& str_token, icode::var_info var)
    {
        if (var.dimensions.size() != 1 || !icode::is_int(var.dtype))
        {
            log::error_tok(module.name,
                           "String assignment only allowed on 1D INT ARRAY",
                           file,
                           str_token);
            throw log::compile_error();
        }

        std::string name = "_str_l" + std::to_string(str_token.lineno) + "_c" +
                           std::to_string(str_token.col);

        /* Convert string to int ASCII list, get size */
        std::vector<int> str_int = str_literal_toint(str_token);
        size_t char_count = str_int.size();
        size_t size = char_count * icode::dtype_size[var.dtype];

        /* Append string data */
        module.str_data[name] = str_int;

        /* Check size */
        if (char_count > var.dimensions[0])
        {
            log::error_tok(module.name, "String too big", file, str_token);
            throw log::compile_error();
        }

        /* Create icode::operand */
        icode::operand opr = icode::gen_str_dat_opr(name, size, id());

        return op_var_pair(opr, var);
    }

    void ir_generator::assign_str_literal_tovar(op_var_pair var, node::node& root)
    {
        if (var.second.dimensions.size() != 1 || !icode::is_int(var.second.dtype))
        {
            log::error_tok(module.name,
                           "String assignment only allowed on 1D INT ARRAY",
                           file,
                           root.tok);
            throw log::compile_error();
        }

        /* Create Addr Temp */
        icode::operand curr_offset = var.first;

        /* Convert string literal to vector of ASCII ints */
        std::vector<int> str_int = str_literal_toint(root.tok);
        int char_count = str_int.size();

        /* Check size */
        if (char_count > var.second.dimensions[0])
        {
            log::error_tok(module.name, "String too big", file, root.tok);
            throw log::compile_error();
        }

        /* Loop through int and initialize string */
        for (size_t i = 0; i < str_int.size(); i++)
        {
            int character = str_int[i];

            /* Write to current offset */
            icode::entry write_entry;
            write_entry.op1 = curr_offset;
            write_entry.op2 = icode::gen_literal_opr(target.default_int, character, id());
            write_entry.opcode = icode::EQUAL;
            (*current_func_desc).icode_table.push_back(write_entry);

            /* Update offset */
            if (i != str_int.size() - 1)
            {
                icode::entry offset_entry;
                offset_entry.op1 = icode::gen_temp_ptr_opr(var.second.dtype, id());
                offset_entry.op2 = curr_offset;
                offset_entry.op3 = icode::gen_addr_opr(var.second.dtype_size, id());
                offset_entry.opcode = icode::ADDR_ADD;
                (*current_func_desc).icode_table.push_back(offset_entry);
                curr_offset = offset_entry.op1;
            }
        }
    }

    void ir_generator::copy_array(icode::operand& left, op_var_pair right)
    {
        icode::operand curr_offset_left = left;
        icode::operand curr_offset_right = right.first;

        unsigned int size = right.second.size;
        unsigned int dtype_size = right.second.dtype_size;

        /* Loop through array and copy each element */
        for (size_t i = 0; i < size; i += dtype_size)
        {
            if (right.second.dtype == icode::STRUCT)
            {
                /* Copy struct from right to left */
                copy_struct(curr_offset_left, op_var_pair(curr_offset_right, right.second));
            }
            else
            {
                /* Copy element from right to left */
                icode::entry read_entry;
                read_entry.op1 = curr_offset_left;
                read_entry.op2 = curr_offset_right;
                read_entry.opcode = icode::EQUAL;
                (*current_func_desc).icode_table.push_back(read_entry);
            }

            /* Update offset */
            if (i != size - dtype_size)
            {
                icode::operand update = icode::gen_addr_opr(dtype_size, id());

                icode::entry left_update;
                left_update.op1 = icode::gen_temp_ptr_opr(right.second.dtype, id());
                left_update.op2 = curr_offset_left;
                left_update.op3 = update;
                left_update.opcode = icode::ADDR_ADD;
                (*current_func_desc).icode_table.push_back(left_update);
                curr_offset_left = left_update.op1;

                icode::entry right_update;
                right_update.op1 = icode::gen_temp_ptr_opr(right.second.dtype, id());
                right_update.op2 = curr_offset_right;
                right_update.op3 = update;
                right_update.opcode = icode::ADDR_ADD;
                (*current_func_desc).icode_table.push_back(right_update);
                curr_offset_right = right_update.op1;
            }
        }
    }

    void ir_generator::copy_struct(icode::operand& left, op_var_pair right)
    {
        icode::operand curr_offset_left = left;
        icode::operand curr_offset_right = right.first;

        /* Loop through each field and copy them */
        unsigned int count = 0;
        icode::operand update;
        for (auto field : module.structures[right.second.dtype_name].fields)
        {
            icode::var_info field_info = field.second;

            if (count != 0)
            {
                icode::entry left_update;
                left_update.op1 = icode::gen_temp_ptr_opr(field_info.dtype, id());
                left_update.op2 = curr_offset_left;
                left_update.op3 = update;
                left_update.opcode = icode::ADDR_ADD;
                (*current_func_desc).icode_table.push_back(left_update);
                curr_offset_left = left_update.op1;

                icode::entry right_update;
                right_update.op1 = icode::gen_temp_ptr_opr(field_info.dtype, id());
                right_update.op2 = curr_offset_right;
                right_update.op3 = update;
                right_update.opcode = icode::ADDR_ADD;
                (*current_func_desc).icode_table.push_back(right_update);
                curr_offset_right = right_update.op1;
            }

            /* Copy field */
            if (field_info.dimensions.size() != 0)
            {
                copy_array(curr_offset_left, op_var_pair(curr_offset_right, field_info));
            }
            else if (field_info.dtype != icode::STRUCT)
            {
                /* Copy field from right into left */
                icode::entry read_entry;
                read_entry.op1 = curr_offset_left;
                read_entry.op2 = curr_offset_right;
                read_entry.opcode = icode::EQUAL;
                (*current_func_desc).icode_table.push_back(read_entry);
            }
            else
            {
                copy_struct(curr_offset_left, op_var_pair(curr_offset_right, field_info));
            }

            update = icode::gen_addr_opr(field_info.size, id());

            count++;
        }
    }

    void ir_generator::assign_init_list_tovar(op_var_pair var, node::node& root)
    {
        /* Cannot use initializer list to assign to var */
        if (var.second.dimensions.size() == 0)
        {
            log::error_tok(module.name,
                           "Cannot initialize a NON-ARRAY with initializer list",
                           file,
                           root.tok);
            throw log::compile_error();
        }

        icode::operand curr_offset = var.first;

        /* Create var info for the elements inside the list */
        icode::var_info element_var = var.second;
        element_var.size /= element_var.dimensions[0];
        element_var.dimensions.erase(element_var.dimensions.begin());

        /* Keep track of number of dimensions written */
        unsigned int dim_count = 0;

        for (size_t i = 0; i < root.children.size(); i++)
        {
            node::node child = root.children[i];

            if (dim_count >= var.second.dimensions[0])
            {
                log::error_tok(module.name, "Dimension size too big", file, child.tok);
                throw log::compile_error();
            }

            if (element_var.dimensions.size() == 0)
            {
                /* Check if expression */
                if (child.type != node::TERM && child.type != node::EXPRESSION)
                {
                    log::error_tok(module.name, "Incorrect dimensions", file, child.tok);
                    throw log::compile_error();
                }

                op_var_pair element_expr = expression(child);

                /* Type check */
                if (!icode::type_eq(element_var, element_expr.second))
                {
                    log::type_error(
                      module.name, file, child.tok, element_var, element_expr.second);
                    throw log::compile_error();
                }

                if (element_expr.second.dtype != icode::STRUCT)
                {
                    /* Write to current offset if not a struct */
                    icode::entry write_entry;
                    write_entry.op1 = curr_offset;
                    write_entry.op2 = element_expr.first;
                    write_entry.opcode = icode::EQUAL;
                    (*current_func_desc).icode_table.push_back(write_entry);
                }
                else
                {
                    copy_struct(curr_offset, element_expr);
                }
            }
            else if (child.type == node::STR_LITERAL)
            {
                assign_str_literal_tovar(op_var_pair(curr_offset, element_var), child);
            }
            else
            {
                assign_init_list_tovar(op_var_pair(curr_offset, element_var), child);
            }

            dim_count++;

            /* Update offset */
            if (i != root.children.size() - 1)
            {
                icode::entry offset_entry;
                offset_entry.op1 = icode::gen_temp_ptr_opr(element_var.dtype, id());
                offset_entry.op2 = curr_offset;
                offset_entry.op3 = icode::gen_addr_opr(element_var.size, id());
                offset_entry.opcode = icode::ADDR_ADD;
                (*current_func_desc).icode_table.push_back(offset_entry);
                curr_offset = offset_entry.op1;
            }
        }

        if (dim_count < var.second.dimensions[0])
        {
            log::error_tok(module.name, "Dimension size too small", file, root.tok);
            throw log::compile_error();
        }
    }

    void ir_generator::var(const node::node& root)
    {
        std::pair<token::token, icode::var_info> var = var_from_node(root);

        /* Set mutable for var */
        if (root.type == node::VAR)
            var.second.set_prop(icode::IS_MUT);

        /* Check if symbol already exists */
        if (module.symbol_exists(var.first.str, target) ||
            (*current_func_desc).symbol_exists(var.first.str))
        {
            log::error_tok(module.name, "Symbol already defined", file, var.first);
            throw log::compile_error();
        }

        /* Check for initialization expression or initializer list */
        node::node last_node = root.children.back();

        if (last_node.type == node::EXPRESSION || last_node.type == node::TERM)
        {
            /* If an array requires initializer list */
            if (var.second.dimensions.size() != 0)
            {
                log::error_tok(module.name,
                               "Initializer list required to initialize array",
                               file,
                               last_node.tok);
                throw log::compile_error();
            }

            /* Create icode operands, one for variable other for temp
                to hold result of initialization expression */
            icode::operand left =
              icode::gen_var_opr(var.second.dtype, var.first.str, id());

            op_var_pair init_exp = expression(last_node);

            /* Check if the type match */
            if (!icode::type_eq(var.second, init_exp.second))
            {
                log::type_error(
                  module.name, file, last_node.tok, var.second, init_exp.second);
                throw log::compile_error();
            }

            if (var.second.dtype != icode::STRUCT)
            {
                /* Create EQUAL icode entry is not a STRUCT */
                icode::entry entry;
                entry.op1 = left;
                entry.op2 = init_exp.first;
                entry.opcode = icode::EQUAL;
                (*current_func_desc).icode_table.push_back(entry);
            }
            else
            {
                copy_struct(left, init_exp);
            }
        }
        else if (last_node.type == node::STR_LITERAL)
        {
            op_var_pair var_pair = op_var_pair(
              icode::gen_var_opr(var.second.dtype, var.first.str, id()), var.second);
            assign_str_literal_tovar(var_pair, last_node);
        }
        else if (last_node.type == node::INITLIST)
        {
            op_var_pair var_pair = op_var_pair(
              icode::gen_var_opr(var.second.dtype, var.first.str, id()), var.second);
            assign_init_list_tovar(var_pair, last_node);
        }

        /* Add to symbol table */
        (*current_func_desc).symbols[var.first.str] = var.second;
    }

    op_var_pair ir_generator::var_access(const node::node& root)
    {
        icode::operand current_offset_temp = icode::gen_addr_opr(0, id());
        icode::var_info current_var_info;
        std::string ident_name;
        unsigned int dim_count = 0;
        unsigned int rem_dim = 0;
        bool is_global = false;
        bool is_ptr = false;

        int enum_val;
        icode::def def;

        /* Check if identifier exists and get dtype and size */
        node::node child = root.children[0];
        ident_name = child.tok.str;
        if ((*current_func_desc).get_symbol(ident_name, current_var_info))
        {
            is_ptr = current_var_info.check(icode::IS_PTR);
        }
        else if (module.get_global(ident_name, current_var_info))
        {
            is_global = true;
        }
        else if (get_enum(ident_name, enum_val))
        {
            /* Check if there is STRUCT or ARRAY access of enum */
            if (root.children.size() > 1)
            {
                log::error_tok(module.name, "Invalid use of ENUM", file, child.tok);
                throw log::compile_error();
            }

            icode::operand op = icode::gen_literal_opr(icode::INT, enum_val, id());
            icode::var_info var = icode::var_from_dtype(icode::INT, target);
            return op_var_pair(op, var);
        }
        else if (get_def(ident_name, def))
        {
            /* Check if there is STRUCT or ARRAY access of def */
            if (root.children.size() > 1)
            {
                log::error_tok(module.name, "Invalid use of DEF", file, child.tok);
                throw log::compile_error();
            }

            icode::operand op;

            if (def.dtype == icode::INT)
                op = icode::gen_literal_opr(icode::INT, def.val.integer, id());
            else
                op = icode::gen_literal_opr(icode::FLOAT, def.val.floating, id());

            icode::var_info var = icode::var_from_dtype(def.dtype, target);
            return op_var_pair(op, var);
        }
        else
        {
            log::error_tok(module.name, "Symbol does not exist", file, child.tok);
            throw log::compile_error();
        }

        /* Check if the variable is available in the current scope */
        if (!in_scope(current_var_info.scope_id))
        {
            log::error_tok(module.name, "Symbol not in scope", file, child.tok);
            throw log::compile_error();
        }

        /* If no struct or subscript */
        if (root.children.size() == 1)
        {
            icode::operand op = icode::gen_var_opr(
              current_var_info.dtype, ident_name, id(), is_global, is_ptr);
            return op_var_pair(op, current_var_info);
        }

        /* Go through struct fields and subsripts */
        for (size_t i = 1; i < root.children.size();)
        {
            node::node child = root.children[i];

            switch (child.type)
            {
                case node::STRUCT_VAR:
                {
                    child = root.children[++i];

                    if (current_var_info.dtype != icode::STRUCT)
                    {
                        log::error_tok(module.name,
                                       "STRUCT access on a NON-STRUCT data type",
                                       file,
                                       child.tok);
                        throw log::compile_error();
                    }
                    else if (dim_count == 0 && current_var_info.dimensions.size() != 0)
                    {
                        log::error_tok(
                          module.name, "STRUCT access on an ARRAY", file, child.tok);
                        throw log::compile_error();
                    }
                    else
                    {
                        /* Get struct description */
                        icode::struct_desc struct_desc;
                        struct_desc = ext_modules_map[current_var_info.module_name]
                                        .structures[current_var_info.dtype_name];

                        /* Check if field exists */
                        if (struct_desc.field_exists(child.tok.str))
                        {
                            /* If it does, update offset */
                            current_var_info = struct_desc.fields[child.tok.str];

                            /* Add offset */
                            icode::entry entry;
                            entry.op1 =
                              icode::gen_temp_ptr_opr(current_var_info.dtype, id());
                            entry.op2 = current_offset_temp;
                            entry.op3 =
                              icode::gen_addr_opr(current_var_info.offset, id());
                            entry.opcode = icode::ADDR_ADD;

                            (*current_func_desc).icode_table.push_back(entry);

                            current_offset_temp = entry.op1;

                            dim_count = 0;
                        }
                        else
                        {
                            log::error_tok(
                              module.name, "Undefined STRUCT field", file, child.tok);
                            throw log::compile_error();
                        }
                    }
                    i++;
                    break;
                }
                case node::SUBSCRIPT:
                {
                    dim_count = 1;

                    if (current_var_info.dimensions.size() == 0)
                    {
                        log::error_tok(
                          module.name, "ARRAY access on a NON ARRAY", file, child.tok);
                        throw log::compile_error();
                    }

                    unsigned int elem_width =
                      current_var_info.size / current_var_info.dimensions[0];

                    while (child.type == node::SUBSCRIPT)
                    {
                        if (dim_count > current_var_info.dimensions.size())
                        {
                            log::error_tok(
                              module.name, "Too many subscripts", file, child.tok);
                            throw log::compile_error();
                        }

                        icode::entry subs_entry;

                        /* Operand to store result of expression */
                        op_var_pair subs_expr = expression(child.children[0]);
                        subs_entry.op2 = subs_expr.first;

                        /* Operand to store offset */
                        subs_entry.op1 =
                          icode::gen_temp_ptr_opr(subs_expr.first.dtype, id());

                        /* Check if int expression */
                        if (!icode::is_int(subs_expr.second.dtype) ||
                            subs_expr.second.dimensions.size() != 0)
                        {
                            log::error_tok(module.name,
                                           "Index must be an integer",
                                           file,
                                           child.children[0].tok);
                            throw log::compile_error();
                        }

                        /* Get element width multiplier */
                        subs_entry.op3 = icode::gen_addr_opr(elem_width, id());

                        if (dim_count != current_var_info.dimensions.size())
                            elem_width /= current_var_info.dimensions[dim_count];

                        subs_entry.opcode = icode::ADDR_MUL;

                        /* Create entry for adding to the current offset */
                        icode::entry offset_entry;
                        offset_entry.op1 =
                          icode::gen_temp_ptr_opr(subs_expr.first.dtype, id());
                        offset_entry.op2 = current_offset_temp;
                        offset_entry.op3 = subs_entry.op1;
                        offset_entry.opcode = icode::ADDR_ADD;

                        /* Update icode table */
                        (*current_func_desc).icode_table.push_back(subs_entry);
                        (*current_func_desc).icode_table.push_back(offset_entry);

                        current_offset_temp = offset_entry.op1;

                        if (i < root.children.size() - 1)
                        {
                            child = root.children[++i];
                            if (child.type == node::SUBSCRIPT)
                                dim_count++;
                        }
                        else
                        {
                            i++;
                            break;
                        };
                    }

                    /* After indexing, only some dimensions will be left */
                    rem_dim = current_var_info.dimensions.size() - dim_count;

                    current_var_info.dimensions.erase(current_var_info.dimensions.begin(),
                                                      current_var_info.dimensions.end() -
                                                        rem_dim);

                    break;
                }
                default:
                {
                    log::internal_error_tok(module.name, file, child.tok);
                    throw log::internal_bug_error();
                }
            }
        }

        /* Add var to offset */
        icode::entry add_entry;
        add_entry.op1 = icode::gen_temp_ptr_opr(current_var_info.dtype, id());
        add_entry.op2 = current_offset_temp;
        add_entry.op3 =
          icode::gen_var_opr(current_var_info.dtype, ident_name, id(), is_global, is_ptr);
        add_entry.opcode = icode::ADDR_ADD;
        (*current_func_desc).icode_table.push_back(add_entry);

        /* Return var */
        return op_var_pair(add_entry.op1, current_var_info);
    }

    op_var_pair ir_generator::funccall(const node::node& root)
    {
        std::vector<icode::entry> pass_entries;
        icode::module_desc* temp = current_ext_module;

        /* Get the first argument (if not a string literal) */
        op_var_pair first_arg;
        if (root.children.size() != 0)
        {
            if (root.children[0].type != node::STR_LITERAL)
                first_arg = expression(root.children[0]);

            /* If struct funccall, switch to struct's (first arg's) module */
            if (root.type == node::STRUCT_FUNCCALL)
                current_ext_module = &ext_modules_map[first_arg.second.module_name];
        }

        /* Check if function exits */
        icode::func_desc func_desc;
        icode::data_type func_dtype;
        unsigned int return_size;
        std::string func_name = root.tok.str;

        if (!get_func(func_name, func_desc))
        {
            log::error_tok(module.name, "Function does not exist", file, root.tok);
            throw log::compile_error();
        }
        else
        {
            func_dtype = func_desc.func_info.dtype;
            return_size = func_desc.func_info.size;
        }

        /* Check number of parameters */
        if (root.children.size() != func_desc.params.size())
        {
            log::error_tok(
              module.name, "Number of parameters don't match", file, root.tok);
            throw log::compile_error();
        }

        /* Go through each parameter */
        for (size_t i = 0; i < root.children.size(); i++)
        {
            /* Get parameter information */
            icode::var_info param = func_desc.symbols[func_desc.params[i]];
            bool mut = param.check(icode::IS_MUT);

            /* Get argument passed to function */
            op_var_pair arg;
            if (root.children[i].type == node::STR_LITERAL)
            {
                arg = gen_str_dat(root.children[i].tok, param);
            }
            else if (i == 0 && root.children[0].type != node::STR_LITERAL)
                arg = first_arg;
            else
                arg = expression(root.children[i]);

            /* Type check */
            if (!icode::type_eq(param, arg.second))
            {
                log::type_error(
                  module.name, file, root.children[i].tok, param, arg.second);
                throw log::compile_error();
            }

            /* If mutable, it has to be a variable or address */
            if (mut &&
                (arg.first.optype == icode::TEMP || arg.first.optype == icode::STR_DATA ||
                 arg.first.optype == icode::LITERAL))
            {
                log::error_tok(module.name,
                               "Cannot pass an EXPRESSION or STRING LITERAL as MUTABLE",
                               file,
                               root.children[i].tok);
                throw log::compile_error();
            }

            /* If param is mut, arg has to be mutable to */
            if (mut && !arg.second.check(icode::IS_MUT))
            {
                log::error_tok(module.name,
                               "Cannot pass IMMUTABLE as MUTABLE",
                               file,
                               root.children[i].tok);
                throw log::compile_error();
            }

            /* Pass arguments */
            icode::entry entry;
            entry.op1 = arg.first;
            entry.op2 = icode::gen_var_opr(func_dtype, func_name, id());
            entry.op3 = icode::gen_module_opr(func_desc.module_name, id());

            /* If mut or struct or array, pass by ref */
            if (mut || param.dtype == icode::STRUCT || param.dimensions.size() != 0)
                entry.opcode = icode::PASS_ADDR;
            /* Else pass by value */
            else
                entry.opcode = icode::PASS;

            pass_entries.push_back(entry);
        }

        /* Add pass entries to icode table */
        (*current_func_desc)
          .icode_table.insert((*current_func_desc).icode_table.end(),
                              pass_entries.begin(),
                              pass_entries.end());

        /* Call function */
        icode::entry call_entry;
        call_entry.op1 = icode::gen_temp_opr(func_dtype, return_size, id());
        call_entry.op2 = icode::gen_var_opr(func_dtype, func_name, id());
        call_entry.op3 = icode::gen_module_opr(func_desc.module_name, id());
        call_entry.opcode = icode::CALL;

        (*current_func_desc).icode_table.push_back(call_entry);

        /* Switch back to current module if ext_mod was modified */
        if (root.type == node::STRUCT_FUNCCALL)
            current_ext_module = temp;

        return op_var_pair(call_entry.op1, func_desc.func_info);
    }

    op_var_pair ir_generator::size_of(const node::node& root)
    {
        std::string ident = root.children.back().tok.str;

        /* Enter the specified ext module */
        icode::module_desc* current_module = &module;

        node::node mod_node = root.children[0];

        int i = 0;
        while (mod_node.type == node::MODULE)
        {
            std::string mod_name = mod_node.tok.str;

            /* Check if module exists */
            if (!(*current_module).use_exists(mod_name))
            {
                log::error_tok(module.name, "Module does not exist", file, mod_node.tok);
                throw log::compile_error();
            }

            /* Swtich to module */
            current_module = &ext_modules_map[mod_name];

            i++;
            mod_node = root.children[i];
        }

        /* Get size of type */
        int size = 0;
        icode::data_type dtype = icode::from_dtype_str(ident, target);

        icode::struct_desc struct_desc;
        icode::var_info global;
        icode::var_info symbol;

        if (dtype != icode::STRUCT)
            size = icode::dtype_size[dtype];
        else if ((*current_module).get_struct(ident, struct_desc))
            size = struct_desc.size;
        else if ((*current_module).get_global(ident, global))
            size = global.size;
        else if ((*current_func_desc).get_symbol(ident, symbol))
            size = symbol.dtype_size;
        else
        {
            log::error_tok(module.name, "Symbol not found", file, root.tok);
            throw log::compile_error();
        }

        /* return a icode::INT literal  */
        return op_var_pair(icode::gen_literal_opr(icode::INT, size, id()),
                           icode::var_from_dtype(icode::INT, target));
    }

    op_var_pair ir_generator::term(const node::node& root)
    {
        node::node child = root.children[0];
        switch (child.type)
        {
            case node::LITERAL:
            {
                switch (child.tok.type)
                {
                    case token::INT_LITERAL:
                    case token::HEX_LITERAL:
                    case token::BIN_LITERAL:
                    {
                        /* Return literal icode operand */
                        int literal = std::stoi(child.tok.str);
                        icode::data_type dtype = icode::INT;
                        return op_var_pair(gen_literal_opr(dtype, literal, id()),
                                           icode::var_from_dtype(dtype, target));

                        break;
                    }
                    case token::CHAR_LITERAL:
                    {
                        int character;
                        char c = child.tok.str[1];

                        if (c != '\\')
                            character = (int)c;
                        else
                        {
                            c = child.tok.str[2];

                            switch (c)
                            {
                                case 'n':
                                    character = (int)'\n';
                                    break;
                                case 'b':
                                    character = (int)'\b';
                                    break;
                                case 't':
                                    character = (int)'\t';
                                    break;
                                case '0':
                                    character = 0;
                                    break;
                                default:
                                    character = (int)c;
                                    break;
                            }
                        }

                        icode::data_type dtype = icode::INT;
                        return op_var_pair(gen_literal_opr(dtype, character, id()),
                                           icode::var_from_dtype(dtype, target));
                    }
                    case token::FLOAT_LITERAL:
                    {
                        /* Return literal icode operand */
                        icode::data_type dtype = icode::FLOAT;
                        float literal = (float)stof(child.tok.str);
                        return op_var_pair(gen_literal_opr(dtype, literal, id()),
                                           icode::var_from_dtype(dtype, target));

                        break;
                    }
                    default:
                    {
                        log::internal_error_tok(module.name, file, child.tok);
                        throw log::internal_bug_error();
                    }
                }
            }
            case node::IDENTIFIER:
            {
                return var_access(root);
            }
            case node::CAST:
            {
                icode::data_type cast_dtype =
                  icode::from_dtype_str(child.tok.str, target);

                unsigned int cast_size = icode::dtype_size[cast_dtype];

                /* Create icode entry for casting */
                icode::entry entry;
                entry.opcode = icode::CAST;
                entry.op1 = icode::gen_temp_opr(cast_dtype, cast_size, id());

                op_var_pair cast_term = term(child.children[0]);

                /* Cannot cast ARRAY */
                if (cast_term.second.dimensions.size() != 0 ||
                    cast_term.second.dtype == icode::STRUCT)
                {
                    log::error_tok(
                      module.name, "Cannot cast STRUCT or ARRAY", file, child.tok);
                    throw log::compile_error();
                }

                entry.op2 = cast_term.first;
                entry.op3 = icode::gen_dtype_opr(cast_dtype, id());
                (*current_func_desc).icode_table.push_back(entry);

                /* Return temp */
                return op_var_pair(entry.op1, icode::var_from_dtype(cast_dtype, target));
            }
            case node::UNARY_OPR:
            {
                icode::entry entry;

                op_var_pair term_var = term(child.children[0]);

                icode::data_type dtype = term_var.second.dtype;
                unsigned int size = icode::dtype_size[dtype];
                entry.op1 = icode::gen_temp_opr(dtype, size, id());

                /* Unary operator not allowed on ARRAY */
                if (term_var.second.dimensions.size() != 0)
                {
                    log::error_tok(module.name,
                                   "Unary operator not allowed on ARRAY",
                                   file,
                                   child.tok);
                    throw log::compile_error();
                }

                /* Unary operator not allowed on STRUCT */
                if (dtype == icode::STRUCT)
                {
                    log::error_tok(module.name,
                                   "Unary operator not allowed on STRUCT",
                                   file,
                                   child.tok);
                    throw log::compile_error();
                }

                /* NOT operator not allowed on float */
                if (!icode::is_int(dtype) && child.tok.type == token::NOT)
                {
                    log::error_tok(module.name,
                                   "Unary operator NOT not allowed on FLOAT",
                                   file,
                                   child.tok);
                    throw log::compile_error();
                }

                entry.op2 = term_var.first;

                switch (child.tok.type)
                {
                    case token::MINUS:
                        entry.opcode = icode::UNARY_MINUS;
                        break;
                    case token::NOT:
                        entry.opcode = icode::NOT;
                        break;
                    case token::CONDN_NOT:
                        log::error_tok(
                          module.name, "Did not expect CONDN NOT", file, child.tok);
                        throw log::compile_error();
                    default:
                        log::internal_error_tok(module.name, file, child.tok);
                        throw log::internal_bug_error();
                }

                (*current_func_desc).icode_table.push_back(entry);

                /* Return temp */
                return op_var_pair(entry.op1, term_var.second);
            }
            case node::EXPRESSION:
            {
                return expression(child);
            }
            case node::STRUCT_FUNCCALL:
            case node::FUNCCALL:
            {
                return funccall(child);
            }
            case node::MODULE:
            {
                node::node_type mod_child_type = child.children[0].children[0].type;
                if (mod_child_type != node::FUNCCALL &&
                    mod_child_type != node::IDENTIFIER && mod_child_type != node::MODULE)
                {
                    log::error_tok(
                      module.name, "Invalid use of MODULE ACCESS", file, child.tok);
                    throw log::compile_error();
                }

                /* Switch to external module */
                icode::module_desc* temp = current_ext_module;

                /* Check if the module exists */
                if (!(*current_ext_module).use_exists(child.tok.str))
                {
                    log::error_tok(module.name, "Module does not exist", file, child.tok);
                    throw log::compile_error();
                }

                current_ext_module = &ext_modules_map[child.tok.str];

                op_var_pair ret_val = term(child.children[0]);

                /* Switch back to self */
                current_ext_module = temp;

                return ret_val;
            }
            case node::SIZEOF:
            {
                return size_of(child);
            }
            default:
            {
                log::internal_error_tok(module.name, file, child.tok);
                throw log::internal_bug_error();
            }
        }

        return op_var_pair(icode::operand(), icode::var_from_dtype(icode::VOID, target));
    }

    op_var_pair ir_generator::expression(const node::node& root)
    {
        if (root.type == node::TERM)
        {
            /* Single child, no operator, process term */
            return term(root);
        }
        else if (root.children.size() == 1)
        {
            return expression(root.children[0]);
        }
        else
        {
            token::token expr_opr = root.children[1].tok;

            icode::entry entry;

            /* First operand */
            op_var_pair first_operand = expression(root.children[0]);
            entry.op2 = first_operand.first;
            icode::data_type dtype = first_operand.second.dtype;
            unsigned int size = icode::dtype_size[dtype];

            /* Expression not allowed on arrays or struct */
            if (dtype == icode::STRUCT || first_operand.second.dimensions.size() != 0)
            {
                log::error_tok(
                  module.name, "Operator not allowed on STRUCT or ARRAY", file, expr_opr);
                throw log::compile_error();
            }

            /* Second operand */
            op_var_pair second_operand = expression(root.children[2]);
            entry.op3 = second_operand.first;

            /* Type check */
            if (!icode::type_eq(first_operand.second, second_operand.second))
            {
                log::type_error(module.name,
                                file,
                                root.children[2].tok,
                                first_operand.second,
                                second_operand.second);
                throw log::compile_error();
            }

            /* If bitwise operator, dtype has to be int */
            if (expr_opr.type == token::BITWISE_OR ||
                expr_opr.type == token::BITWISE_AND ||
                expr_opr.type == token::BITWISE_XOR)
            {
                if (!icode::is_int(dtype))
                {
                    log::error_tok(module.name,
                                   "Bitwise operations not allowed on FLOAT",
                                   file,
                                   expr_opr);
                    throw log::compile_error();
                }
            }

            /* If second operand is a ptr, read it into a temp */
            if (entry.op3.optype == icode::PTR || entry.op3.optype == icode::TEMP_PTR)
            {
                icode::entry read_entry;
                read_entry.op1 = icode::gen_temp_opr(dtype, size, id());
                read_entry.op2 = entry.op3;
                read_entry.opcode = icode::EQUAL;
                (*current_func_desc).icode_table.push_back(read_entry);

                entry.op3 = read_entry.op1;
            }

            /* Temp operand to store the result */
            entry.op1 = icode::gen_temp_opr(dtype, size, id());

            /* Generate corresponding opcode for operator */
            switch (expr_opr.type)
            {
                case token::MULTIPLY:
                    entry.opcode = icode::MUL;
                    break;
                case token::DIVIDE:
                    entry.opcode = icode::DIV;
                    break;
                case token::MOD:
                    entry.opcode = icode::MOD;
                    break;
                case token::PLUS:
                    entry.opcode = icode::ADD;
                    break;
                case token::MINUS:
                    entry.opcode = icode::SUB;
                    break;
                case token::RIGHT_SHIFT:
                    entry.opcode = icode::RSH;
                    break;
                case token::LEFT_SHIFT:
                    entry.opcode = icode::LSH;
                    break;
                case token::BITWISE_AND:
                    entry.opcode = icode::BWA;
                    break;
                case token::BITWISE_XOR:
                    entry.opcode = icode::BWX;
                    break;
                case token::BITWISE_OR:
                    entry.opcode = icode::BWO;
                    break;
                case token::CONDN_AND:
                case token::CONDN_OR:
                case token::LESS_THAN:
                case token::LESS_THAN_EQUAL:
                case token::GREATER_THAN:
                case token::GREATER_THAN_EQUAL:
                case token::CONDN_EQUAL:
                case token::CONDN_NOT_EQUAL:
                    log::error_tok(module.name,
                                   "Did not expect conditional operator",
                                   file,
                                   root.children[1].tok);
                    throw log::compile_error();
                    break;
                default:
                    log::internal_error_tok(module.name, file, expr_opr);
                    throw log::internal_bug_error();
            }

            (*current_func_desc).icode_table.push_back(entry);

            /* Return the operand where final result is stored */
            return op_var_pair(entry.op1, first_operand.second);
        }
    }

    void ir_generator::assignment(const node::node& root)
    {
        /* The variable to write to */
        op_var_pair var = var_access(root.children[0]);

        /* Assignment expression */
        op_var_pair expr = expression(root.children[2]);

        /* Assignement operator */
        token::token assign_opr = root.children[1].tok;

        /* Var connot be enum of a def */
        if (var.first.optype == icode::LITERAL)
        {
            log::error_tok(
              module.name, "Cannot assign to LITERAL", file, root.children[0].tok);
            throw log::compile_error();
        }

        /* Check if type matches */
        if (!icode::type_eq(var.second, expr.second))
        {
            log::type_error(module.name, file, root.children[2].tok, var.second, expr.second);
            throw log::compile_error();
        }

        /* Check if mutable */
        if (!var.second.check(icode::IS_MUT))
        {
            log::error_tok(module.name,
                           "Cannot modify IMMUTABLE variable or parameter",
                           file,
                           root.children[0].tok);
            throw log::compile_error();
        }

        /* Assignment operator not allowed on arrays */
        if (var.second.dimensions.size() != 0)
        {
            log::error_tok(
              module.name, "Assignment operators not allowed on ARRAY", file, assign_opr);
            throw log::compile_error();
        }

        /* Only EQUAL operator allowed on STRUCT */
        if (var.second.dtype == icode::STRUCT && assign_opr.type != token::EQUAL)
        {
            log::error_tok(
              module.name, "Only EQUAL operator allowed on STRUCT", file, assign_opr);
            throw log::compile_error();
        }

        /* If bitwise operator, dtype has to be int */
        if (assign_opr.type == token::OR_EQUAL || assign_opr.type == token::AND_EQUAL ||
            assign_opr.type == token::XOR_EQUAL)
        {
            if (!icode::is_int(var.second.dtype))
            {
                log::error_tok(module.name,
                               "Bitwise operation not allowed on FLOAT",
                               file,
                               assign_opr);
                throw log::compile_error();
            }
        }

        /* Create icode entry */
        icode::entry entry;

        /* Convert token type to opcode */
        switch (assign_opr.type)
        {
            case token::EQUAL:
                entry.opcode = icode::EQUAL;
                break;
            case token::PLUS_EQUAL:
                entry.opcode = icode::ADD;
                break;
            case token::MINUS_EQUAL:
                entry.opcode = icode::SUB;
                break;
            case token::DIVIDE_EQUAL:
                entry.opcode = icode::DIV;
                break;
            case token::MULTIPLY_EQUAL:
                entry.opcode = icode::MUL;
                break;
            case token::OR_EQUAL:
                entry.opcode = icode::BWO;
                break;
            case token::AND_EQUAL:
                entry.opcode = icode::BWA;
                break;
            case token::XOR_EQUAL:
                entry.opcode = icode::BWX;
                break;
            default:
                log::internal_error_tok(module.name, file, assign_opr);
                throw log::internal_bug_error();
        }

        /* If struct */
        if (var.second.dtype == icode::STRUCT)
        {
            copy_struct(var.first, expr);
        }
        /* If not a struct field */
        else if (var.first.optype == icode::VAR || var.first.optype == icode::GBL_VAR)
        {
            if (assign_opr.type == token::EQUAL)
            {
                entry.op1 = var.first;
                entry.op2 = expr.first;
                (*current_func_desc).icode_table.push_back(entry);
            }
            else
            {
                entry.op1 = var.first;
                entry.op2 = var.first;
                entry.op3 = expr.first;
                (*current_func_desc).icode_table.push_back(entry);
            }
        }
        /* If a pointer */
        else
        {
            if (assign_opr.type == token::EQUAL)
            {
                entry.op1 = var.first;
                entry.op2 = expr.first;
                entry.opcode = icode::EQUAL;
                (*current_func_desc).icode_table.push_back(entry);
            }
            else
            {
                /* Add the read temp value to expr and store it in
                    another temp */
                entry.op1 = var.first;
                entry.op2 = var.first;

                if (expr.first.optype == icode::PTR ||
                    expr.first.optype == icode::TEMP_PTR)
                {
                    /* If pointer read into a temp */
                    icode::entry read_entry;
                    unsigned int size = icode::dtype_size[expr.first.dtype];
                    read_entry.op1 = icode::gen_temp_opr(expr.first.dtype, size, id());
                    read_entry.op2 = expr.first;
                    read_entry.opcode = icode::EQUAL;
                    (*current_func_desc).icode_table.push_back(read_entry);

                    entry.op3 = read_entry.op1;
                }
                else
                    entry.op3 = expr.first;

                (*current_func_desc).icode_table.push_back(entry);
            }
        }
    }

    icode::operand
    ir_generator::gen_label(token::token tok, bool true_label, std::string prefix)
    {
        /* Generate label using token's line and col number */

        std::string label_name =
          "_l" + std::to_string(tok.lineno) + "_c" + std::to_string(tok.col);

        if (true_label)
            return icode::gen_label_opr("_" + prefix + "_true" + label_name, id());
        else
            return icode::gen_label_opr("_" + prefix + "_false" + label_name, id());
    }

    void ir_generator::condn_expression(const node::node& root,
                                        const icode::operand& t_label,
                                        const icode::operand& f_label,
                                        bool t_fall,
                                        bool f_fall)
    {
        if (root.type == node::TERM)
        {
            /* For unary operator NOT */
            if (root.children[0].type == node::EXPRESSION)
            {
                condn_expression(root.children[0], t_label, f_label, t_fall, f_fall);
                return;
            }
            else if (root.children[0].tok.type != token::CONDN_NOT)
            {
                log::error_tok(
                  module.name, "Invalid conditional expression", file, root.tok);
                throw log::compile_error();
            }

            condn_expression(
              root.children[0].children[0].children[0], f_label, t_label, f_fall, t_fall);
        }
        else if (root.children.size() == 1)
        {
            condn_expression(root.children[0], t_label, f_label, t_fall, f_fall);
        }
        else
        {
            token::token expr_opr = root.children[1].tok;

            /* See the dragon book, Figure 6.39 and Figure 6.40 */

            switch (expr_opr.type)
            {
                case token::CONDN_AND:
                {
                    icode::operand new_t_label = gen_label(expr_opr, true);
                    icode::operand new_f_label = gen_label(expr_opr, false);

                    if (!f_fall)
                        condn_expression(
                          root.children[0], new_t_label, f_label, true, false);
                    else
                        condn_expression(
                          root.children[0], new_t_label, new_f_label, true, false);

                    condn_expression(root.children[2], t_label, f_label, t_fall, f_fall);

                    if (f_fall)
                    {
                        icode::entry label_entry;
                        label_entry.op1 = new_f_label;
                        label_entry.opcode = icode::CREATE_LABEL;
                        (*current_func_desc).icode_table.push_back(label_entry);
                    }

                    break;
                }
                case token::CONDN_OR:
                {
                    icode::operand new_t_label = gen_label(expr_opr, true);
                    icode::operand new_f_label = gen_label(expr_opr, false);

                    if (!t_fall)
                        condn_expression(
                          root.children[0], t_label, new_f_label, false, true);
                    else
                        condn_expression(
                          root.children[0], new_t_label, new_f_label, false, true);

                    condn_expression(root.children[2], t_label, f_label, t_fall, f_fall);

                    if (t_fall)
                    {
                        icode::entry label_entry;
                        label_entry.op1 = new_t_label;
                        label_entry.opcode = icode::CREATE_LABEL;
                        (*current_func_desc).icode_table.push_back(label_entry);
                    }

                    break;
                }
                default:
                {
                    icode::entry comp_entry;

                    switch (expr_opr.type)
                    {
                        case token::LESS_THAN:
                            comp_entry.opcode = icode::LT;
                            break;
                        case token::LESS_THAN_EQUAL:
                            comp_entry.opcode = icode::LTE;
                            break;
                        case token::GREATER_THAN:
                            comp_entry.opcode = icode::GT;
                            break;
                        case token::GREATER_THAN_EQUAL:
                            comp_entry.opcode = icode::GTE;
                            break;
                        case token::CONDN_EQUAL:
                            comp_entry.opcode = icode::EQ;
                            break;
                        case token::CONDN_NOT_EQUAL:
                            comp_entry.opcode = icode::NEQ;
                            break;
                        default:
                            log::internal_error_tok(module.name, file, expr_opr);
                            throw log::internal_bug_error();
                    }

                    /* Create icode entry for comparing two expressions */
                    op_var_pair first_operand = expression(root.children[0]);
                    comp_entry.op1 = first_operand.first;

                    /* Cannot compare structs and arrays */
                    if (first_operand.second.dtype == icode::STRUCT ||
                        first_operand.second.dimensions.size() != 0)
                    {
                        log::error_tok(
                          module.name, "Cannot compare STRUCT or ARRAYS", file, expr_opr);
                        throw log::compile_error();
                    }

                    op_var_pair second_operand = expression(root.children[2]);
                    comp_entry.op2 = second_operand.first;

                    /* Type check */
                    if (!icode::type_eq(first_operand.second, second_operand.second))
                    {
                        log::type_error(module.name,
                                        file,
                                        root.children[2].tok,
                                        first_operand.second,
                                        second_operand.second);
                        throw log::compile_error();
                    }

                    /* If second operand is a ptr, read it into a temp */
                    if (comp_entry.op2.optype == icode::PTR ||
                        comp_entry.op2.optype == icode::TEMP_PTR)
                    {
                        icode::entry read_entry;
                        unsigned int size = icode::dtype_size[comp_entry.op2.dtype];
                        read_entry.op1 =
                          icode::gen_temp_opr(comp_entry.op2.dtype, size, id());
                        read_entry.op2 = comp_entry.op2;
                        read_entry.opcode = icode::EQUAL;
                        (*current_func_desc).icode_table.push_back(read_entry);

                        comp_entry.op2 = read_entry.op1;
                    }

                    /* Add to icode */
                    (*current_func_desc).icode_table.push_back(comp_entry);

                    /* Create icode entry for goto */
                    icode::entry goto_entry;

                    if (!t_fall)
                    {
                        goto_entry.op1 = t_label;
                        goto_entry.opcode = icode::IF_TRUE_GOTO;
                    }

                    if (!f_fall)
                    {
                        goto_entry.op1 = f_label;
                        goto_entry.opcode = icode::IF_FALSE_GOTO;
                    }

                    (*current_func_desc).icode_table.push_back(goto_entry);
                }
            }
        }
    }

    void ir_generator::ifstmt(const node::node& root,
                              bool loop,
                              const icode::operand& start_label,
                              const icode::operand& break_label,
                              const icode::operand& cont_label)
    {
        /* Create label for end of all if statements */
        icode::operand end_label = gen_label(root.tok, false, "ifend");

        for (size_t i = 0; i < root.children.size(); i++)
        {
            node::node child = root.children[i];

            icode::operand new_t_label = gen_label(child.tok, true, "if");
            icode::operand new_f_label = gen_label(child.tok, false, "if");

            if (child.type != node::ELSE)
            {
                /* Process conditional expression */
                condn_expression(
                  child.children[0], new_t_label, new_f_label, true, false);

                /* Process block */
                block(child.children[1], loop, start_label, break_label, cont_label);

                if (i != root.children.size() - 1)
                {
                    /* Go to end */
                    icode::entry goto_entry;
                    goto_entry.op1 = end_label;
                    goto_entry.opcode = icode::GOTO;
                    (*current_func_desc).icode_table.push_back(goto_entry);
                }

                /* Create label to skip block */
                icode::entry label_entry;
                label_entry.op1 = new_f_label;
                label_entry.opcode = icode::CREATE_LABEL;
                (*current_func_desc).icode_table.push_back(label_entry);
            }
            else
            {
                block(child.children[0], loop, start_label, break_label, cont_label);
            }
        }

        if (root.children.size() != 1)
        {
            /* Create label for end of if statement */
            icode::entry end_entry;
            end_entry.op1 = end_label;
            end_entry.opcode = icode::CREATE_LABEL;
            (*current_func_desc).icode_table.push_back(end_entry);
        }
    }

    void ir_generator::whileloop(const node::node& root)
    {
        icode::operand new_t_label = gen_label(root.tok, true, "while");
        icode::operand new_f_label = gen_label(root.tok, false, "while");

        /* Create label for looping */
        icode::entry loop_entry;
        loop_entry.op1 = new_t_label;
        loop_entry.opcode = icode::CREATE_LABEL;
        (*current_func_desc).icode_table.push_back(loop_entry);

        /* Process conditional expression */
        condn_expression(root.children[0], new_t_label, new_f_label, true, false);

        /* Process block */
        block(root.children[1], true, new_t_label, new_f_label, new_t_label);

        /* Go back to beginning */
        icode::entry goto_entry;
        goto_entry.op1 = new_t_label;
        goto_entry.opcode = icode::GOTO;
        (*current_func_desc).icode_table.push_back(goto_entry);

        /* Create label to skip block */
        icode::entry skip_entry;
        skip_entry.op1 = new_f_label;
        skip_entry.opcode = icode::CREATE_LABEL;
        (*current_func_desc).icode_table.push_back(skip_entry);
    }

    void ir_generator::forloop(const node::node& root)
    {
        /* Process initialization */
        if (root.children[0].type == node::VAR)
            var(root.children[0]);
        else
            assignment(root.children[0]);

        /* Process conditional  */
        icode::operand new_t_label = gen_label(root.tok, true, "for");
        icode::operand new_f_label = gen_label(root.tok, false, "for");
        icode::operand cont_label = gen_label(root.tok, true, "for_cont");

        /* Create label for looping */
        icode::entry loop_entry;
        loop_entry.op1 = new_t_label;
        loop_entry.opcode = icode::CREATE_LABEL;
        (*current_func_desc).icode_table.push_back(loop_entry);

        /* Process conditional expression */
        condn_expression(root.children[1], new_t_label, new_f_label, true, false);

        /* Process block */
        block(root.children[3], true, new_t_label, new_f_label, cont_label);

        /* Create label for continue */
        icode::entry cont_entry;
        cont_entry.op1 = cont_label;
        cont_entry.opcode = icode::CREATE_LABEL;
        (*current_func_desc).icode_table.push_back(cont_entry);

        /* Process assignment */
        assignment(root.children[2]);

        /* Go back to beginning */
        icode::entry goto_entry;
        goto_entry.op1 = new_t_label;
        goto_entry.opcode = icode::GOTO;
        (*current_func_desc).icode_table.push_back(goto_entry);

        /* Create label to skip block */
        icode::entry skip_entry;
        skip_entry.op1 = new_f_label;
        skip_entry.opcode = icode::CREATE_LABEL;
        (*current_func_desc).icode_table.push_back(skip_entry);
    }

    void ir_generator::print(const node::node& root)
    {
        for (size_t i = 0; i < root.children.size(); i++)
        {
            node::node child = root.children[i];

            icode::entry print_entry;

            /* If string literal, create temp string an print it */
            if (child.type == node::STR_LITERAL)
            {
                /* Get str len and str size */
                int str_len = child.tok.str.length();
                int str_size = str_len * icode::dtype_size[target.default_int];

                /* Generate temp string operand */
                icode::var_info print_var =
                  icode::var_from_dtype(target.default_int, target);
                print_var.dimensions.push_back(str_len);
                print_var.size = str_size;

                print_entry.op1 = gen_str_dat(child.tok, print_var).first;

                print_entry.opcode = icode::PRINT_STR;
            }
            /* Else expression */
            else
            {
                op_var_pair print_var = expression(child);

                /* Cannot peint struct or arrays */
                if (print_var.second.dtype == icode::STRUCT ||
                    print_var.second.dimensions.size() > 1)
                {
                    log::error_tok(module.name,
                                   "Cannot print STRUCT or multi-dimensional ARRAY",
                                   file,
                                   child.tok);
                    throw log::compile_error();
                }

                print_entry.op1 = print_var.first;

                if (print_var.second.dimensions.size() != 0)
                    print_entry.opcode = icode::PRINT_STR;
                else
                    print_entry.opcode = icode::PRINT;
            }

            (*current_func_desc).icode_table.push_back(print_entry);

            /* Add space after printing (except for the last print) */
            if (i != root.children.size() - 1)
            {
                icode::entry space_entry;
                space_entry.opcode = icode::SPACE;
                (*current_func_desc).icode_table.push_back(space_entry);
            }

            /* If println, the last print should add new line */
            if (i == root.children.size() - 1 && root.type == node::PRINTLN)
            {
                icode::entry newln_entry;
                newln_entry.opcode = icode::NEWLN;
                (*current_func_desc).icode_table.push_back(newln_entry);
            }
        }
    }

    void ir_generator::input(const node::node& root)
    {
        op_var_pair input_var = expression(root.children[0]);

        /* Creare icode entry */
        icode::entry input_entry;
        input_entry.op1 = input_var.first;

        /* Check if the input var is writable */
        if (!(input_var.first.optype == icode::VAR ||
              input_var.first.optype == icode::GBL_VAR ||
              input_var.first.optype == icode::TEMP_PTR ||
              input_var.first.optype == icode::PTR))
        {
            log::error_tok(
              module.name, "Invalid term for INPUT", file, root.children[0].tok);
            throw log::compile_error();
        }
        else if (input_var.second.dtype == icode::STRUCT)
        {
            log::error_tok(
              module.name, "Cannot INPUT STRUCT", file, root.children[0].tok);
            throw log::compile_error();
        }
        else if (input_var.second.dimensions.size() > 1)
        {
            log::error_tok(
              module.name, "Cannot INPUT more than 1D ARRAY", file, root.children[0].tok);
            throw log::compile_error();
        }
        else if (input_var.second.dimensions.size() == 1 &&
                 !icode::is_int(input_var.first.dtype))
        {
            log::error_tok(module.name,
                           "String input requires 1D INT ARRAY",
                           file,
                           root.children[0].tok);
            throw log::compile_error();
        }

        /* INPUT or INPUT_STR */
        if (input_var.second.dimensions.size() == 0)
            input_entry.opcode = icode::INPUT;
        else
        {
            input_entry.opcode = icode::INPUT_STR;
            input_entry.op2 = icode::gen_literal_opr(
              icode::INT, (int)input_var.second.dimensions[0], id());
        }

        (*current_func_desc).icode_table.push_back(input_entry);
    }

    void ir_generator::block(const node::node& root,
                             bool loop,
                             const icode::operand& start_label,
                             const icode::operand& break_label,
                             const icode::operand& cont_label)
    {
        /* Setup scope */
        enter_scope();

        for (node::node stmt : root.children)
        {
            switch (stmt.type)
            {
                case node::VAR:
                case node::CONST:
                    var(stmt);
                    break;
                case node::ASSIGNMENT:
                    assignment(stmt);
                    break;
                case node::ASSIGNMENT_STR:
                {
                    /* The variable to write to */
                    op_var_pair var = var_access(stmt.children[0]);

                    assign_str_literal_tovar(var, stmt.children[1]);

                    break;
                }
                case node::ASSIGNMENT_INITLIST:
                {
                    /* The variable to write to */
                    op_var_pair var = var_access(stmt.children[0]);

                    assign_init_list_tovar(var, stmt.children[1]);

                    break;
                }
                case node::FUNCCALL:
                    funccall(stmt);
                    break;
                case node::TERM:
                    funccall(stmt.children[0]);
                    break;
                case node::MODULE:
                {
                    /* Check if the module exists */
                    if (!(*current_ext_module).use_exists(stmt.tok.str))
                    {
                        log::error_tok(
                          module.name, "Module does not exist", file, stmt.tok);
                        throw log::compile_error();
                    }

                    /* Switch to external module */
                    icode::module_desc* temp = current_ext_module;
                    current_ext_module = &ext_modules_map[stmt.tok.str];

                    op_var_pair ret_val = funccall(stmt.children[0]);

                    /* Switch back to self */
                    current_ext_module = temp;

                    break;
                }
                case node::IF:
                    ifstmt(stmt, loop, start_label, break_label, cont_label);
                    break;
                case node::WHILE:
                    whileloop(stmt);
                    break;
                case node::FOR:
                    forloop(stmt);
                    break;
                case node::BREAK:
                {
                    if (!loop)
                    {
                        log::error_tok(module.name, "BREAK outside loop", file, stmt.tok);
                        throw log::compile_error();
                    }

                    /* Go to end */
                    icode::entry goto_entry;
                    goto_entry.op1 = break_label;
                    goto_entry.opcode = icode::GOTO;
                    (*current_func_desc).icode_table.push_back(goto_entry);

                    break;
                }
                case node::CONTINUE:
                {
                    if (!loop)
                    {
                        log::error_tok(
                          module.name, "CONTINUE outside loop", file, stmt.tok);
                        throw log::compile_error();
                    }

                    /* Go to end */
                    icode::entry goto_entry;
                    goto_entry.op1 = cont_label;
                    goto_entry.opcode = icode::GOTO;
                    (*current_func_desc).icode_table.push_back(goto_entry);

                    break;
                }
                case node::RETURN:
                {
                    /* Get return value */
                    if (stmt.children.size() != 0)
                    {
                        op_var_pair ret_val = expression(stmt.children[0]);

                        /* Type check */
                        if (!icode::type_eq((*current_func_desc).func_info,
                                            ret_val.second))
                        {
                            log::type_error(module.name,
                                            file,
                                            stmt.children[0].tok,
                                            (*current_func_desc).func_info,
                                            ret_val.second);
                            throw log::compile_error();
                        }

                        /* Assign return value to return pointer */
                        icode::operand ret_ptr = icode::gen_ret_ptr_opr(id());

                        if (ret_val.second.dtype == icode::STRUCT)
                            copy_struct(ret_ptr, ret_val);
                        else
                        {
                            icode::entry ret_eq_entry;
                            ret_eq_entry.opcode = icode::EQUAL;
                            ret_eq_entry.op1 = ret_ptr;
                            ret_eq_entry.op2 = ret_val.first;
                            (*current_func_desc).icode_table.push_back(ret_eq_entry);
                        }
                    }
                    else if ((*current_func_desc).func_info.dtype != icode::VOID)
                    {
                        log::error_tok(
                          module.name, "Ret type is not VOID", file, stmt.tok);
                        throw log::compile_error();
                    }

                    /* Add return to icode */
                    icode::entry ret_entry;
                    ret_entry.opcode = icode::RET;
                    (*current_func_desc).icode_table.push_back(ret_entry);

                    break;
                }
                case node::PRINT:
                case node::PRINTLN:
                    print(stmt);
                    break;
                case node::INPUT:
                    input(stmt);
                    break;
                case node::EXIT:
                {
                    icode::entry ret_entry;
                    ret_entry.opcode = icode::EXIT;
                    (*current_func_desc).icode_table.push_back(ret_entry);
                    break;
                }
                default:
                    log::internal_error_tok(module.name, file, stmt.tok);
                    throw log::internal_bug_error();
            }
        }

        exit_scope();
    }

    void ir_generator::initgen(const node::node& root)
    {
        /* Get uses */
        for (node::node child : root.children)
        {
            if (child.type == node::USE)
                use(child);
            else
                break;
        }
    }

    void ir_generator::program(const node::node& root)
    {
        /* Setup scope */
        clear_scope();

        /* Build symbol table */
        for (node::node child : root.children)
        {
            switch (child.type)
            {
                case node::USE:
                    break;
                case node::FROM:
                    from(child);
                    break;
                case node::STRUCT:
                    structure(child);
                    break;
                case node::FUNCTION:
                    fn(child);
                    break;
                case node::ENUM:
                    enumeration(child);
                    break;
                case node::DEF:
                    def(child);
                    break;
                case node::VAR:
                    global_var(child);
                    break;
                default:
                    log::internal_error_tok(module.name, file, child.tok);
                    throw log::internal_bug_error();
            }
        }

        /* Process function blocks */
        for (node::node child : root.children)
        {
            if (child.type == node::FUNCTION)
            {
                /* Get function name */
                std::string func_name = child.children[0].tok.str;

                /* Switch symbol and icode table */
                current_func_desc = &module.functions[func_name];

                /* Clear scope */
                clear_scope();

                /* Process block */
                block(child.children.back(),
                      false,
                      icode::gen_label_opr("", 0),
                      icode::gen_label_opr("", 0),
                      icode::gen_label_opr("", 0));

                /* Generate ret instruction for function */
                icode::entry ret_entry;

                if (func_name == "main")
                    ret_entry.opcode = icode::EXIT;
                else
                    ret_entry.opcode = icode::RET;

                (*current_func_desc).icode_table.push_back(ret_entry);
            }
        }
    }
} // namespace irgen
