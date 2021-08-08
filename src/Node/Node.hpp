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
        EXTERN_FUNCTION,
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
        CAST,
        BREAK,
        CONTINUE,
        ASSIGNMENT,
        RETURN,
        FUNCCALL,
        METHODCALL,
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
        STRUCT_FIELD,
        INITLIST,
        TERM,
        SIZEOF,
        TYPEOF,
        PRINT,
        PRINTLN,
        INPUT
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