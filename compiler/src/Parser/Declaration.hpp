#ifndef PARSER_DECLARATION
#define PARSER_DECLARATION

#include "ParserContext.hpp"

void identifierDecleration(parser::ParserContext& ctx);
void identifierDeclarationOptionalInit(parser::ParserContext& ctx);
void identifierDeclareListOptionalInit(parser::ParserContext& ctx, bool initAllowed);
void identifierDeclarationRequiredInit(parser::ParserContext& ctx);
void identifierDeclareListRequiredInit(parser::ParserContext& ctx);

void walrusDeclaration(parser::ParserContext& ctx);
void destructureList(parser::ParserContext& ctx);
void destructureDeclaration(parser::ParserContext& ctx);

#endif