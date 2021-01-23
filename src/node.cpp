#include "node.hpp"

namespace node
{
    node::node(node_type ntype, token::token token)
    {
        type = ntype;
        tok = token;
    }

    node::node(node_type ntype)
    {
        type = ntype;
        tok = token::token();
    }
} // namespace node