#ifndef PARSER
#define PARSER

#include <fstream>
#include <vector>

#include "../Console/Console.hpp"
#include "../Lexer/Lexer.hpp"
#include "../Node/Node.hpp"
#include "../Token/Token.hpp"

namespace parser
{
    class ParserContext
    {

        lexer::Lexer& lex;
        Console& console;

        Node* currentNode;
        std::vector<Node*> nodeStack;

        Token symbol;

        Node ast;

      public:
        void next();
        bool peek(token::TokenType type);
        bool dpeek(token::TokenType type);
        bool accept(token::TokenType type);
        int getOperatorPrecedence();
        void expect(token::TokenType type);
        void expect(token::TokenType* types, int ntypes);

        void addNode(node::NodeType NodeType, bool traverse = false, bool nexttoken = true);
        void insertNode(node::NodeType NodeType);
        void insertNodeBeginning(node::NodeType NodeType);
        void duplicateNode();
        void pushNode();
        void popNode();

        ParserContext(lexer::Lexer& lexer, Console& console);
        Node getAST();
    };
}

#endif