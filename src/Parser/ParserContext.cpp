#include "Module.hpp"

#include "ParserContext.hpp"

/*
    Hand-written recursive descent parser for ShnooTalk.
    refer grammer.txt for grammer specification of the language.
*/

namespace parser
{
    ParserContext::ParserContext(lexer::Lexer& lexer, Console& console)
        : lex(lexer)
        , console(console)
        , ast(node::PROGRAM)
    {
        currentNode = &ast;

        consume();
    }

    void ParserContext::consume()
    {
        /* Get next token from lexer's token queue */
        lex.getToken(symbol);
    }

    bool ParserContext::peek(token::TokenType type)
    {
        /* Peek into token queue without popping it */
        return type == lex.peekToken().getType();
    }

    bool ParserContext::dpeek(token::TokenType type)
    {
        /* Peek into token queue without popping it */
        return type == lex.doublePeekToken().getType();
    }

    bool ParserContext::accept(token::TokenType type)
    {
        /* Check if symbol is equal to type passed as argument */
        return type == symbol.getType();
    }

    int ParserContext::getOperatorPrecedence()
    {
        return symbol.getPrecedence();
    }

    void ParserContext::expect(token::TokenType type)
    {
        /* If symbol is not equal to type passed as argument,
            throw exception*/
        if (!accept(type))
            console.parseError(type, symbol);
    }

    void ParserContext::expect(token::TokenType* types, int ntypes)
    {
        /* If symbol is not equal to atleast one of the types
            passed as argument, throw exception */
        bool accepted = false;

        for (int i = 0; i < ntypes; i++)
            if (accept(types[i]))
                accepted = true;

        if (!accepted)
            console.parserErrorMultiple(types, ntypes, symbol);
    }

    void ParserContext::addNode(node::NodeType nodeType)
    {
        /* Add child node to current node, if traverse iw true
            make current node the child node */
        /* Also fetch next symbol, if nexttoken is true */
        currentNode->children.push_back(Node(nodeType, symbol));

        consume();
    }

    void ParserContext::addNodeMakeCurrent(node::NodeType nodeType)
    {
        currentNode->children.push_back(Node(nodeType, symbol));

        currentNode = &currentNode->children.back();

        consume();
    }

    void ParserContext::addNodeMakeCurrentNoConsume(node::NodeType nodeType)
    {
        currentNode->children.push_back(Node(nodeType, symbol));

        currentNode = &currentNode->children.back();
    }

    void ParserContext::insertNode(node::NodeType type)
    {
        /* Adopts last child of current node
            and becomes the last child of current node */

        Node new_node(type, currentNode->children.back().tok);

        new_node.children.push_back(currentNode->children.back());

        currentNode->children.pop_back();

        currentNode->children.push_back(new_node);

        currentNode = &currentNode->children.back();
    }

    void ParserContext::insertNodeBeginning(node::NodeType type)
    {
        /* Adopts ALL children of current node
            and becomes the last child of current node */

        Node new_node(type, currentNode->children.back().tok);

        std::swap(currentNode->children, new_node.children);

        currentNode->children.push_back(new_node);
    }

    void ParserContext::duplicateNode()
    {
        /* Takes all children of current node, moves
            it to a new duplicate of the current node. The duplicate
            node becomes the child of the current node */

        Node new_node(currentNode->type, symbol);

        new_node.children = currentNode->children;

        currentNode->children.clear();

        currentNode->children.push_back(new_node);

        consume();
    }

    void ParserContext::pushNode()
    {
        /* Push current node onto stack */
        nodeStack.push_back(currentNode);
    }

    void ParserContext::popNode()
    {
        /* Pop node from stack, assign to current node */
        currentNode = nodeStack.back();
        nodeStack.pop_back();
    }

    Node ParserContext::getAST()
    {
        return ast;
    }

} // namespace parser
