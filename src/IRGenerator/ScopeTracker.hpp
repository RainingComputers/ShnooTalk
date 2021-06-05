#ifndef SCOPE_TRACKER
#define SCOPE_TRACKER

#include <map>
#include <vector>

#include "../Token/Token.hpp"

class ScopeTracker
{
    unsigned int scopeCounter;
    std::vector<unsigned int> scopeStack;
    std::map<std::string, int> symbolScopeMap;

    unsigned int getCurrentScope();

  public:
    ScopeTracker();
    void createScope();
    void exitScope();
    void resetScope();
    void putInCurrentScope(const Token& symbol);
    bool isInCurrentScope(const Token& symbol);
};

#endif