#include <algorithm>

#include "../Utils/ItemInList.hpp"

#include "ScopeTracker.hpp"

ScopeTracker::ScopeTracker()
{
    resetScope();
}

unsigned int ScopeTracker::getCurrentScope()
{
    return scopeStack.back();
}

void ScopeTracker::createScope()
{
    scopeStack.push_back(++scopeCounter);
}

void ScopeTracker::exitScope()
{
    scopeStack.pop_back();
}

void ScopeTracker::resetScope()
{
    scopeCounter = 0;
    scopeStack.clear();
    scopeStack.push_back(0);
}

bool ScopeTracker::isInCurrentScope(const Token& symbol)
{
    const unsigned int symbolScope = symbolScopeMap.at(symbol.toString());
    return itemInList<unsigned int>(symbolScope, scopeStack);
}

void ScopeTracker::putInCurrentScope(const Token& symbol)
{
    symbolScopeMap[symbol.toString()] = getCurrentScope();
}