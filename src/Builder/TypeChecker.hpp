#ifndef BUILDER_TYPE_CHECKER
#define BUILDER_TYPE_CHECKER

#include "../Console/Console.hpp"
#include "FunctionBuilder.hpp"

class TypeChecker
{
    FunctionBuilder& functionBuilder;

    void autoCast(Unit& LHS, Unit& RHS);

  public:
    TypeChecker(FunctionBuilder& functionBuilder);

    bool check(Unit& LHS, Unit& RHS);
};

#endif