#ifndef GENERATOR_STRUCTURE_HPP
#define GENERATOR_STRUCTURE_HPP

#include "GeneratorContext.hpp"

void createStructFromNode(generator::GeneratorContext& ctx, const Node& root);
void registerIncompleteTypeFromNode(generator::GeneratorContext& ctx, const Node& root);

#endif