#include "Console.hpp"

Console::Console(const std::string& fileName, std::ifstream* file)
    : fileName(fileName)
{
    this->file = file;
}

void Console::compileErrorOnToken(const std::string& message, const Token& tok)
{
    pp::errorOnToken(fileName, message, *file, tok);
    throw CompileError();
}

void Console::typeError(const Token& tok, const Unit& expected, const Unit& found)
{
    pp::typeError(fileName, *file, tok, expected.type(), found.type());
    throw CompileError();
}

void Console::internalBugErrorOnToken(const Token& tok)
{
    pp::errorOnToken(fileName, "Internal compiler error, REPORT THIS BUG", *file, tok);
    throw InternalBugError();
}

void Console::internalBugError()
{
    pp::errorOnModuleName(fileName, "Internal compiler error, REPORT THIS BUG");
    throw InternalBugError();
}

void Console::internalBugErrorMessage(const std::string& message)
{
    pp::println(message);
    throw InternalBugError();
}

void Console::parseError(token::TokenType& expected, Token& found)
{
    pp::parserError(fileName, expected, found, *file);
    throw CompileError();
}

void Console::parserErrorMultiple(const token::TokenType* expected, int ntoks, const Token& found)
{
    pp::parserErrorMultiple(fileName, expected, ntoks, found, *file);
    throw CompileError();
}

void Console::lexerError(const std::string& errorMessage, const std::string& line, int lineno, int col)
{
    pp::errorOnLine(fileName, errorMessage, line, lineno, col);
    throw CompileError();
}

void Console::check(bool flag)
{
    if (!flag)
        internalBugError();
}

std::ifstream* Console::getStream()
{
    return file;
}