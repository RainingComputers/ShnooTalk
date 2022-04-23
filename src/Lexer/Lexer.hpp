#ifndef LEXER_LEXER
#define LEXER_LEXER

#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include "../Console/Console.hpp"
#include "../Token/Token.hpp"

typedef std::pair<int, token::TokenType> LenTypePair;

namespace lexer
{
    class Lexer
    {
        std::vector<Token> tokenQueue;
        std::string fileName;
        size_t getTokenIndex;
        Console& console;

        void consumeLine(std::string& line, int lineNo);
        bool pushPrecedingTokenIfIdentifier(std::string& line, int lineNo, size_t startIndex, size_t i);
        void pushPrecedingToken(std::string& line, int lineNo, size_t startIndex, size_t i);
        bool ignoreWhiteSpace(std::string& line, size_t i);
        LenTypePair consumePunctuatorOrStringLtrl(const std::string& line, int lineNo, int i);
        token::TokenType typeFromStringMatch(const std::string& string);
        void consume(std::ifstream& file);

    public:
        Lexer(std::ifstream& file, Console& console);
        Token getToken();
        Token peekToken();
        Token doublePeekToken();
        Token matchedBracketPeek(token::TokenType open, token::TokenType close);
    };
}

#endif