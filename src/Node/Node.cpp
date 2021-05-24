#include "Node.hpp"

namespace node
{
    Node::Node(NodeType nodeType, token::Token token)
    {
        type = nodeType;
        tok = token;
    }

    Node::Node(NodeType nodeType)
    {
        type = nodeType;
        tok = token::Token();
    }

    bool Node::isNodeType(NodeType nodeType) const
    {
        return type == nodeType;
    }

    bool Node::isNthChild(NodeType nodeType, size_t N) const
    {
        if (N < children.size())
            if (children[N].type == nodeType)
                return true;

        return false;
    }

    bool Node::isNthChildFromLast(NodeType nodeType, size_t N) const
    {
        size_t index = children.size() - N;

        if (index > 0)
            if (children[index].type == nodeType)
                return true;

        return false;
    }

    token::Token Node::getNthChildToken(size_t N) const
    {
        return children[N].tok;
    }

    token::Token Node::getNthChildTokenFromLast(size_t N) const
    {
        return children[children.size() - N].tok;
    }
} // namespace node