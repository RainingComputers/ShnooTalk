#ifndef GENERATOR_LOCAL
#define GENERATOR_LOCAL

#include "GeneratorContext.hpp"

void local(generator::GeneratorContext& ctx, const Node& root);
void walrusLocal(generator::GeneratorContext& ctx, const Node& root);
void destructureLocal(generator::GeneratorContext& ctx, const Node& root);

#endif
