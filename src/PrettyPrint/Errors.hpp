#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <string>

#include "../IntermediateRepresentation/TypeDescription.hpp"
#include "../Token/Token.hpp"

namespace mikpp
{
    void println(const std::string& msg);

    void errorOnLine(const std::string& moduleName,
                     const std::string& errorMessage,
                     const std::string& line,
                     int lineno,
                     int col);

    void errorOnModuleName(const std::string& moduleName, const std::string& errorMessage);

    void errorOnToken(const std::string& moduleName,
                      const std::string& errorMessage,
                      std::ifstream& file,
                      const Token& token);

    void parserError(const std::string& moduleName,
                     token::TokenType expected,
                     Token& found,
                     std::ifstream& file);

    void parserErrorMultiple(const std::string& moduleName,
                             const token::TokenType* expected,
                             int ntoks,
                             const Token& found,
                             std::ifstream& file);

    void typeError(const std::string& moduleName,
                   std::ifstream& file,
                   const Token& tok,
                   icode::TypeDescription& expected,
                   icode::TypeDescription& found);

    void internalCompilerErrorToken(const std::string& moduleName, std::ifstream& file, const Token& tok);

    void internalCompileError(const std::string& moduleName);
}

#endif