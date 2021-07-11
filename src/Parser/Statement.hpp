#ifndef PARSER_STATEMENT
#define PARSER_STATEMENT

#include "ParserContext.hpp"

void assignmentOrMethodCall(parser::ParserContext& ctx);
void moduleFunctionCall(parser::ParserContext& ctx);
void ifStatement(parser::ParserContext& ctx);
void whileLoop(parser::ParserContext& ctx);
void forLoop(parser::ParserContext& ctx);
void returnExpression(parser::ParserContext& ctx);
void statement(parser::ParserContext& ctx);
void block(parser::ParserContext& ctx);

#endif