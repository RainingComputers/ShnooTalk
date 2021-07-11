#ifndef PARSER_PARSER
#define PARSER_PARSER

#include "../Console/Console.hpp"
#include "../Lexer/Lexer.hpp"
#include "../Node/Node.hpp"


namespace parser
{
    Node generateAST(lexer::Lexer& lexer, Console& console);
}

#endif