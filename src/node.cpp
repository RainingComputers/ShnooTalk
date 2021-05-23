#include "node.hpp"

namespace node
{
    node::node(NodeType ntype, token::Token token)
    {
        type = ntype;
        tok = token;
    }

    node::node(NodeType ntype)
    {
        type = ntype;
        tok = token::Token();
    }

    bool node::isNthChild(NodeType ntype, size_t N) const
    {
        if (N < children.size())
            if (children[N].type == ntype)
                return true;

        return false;
    }

    bool node::isNthChildFromLast(NodeType ntype, size_t N) const
    {
        size_t index = children.size() - N;

        if (index > 0)
            if (children[index].type == ntype)
                return true;

        return false;
    }

    token::Token node::getNthChildToken(size_t N) const
    {
        return children[N].tok;
    }

    token::Token node::getNthChildTokenFromLast(size_t N) const
    {
        return children[children.size() - N].tok;
    }
} // namespace node