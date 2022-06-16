#ifndef PP_AST_PRINTER
#define PP_AST_PRINTER

#include "../Node/Node.hpp"
#include "../Token/Token.hpp"

namespace pp
{
    void printNode(const Node& node, int indent = 0);
    void printJSONAST(const Node& ast);
}

#endif