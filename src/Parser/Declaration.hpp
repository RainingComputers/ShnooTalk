#ifndef PARSER_DECLARATION
#define PARSER_DECLARATION

#include "ParserContext.hpp"

void identifierDecleration(parser::ParserContext& ctx);
void identifierDeclarationAndInit(parser::ParserContext& ctx);
void identifierDeclareList(parser::ParserContext& ctx, bool initAllowed);

#endif