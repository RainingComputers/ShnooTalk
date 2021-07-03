#ifndef BUILDER_TYPE_CHECKER
#define BUILDER_TYPE_CHECKER

#include "FunctionBuilder.hpp"
#include "../Console/Console.hpp"

class TypeChecker
{
    FunctionBuilder& functionBuilder;

    bool checkAggType(Unit& LHS, Unit& RHS);

    bool autoCast(Unit& LHS, Unit& RHS);

  public:
    TypeChecker(FunctionBuilder& functionBuilder);

    bool check(Unit& LHS, Unit& RHS);
};

#endif