#ifndef EXPRESSION_HPP
#define EXPRESSUON_HPP

#include "../irgen_old.hpp"

Unit term(irgen::ir_generator& ctx, const Node& root);
Unit expression(irgen::ir_generator& ctx, const Node& root);

#endif