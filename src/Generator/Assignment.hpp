#ifndef GENERATOR_ASSIGNMENT
#define GENERATOR_ASSIGNMENT

#include "GeneratorContext.hpp"

void conditionalOrExpressionAssignmentFromTree(generator::GeneratorContext& ctx,
                                               const Node& root,
                                               const Unit& LHS,
                                               const Node& expressionTree);
void assignment(generator::GeneratorContext& ctx, const Node& root);

#endif