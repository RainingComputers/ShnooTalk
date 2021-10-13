#ifndef CONSOLE_CONSOLE
#define CONSOLE_CONSOLE

#include <map>
#include <vector>
#include <fstream>

#include "../Builder/Unit.hpp"
#include "../PrettyPrint/Errors.hpp"

struct CompileError
{
};

struct InternalBugError
{
};

class Console
{
    std::string fileName;
    std::ifstream* file;

    std::map<std::string, std::ifstream> streamsMap;
    std::vector<std::string> fileNameStack;
    std::vector<std::ifstream*> fileStack;

public:
    [[noreturn]] void compileErrorOnToken(const std::string& message, const Token& tok);

    [[noreturn]] void typeError(const Token& tok, const Unit& expected, const Unit& found);

    [[noreturn]] void internalBugErrorOnToken(const Token& tok);

    [[noreturn]] void internalBugError();

    [[noreturn]] void internalBugErrorMessage(const std::string& message);

    [[noreturn]] void parseError(token::TokenType& expected, Token& found);

    [[noreturn]] void parserErrorMultiple(const token::TokenType* expected, int ntoks, const Token& found);

    [[noreturn]] void lexerError(const std::string& errorMessage, const std::string& line, int lineno, int col);

    void check(bool flag);

    std::ifstream* getStream();

    std::string getFileName();

    void pushModule(const std::string& fileName);

    void popModule();
};

#endif