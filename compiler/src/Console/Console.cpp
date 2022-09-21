#include "../Utils/KeyExistsInMap.hpp"
#include "ModuleSearch.hpp"

#include "Console.hpp"

void Console::printModuleStackLine(int lineNo, int colNo)
{
    pp::println("");

    pp::printModuleLocation(fileNameStack.back(), lineNo, colNo);

    for (size_t i = fileNameStack.size() - 2; i <= 0; i -= 1)
        pp::println(fileNameStack[i]);
}

void Console::printModuleStack()
{
    pp::println("");

    for (size_t i = fileNameStack.size() - 1; i <= 0; i -= 1)
        pp::println(fileNameStack[i]);
}

void Console::compileErrorOnToken(const std::string& message, const Token& tok)
{
    pp::errorOnToken(tok.getFileName(), message, streamsMap[tok.getFileName()], tok);
    printModuleStackLine(tok.getLineNo(), tok.getColumn());
    throw CompileError();
}

void Console::typeError(const Token& tok, const Unit& expected, const Unit& found)
{
    pp::typeError(fileName, *file, tok, expected.type(), found.type());
    printModuleStackLine(tok.getLineNo(), tok.getColumn());
    throw CompileError();
}

void Console::operatorError(const Token& tok, const Unit& expected, const Unit& found)
{
    pp::operatorError(fileName, *file, tok, expected.type(), found.type());
    printModuleStackLine(tok.getLineNo(), tok.getColumn());
    throw CompileError();
}

void Console::internalBugErrorOnToken(const Token& tok)
{
    pp::errorOnToken(fileName, "Internal compiler error, REPORT THIS BUG", *file, tok);
    printModuleStackLine(tok.getLineNo(), tok.getColumn());
    throw InternalBugError();
}

void Console::internalBugError()
{
    pp::errorOnModuleName(fileName, "Internal compiler error, REPORT THIS BUG");
    printModuleStack();
    throw InternalBugError();
}

void Console::internalBugErrorMessage(const std::string& message)
{
    pp::println(message);
    printModuleStack();
    throw InternalBugError();
}

void Console::parseError(token::TokenType& expected, Token& found)
{
    pp::parserError(fileName, expected, found, *file);
    printModuleStackLine(found.getLineNo(), found.getColumn());
    throw CompileError();
}

void Console::parserErrorMultiple(const token::TokenType* expected, int ntoks, const Token& found)
{
    pp::parserErrorMultiple(fileName, expected, ntoks, found, *file);
    printModuleStackLine(found.getLineNo(), found.getColumn());
    throw CompileError();
}

void Console::lexerError(const std::string& errorMessage, const std::string& line, int lineno, int col)
{
    pp::errorOnLine(fileName, errorMessage, line, lineno, col);
    printModuleStackLine(lineno, col);
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

std::string Console::getFileName()
{
    return fileName;
}

void Console::pushRootModule(const std::string& path)
{
    if (!keyExistsInMap(streamsMap, path))
    {
        streamsMap[path].exceptions(std::ifstream::failbit | std::ifstream::badbit);
        streamsMap[path].open(path);
    }

    fileName = path;
    file = &streamsMap[path];

    fileNameStack.push_back(path);
    fileStack.push_back(&streamsMap[path]);
}

bool invalidModuleName(const std::string& path)
{
    for (const char c : path)
        if (std::string("[]@:~*").find(c) != std::string::npos)
            return true;

    return false;
}

void Console::pushModuleString(const std::string& path, const Token& errorToken)
{
    if (invalidModuleName(path))
        compileErrorOnToken("Invalid module name", errorToken);

    const std::string absolutePath = getAbsoluteModulePath(path);

    if (absolutePath == "")
        compileErrorOnToken("File does not exist", errorToken);

    pushRootModule(absolutePath);
}

void Console::pushModuleToken(const Token& pathToken)
{
    pushModuleString(pathToken.toUnescapedString(), pathToken);
}

void Console::popModule()
{
    fileNameStack.pop_back();
    fileStack.pop_back();

    fileName = fileNameStack.back();
    file = fileStack.back();
}
