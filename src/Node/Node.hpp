#ifndef NODE_NODE
#define NODE_NODE

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
}

struct Node
{
    node::NodeType type;
    Token tok;
    std::vector<Node> children;

    Node(node::NodeType nodeType, Token token);
    Node(node::NodeType nodeType);
    bool isNthChild(node::NodeType nodeType, size_t index) const;
    bool isNthChildFromLast(node::NodeType nodeType, size_t N) const;
    Token getNthChildToken(size_t N) const;
    Token getNthChildTokenFromLast(size_t N) const;
};

#endif