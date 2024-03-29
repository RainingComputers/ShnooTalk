#ifndef PARSER_FUNCTION
#define PARSER_FUNCTION

#include "ParserContext.hpp"

void formalParameterList(parser::ParserContext& ctx);
void functionDefinition(parser::ParserContext& ctx);
void externFunctionDefinition(parser::ParserContext& ctx);
void functionDefinitionExternC(parser::ParserContext& ctx);

#endif