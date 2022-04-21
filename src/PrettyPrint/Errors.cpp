#include <fstream>
#include <iostream>

#include "../Generator/Generic.hpp"
#include "../Utils/StringReplace.hpp"
#include "Strings.hpp"

#include "Errors.hpp"

namespace pp
{
    void println(const std::string& msg)
    {
        std::cout << msg << std::endl;
    }

    void printModuleLocation(const std::string& moduleName, int lineNo, int colNo)
    {
        std::cout << moduleName << ":" << lineNo << ":" << colNo << std::endl;
    }

    void errorOnModuleName(const std::string& moduleName, const std::string& errorMessage)
    {
        std::cout << "MODULE " + moduleName << std::endl;
        std::cout << errorMessage << std::endl;
    }

    void errorOnLine(const std::string& moduleName,
                     const std::string& errorMessage,
                     const std::string& line,
                     int lineno,
                     int col)
    {
        /* Accepts line as string and column, prints line and '^' symbol at col
            along with error message */
        std::cout << "MODULE " + moduleName << std::endl;
        std::cout << "ERROR in ";
        std::cout << "Line " << lineno;
        std::cout << " Col " << col + 1 << std::endl;
        std::cout << std::endl;
        std::cout << line << std::endl;
        std::cout << std::string(col, ' ') << "^" << std::endl;
        std::cout << errorMessage << std::endl;
    }

    void errorOnToken(const std::string& moduleName,
                      const std::string& errorMessage,
                      std::ifstream& file,
                      const Token& tok)
    {
        /* Prints the exact line from file using tok
            and error message */

        /* Goto beginning */
        file.clear();
        file.seekg(0, std::ios::beg);

        /* Get line */
        std::string line;
        for (int i = 0; i < tok.getLineNo(); i++)
            getline(file, line);

        errorOnLine(moduleName, errorMessage, line, tok.getLineNo(), tok.getColumn());
    }

    void parserError(const std::string& moduleName, token::TokenType expected, Token& found, std::ifstream& file)
    {
        /* Used by parser when it finds some other token type than expected */

        std::string errorMessage = "Did not expect " + tokenTypeToString[found.getType()];
        errorMessage += ",\nexpected " + tokenTypeToString[expected];
        errorOnToken(moduleName, errorMessage, file, found);
    }

    void parserErrorMultiple(const std::string& moduleName,
                             const token::TokenType* expected,
                             int ntoks,
                             const Token& found,
                             std::ifstream& file)
    {
        /* Used by parser when if finds a token type that does not match
           multiple token types expected */

        std::string errorMessage = "Did not expect " + tokenTypeToString[found.getType()];
        errorMessage += ",\nexpected ";

        for (int i = 0; i < ntoks - 1; i++)
            errorMessage += ((tokenTypeToString[expected[i]]) + " or ");

        errorMessage += tokenTypeToString[expected[ntoks - 1]];

        errorOnToken(moduleName, errorMessage, file, found);
    }

    std::string formatType(const icode::TypeDescription& type)
    {
        std::string str = typeDescriptionToString(type);
        str = stringReplace(str, "@", "::");
        str = stringReplace(str, "~", "*");

        return str;
    }

    void typeError(const std::string& moduleName,
                   std::ifstream& file,
                   const Token& tok,
                   const icode::TypeDescription& expected,
                   const icode::TypeDescription& found)
    {
        /* Used by ir generator for type errors */

        icode::TypeDescription modifiedFound = found;

        if (icode::dataTypeIsEqual(found.dtype, expected.dtype) && found.dtype != icode::STRUCT)
            modifiedFound.dtypeName = expected.dtypeName;

        if (expected.isPointer() && modifiedFound.dimensions.size() <= 1)
            modifiedFound.becomePointer();

        std::string foundString = formatType(modifiedFound);

        std::string expectedString = formatType(expected);

        std::string errorMessage = "Type error, did not expect " + foundString;
        errorMessage += ",\nexpected " + expectedString;

        errorOnToken(moduleName, errorMessage, file, tok);
    }

    void internalCompilerErrorToken(const std::string& moduleName, std::ifstream& file, const Token& tok)
    {
        errorOnToken(moduleName, "Internal compiler error, REPORT THIS BUG", file, tok);
    }

    void internalCompileError(const std::string& moduleName)
    {
        errorOnModuleName(moduleName, "Internal compiler error, REPORT THIS BUG");
    }
}