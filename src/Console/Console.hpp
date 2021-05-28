#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <fstream>

#include "../PrettyPrint/PrettyPrint.hpp"

struct CompileError
{
};

struct InternalBugError
{
};

class Console
{
    const std::string fileName;
    std::ifstream* file;

  public:
    void compileErrorOnToken(const std::string& message, const token::Token& tok);

    void typeError(const token::Token& tok, icode::VariableDescription& expected, icode::VariableDescription& found);

    void internalBugErrorOnToken(const token::Token& tok);

    void internalBugError();

    void internalBugErrorMessage(const std::string& message);

    void* controlReachedEndError();

    void parseError(token::tokenType& expected, token::Token& found);

    void parserErrorMultiple(const token::tokenType* expected, int ntoks, const token::Token& found);

    void lexerError(const std::string& errorMessage, const std::string& line, int lineno, int col);

    std::ifstream* getStream();

    Console(const std::string& fileName, std::ifstream* file);
};

#endif