#include "Console.hpp"

void Console::compileErrorOnToken(const std::string& message, const token::Token& tok)
{
    mikpp::errorOnToken(fileName, message, file, tok);
    throw CompileError();
}

void Console::typeError(const token::Token& tok,
                        icode::VariableDescription& expected,
                        icode::VariableDescription& found)
{
    mikpp::typeError(fileName, file, tok, expected, found);
    throw CompileError();
}

void Console::internalBugErrorOnToken(const token::Token& tok)
{
    mikpp::errorOnToken(fileName, "Internal compiler error, REPORT THIS BUG", file, tok);
    throw InternalBugError();
}

void Console::internalBugError()
{
    mikpp::error(fileName, "Internal compiler error, REPORT THIS BUG");
    throw InternalBugError();
}

void Console::internalBugErrorMessage(const std::string& message)
{
    mikpp::println(message);
    throw InternalBugError();
}

void Console::parseError(token::tokenType& expected, token::Token& found)
{
    mikpp::parserError(fileName, expected, found, file);
    throw CompileError();
}

void Console::parserErrorMultiple(const token::tokenType* expected, int ntoks, const token::Token& found)
{
    mikpp::parserErrorMultiple(fileName, expected, ntoks, found, file);
    throw CompileError();
}

void Console::lexerError(const std::string& errorMessage, const std::string& line, int lineno, int col)
{
    mikpp::errorOnLine(fileName, errorMessage, line, lineno, col);
    throw CompileError();
}

Console::Console(const std::string& fileName, std::ifstream& file)
  : fileName(fileName)
  , file(file)
{
}