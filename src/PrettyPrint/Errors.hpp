#ifndef PP_ERRORS
#define PP_ERRORS

#include <string>

#include "../IntermediateRepresentation/TypeDescription.hpp"
#include "../Token/Token.hpp"

namespace pp
{
    void println(const std::string& msg);

    void printModuleLocation(const std::string& moduleName, const Token& tok);

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

    void parserError(const std::string& moduleName, token::TokenType expected, Token& found, std::ifstream& file);

    void parserErrorMultiple(const std::string& moduleName,
                             const token::TokenType* expected,
                             int ntoks,
                             const Token& found,
                             std::ifstream& file);

    void typeError(const std::string& moduleName,
                   std::ifstream& file,
                   const Token& tok,
                   const icode::TypeDescription& expected,
                   const icode::TypeDescription& found);

    void internalCompilerErrorToken(const std::string& moduleName, std::ifstream& file, const Token& tok);

    void internalCompileError(const std::string& moduleName);
}

#endif