#include "Node.hpp"

using namespace node;

Node::Node(NodeType nodeType, Token token)
{
    type = nodeType;
    tok = token;
}

Node::Node()
{
    type = PROGRAM;
    tok = Token();
}

bool Node::isNthChild(NodeType nodeType, size_t N) const
{
    if (N < children.size())
    {
        if (children[N].type == nodeType)
            return true;
    }

    return false;
}

bool Node::isNthChildFromLast(NodeType nodeType, size_t N) const
{
    size_t index = children.size() - N;

    if (index >= 0 && index < children.size())
        if (children[index].type == nodeType)
            return true;

    return false;
}

Token Node::getNthChildToken(size_t N) const
{
    return children[N].tok;
}

Token Node::getNthChildTokenFromLast(size_t N) const
{
    return children[children.size() - N].tok;
}

bool Node::isConditionalExpression() const
{
    if (tok.getType() == token::CONDN_NOT)
        return true;

    if (type == TERM)
        return children[0].isConditionalExpression();

    if (type == EXPRESSION)
    {
        if (children.size() == 1)
            return children[0].isConditionalExpression();

        return children[1].tok.isConditionalOperator();
    }

    return false;
}

bool Node::isParamNode() const
{
    return type == node::PARAM || type == node::MUT_PARAM;
}

bool Node::isGenericModule() const
{
    return type == node::PROGRAM && isNthChild(node::GENERIC, 0);
}

bool Node::isGenericTypeParamPresent() const
{
    return isNthChildFromLast(node::GENERIC_TYPE_PARAM, 1) || isNthChildFromLast(node::GENERIC_TYPE_PARAM, 2) ||
           isNthChildFromLast(node::GENERIC_TYPE_PARAM, 3);
}