#ifndef NODE_HPP
#define NODE_HPP

#include "token.hpp"
#include <vector>

namespace node
{
    enum node_type
    {
        PROGRAM,
        USE,
        FROM,
        BLOCK,
        MODULE,
        FUNCTION,
        STRUCT,
        ENUM,
        DEF,
        VAR,
        CONST,
        IF,
        ELSEIF,
        ELSE,
        WHILE,
        FOR,
        RET_TYPE,
        CAST,
        BREAK,
        CONTINUE,
        ASSIGNMENT,
        ASSIGNMENT_STR,
        ASSIGNMENT_INITLIST,
        RETURN,
        FUNCCALL,
        STRUCT_FUNCCALL,
        PARAM,
        MUT_PARAM,
        EXPRESSION,
        STR_LITERAL,
        LITERAL,
        IDENTIFIER,
        ASSIGN_OPERATOR,
        OPERATOR,
        UNARY_OPR,
        SUBSCRIPT,
        STRUCT_VAR,
        INITLIST,
        TERM,
        SIZEOF,
        TYPEOF,
        PRINT,
        PRINTLN,
        INPUT,
        EXIT
    };

    struct node
    {
        node_type type;
        token::Token tok;
        std::vector<node> children;

        node(node_type ntype, token::Token tok);
        node(node_type ntype);
    };
}

#endif