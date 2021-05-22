#include "node.hpp"

namespace node
{
    node::node(node_type ntype, token::Token token)
    {
        type = ntype;
        tok = token;
    }

    node::node(node_type ntype)
    {
        type = ntype;
        tok = token::Token();
    }

    bool node::isNthChild(node_type ntype, size_t N) const
    {
        if(N < children.size())
            if(children[N].type == ntype)
                return true;

        return false;
    }

    bool node::isNthChildFromLast(node_type ntype, size_t N) const
    {
        size_t index = children.size() -  N;

        if(index > 0)
            if(children[index].type == ntype)
                return true;

        return false;
    }
} // namespace node