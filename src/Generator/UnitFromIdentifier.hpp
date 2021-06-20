#ifndef GENERATOR_UNIT_FROM_IDENTIFIER
#define GENERATOR_UNIT_FROM_IDENTIFIER

#include "../irgen_old.hpp"

Unit getUnitFromNode(irgen::ir_generator& ctx, const Node& root);

Unit getUnitFromIdentifier(irgen::ir_generator& ctx, const Node& root);

#endif