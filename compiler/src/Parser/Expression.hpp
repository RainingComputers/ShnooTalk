#ifndef PARSER_EXPRESSION
#define PARSER_EXPRESSION

#include "ParserContext.hpp"

void identifierWithQualidentAndSubscript(parser::ParserContext& ctx);
void moduleQualident(parser::ParserContext& ctx);
void typeDefinitionNoPointer(parser::ParserContext& ctx);
void typeDefinition(parser::ParserContext& ctx);
void actualParameterList(parser::ParserContext& ctx);
void functionCall(parser::ParserContext& ctx);
void methodCall(parser::ParserContext& ctx);
void sizeofBuiltIn(parser::ParserContext& ctx);
void initializerList(parser::ParserContext& ctx);
void term(parser::ParserContext& ctx);
void baseExpression(parser::ParserContext& ctx, int minPrecedence);
void expression(parser::ParserContext& ctx);

#endif