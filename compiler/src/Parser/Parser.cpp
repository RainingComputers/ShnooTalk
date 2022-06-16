#include "Module.hpp"
#include "ParserContext.hpp"

#include "Parser.hpp"

Node parser::generateAST(lexer::Lexer& lexer, Console& console)
{
    parser::ParserContext parserContext(lexer, console);
    programModule(parserContext);
    return parserContext.getAST();
}