#ifndef SCOPE_TRACKER
#define SCOPE_TRACKER

#include <vector>
#include <map>

#include "../Token/Token.hpp"

class ScopeTracker
{
    unsigned int scopeCounter;
    std::vector<unsigned int> scopeStack;
    std::map<std::string, int> symbolScopeMap;

  public:
    ScopeTracker();
    unsigned int getCurrentScope();
    void createScope();
    void exitScope();
    void globalScope();
    void putInGlobalScope(const token::Token& symbol);
    void putInCurrentScope(const token::Token& symbol);
    bool isInCurrentScope(const token::Token& symbol);
};

#endif