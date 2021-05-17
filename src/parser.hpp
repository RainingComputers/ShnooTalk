#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <vector>

#include "lexer.hpp"
#include "log.hpp"
#include "node.hpp"
#include "token.hpp"

namespace parser
{
    class rd_parser
    {
        lexer::lexical_analyser& lex;
        std::ifstream& file;
        std::string file_name;

        node::node* current_node;
        std::vector<node::node*> node_stack;

        token::Token symbol;

        void next();
        bool peek(token::tokenType type);
        bool dpeek(token::tokenType type);
        bool accept(token::tokenType type);
        int get_operator_prec();
        void expect(token::tokenType type);
        void expect(token::tokenType* types, int ntypes);

        void push_node();
        void pop_node();
        void add_node(node::node_type node_type, bool traverse = false, bool nexttoken = true);
        void insert_node(node::node_type node_type);
        void insert_node_beg(node::node_type node_type);
        void duplicate_node();

        void program();
        void use();
        void from();
        void def();
        void termvar(bool expr_subscript = false);
        void exprvar();
        void varname();
        void initializerlist();
        void varassign();
        void varlist();
        void constlist();
        void struct_varlist();
        void paramlist();
        void term_literal();
        void structdef();
        void enumdef();

        void struct_funccall();
        void term();
        void expression(int min_precedence = 1);

        void assignment();
        void print();
        void statement();
        void block();
        void function();
        void passparam();
        void funccall();

      public:
        node::node ast;
        rd_parser(lexer::lexical_analyser& lexer, const std::string& name, std::ifstream& ifile);
    };
}

#endif