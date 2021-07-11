#ifndef GENERATOR_UNIT_FROM_IDENTIFIER
#define GENERATOR_UNIT_FROM_IDENTIFIER

#include "GeneratorContext.hpp"

Unit getUnitFromNode(generator::GeneratorContext& ctx, const Node& root);

Unit getUnitFromIdentifier(generator::GeneratorContext& ctx, const Node& root);

#endif