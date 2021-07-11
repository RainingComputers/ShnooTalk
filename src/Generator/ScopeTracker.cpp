#include <algorithm>

#include "ScopeTracker.hpp"

ScopeTracker::ScopeTracker()
{
    scopeCounter = 0;
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
    const int symbolScope = symbolScopeMap.at(symbol.toString());
    return std::find(scopeStack.begin(), scopeStack.end(), symbolScope) != scopeStack.end();
}

void ScopeTracker::putInCurrentScope(const Token& symbol)
{
    symbolScopeMap[symbol.toString()] = getCurrentScope();
}