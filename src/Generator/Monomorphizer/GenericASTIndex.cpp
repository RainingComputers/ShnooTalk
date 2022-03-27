#include "../../Utils/ItemInList.hpp"

#include "GenericASTIndex.hpp"

bool GenericASTIndex::isGenericStructOrFunction(const Token& name) const
{
    return itemInList<std::string>(name.toString(), genericStructs) ||
           itemInList<std::string>(name.toString(), genericFunctions);
}
