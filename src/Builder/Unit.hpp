#ifndef BUILDER_UNIT
#define BUILDER_UNIT

#include <utility>

#include "../IntermediateRepresentation/Operand.hpp"
#include "../IntermediateRepresentation/TypeDescription.hpp"

struct Unit
{
    icode::TypeDescription type;
    icode::Operand op;
    std::vector<Unit> aggs;

    Unit();
    Unit(const icode::TypeDescription& type, const icode::Operand& operand);
    Unit(const icode::TypeDescription& type, const std::vector<Unit>& aggs);
    std::vector<Unit> flatten() const;
};

#endif