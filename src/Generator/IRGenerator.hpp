#ifndef GENERATORIR_IR_GENERATOR
#define GENERATORIR_IR_GENERATOR

#include "GeneratorContext.hpp"

namespace generator
{
    void getUses(generator::GeneratorContext& ctx, const Node& root);
    void generateModule(generator::GeneratorContext& ctx, const Node& root);
}

#endif