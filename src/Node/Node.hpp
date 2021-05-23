#ifndef NODE_HPP
#define NODE_HPP

#include "../Token/Token.hpp"
#include <vector>

namespace node
{
    enum NodeType
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

    struct Node
    {
        NodeType type;
        token::Token tok;
        std::vector<Node> children;

        Node(NodeType nodeType, token::Token token);
        Node(NodeType nodeType);
        bool isNthChild(NodeType nodeType, size_t index) const;
        bool isNthChildFromLast(NodeType nodeType, size_t N) const;
        token::Token getNthChildToken(size_t N) const;
        token::Token getNthChildTokenFromLast(size_t N) const;
    };
}

#endif