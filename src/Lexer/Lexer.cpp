#include "Lexer.hpp"

static std::regex identifierRegex(R"([a-zA-Z_][a-zA-Z0-9_]*)");
static std::regex integerRegex(R"([0-9]+)");
static std::regex hexRegex(R"(0x[a-fA-f0-9]+)");
static std::regex binRegex(R"(0b[0-1]+)");
static std::regex floatRegex(R"([0-9]*\.[0-9]+)");

namespace lexer
{
    bool readLine(std::ifstream& file, std::string& line)
    {
        try
        {
            getline(file, line);
            return true;
        }
        catch (const std::ifstream::failure& e)
        {
            return false;
        }
    }

    Lexer::Lexer(std::ifstream& file, Console& console)
      : console(console)
    {
        consume(file);
        getTokenIndex = 0;
    }

    void Lexer::consume(std::ifstream& file)
    {
        std::string line;
        int lineNo = 0;

        while (readLine(file, line))
        {
            lineNo++;
            consumeLine(line, lineNo);
        }

        tokenQueue.push_back(Token("", token::END_OF_FILE, line.length(), lineNo));
    }

    LenTypePair Lexer::consumePunctuatorOrStringLtrl(const std::string& line, int lineNo, int i)
    {
        switch (line[i])
        {
            case '(':
                return LenTypePair(1, token::LPAREN);
            case ')':
                return LenTypePair(1, token::RPAREN);
            case '{':
                return LenTypePair(1, token::OPEN_BRACE);
            case '}':
                return LenTypePair(1, token::CLOSE_BRACE);
            case ':':
                switch (line[i + 1])
                {
                    case ':':
                        return LenTypePair(2, token::DOUBLE_COLON);
                    default:
                        return LenTypePair(1, token::COLON);
                }
            case ',':
                return LenTypePair(1, token::COMMA);
            case '\'':
                if (line[i + 1] == '\\' && line[i + 2] == '\'')
                    return LenTypePair(3, token::INVALID);
                else if (line[i + 2] == '\'')
                    return LenTypePair(3, token::CHAR_LITERAL);
                else if (line[i + 1] == '\\' && line[i + 3] == '\'')
                    return LenTypePair(4, token::CHAR_LITERAL);
                else
                    return LenTypePair(1, token::CAST);
            case '=':
                switch (line[i + 1])
                {
                    case '=':
                        return LenTypePair(2, token::CONDN_EQUAL);
                    default:
                        return LenTypePair(1, token::EQUAL);
                }
            case '!':
                switch (line[i + 1])
                {
                    case '=':
                        return LenTypePair(2, token::CONDN_NOT_EQUAL);
                    default:
                        return LenTypePair(1, token::CONDN_NOT);
                }
                break;
            case '~':
                return LenTypePair(1, token::NOT);
            case '[':
                return LenTypePair(1, token::OPEN_SQAURE);
            case ']':
                return LenTypePair(1, token::CLOSE_SQUARE);
            case '+':
                switch (line[i + 1])
                {
                    case '=':
                        return LenTypePair(2, token::PLUS_EQUAL);
                    default:
                        return LenTypePair(1, token::PLUS);
                }
                break;
            case '-':
                switch (line[i + 1])
                {
                    case '=':
                        return LenTypePair(2, token::MINUS_EQUAL);
                    case '>':
                        return LenTypePair(2, token::RIGHT_ARROW);
                    default:
                        return LenTypePair(1, token::MINUS);
                }
                break;
            case '|':
                switch (line[i + 1])
                {
                    case '=':
                        return LenTypePair(2, token::OR_EQUAL);
                    default:
                        return LenTypePair(1, token::BITWISE_OR);
                }
                break;
            case '^':
                switch (line[i + 1])
                {
                    case '=':
                        return LenTypePair(2, token::XOR_EQUAL);
                    default:
                        return LenTypePair(1, token::BITWISE_XOR);
                }
                break;
            case '<':
                switch (line[i + 1])
                {
                    case '<':
                        return LenTypePair(2, token::LEFT_SHIFT);
                    case '=':
                        return LenTypePair(2, token::LESS_THAN_EQUAL);
                    default:
                        return LenTypePair(1, token::LESS_THAN);
                }
                break;
            case '>':
                switch (line[i + 1])
                {
                    case '>':
                        return LenTypePair(2, token::RIGHT_SHIFT);
                    case '=':
                        return LenTypePair(2, token::GREATER_THAN_EQUAL);
                    default:
                        return LenTypePair(1, token::GREATER_THAN);
                }
                break;
            case '*':
                switch (line[i + 1])
                {
                    case '=':
                        return LenTypePair(2, token::MULTIPLY_EQUAL);
                    default:
                        return LenTypePair(1, token::MULTIPLY);
                }
                break;
            case '/':
                switch (line[i + 1])
                {
                    case '=':
                        return LenTypePair(2, token::DIVIDE_EQUAL);
                    default:
                        return LenTypePair(1, token::DIVIDE);
                }
                break;
            case '%':
                return LenTypePair(1, token::MOD);
            case '&':
                switch (line[i + 1])
                {
                    case '=':
                        return LenTypePair(2, token::AND_EQUAL);
                    default:
                        return LenTypePair(1, token::BITWISE_AND);
                }
                break;
            case ';':
                return LenTypePair(1, token::SEMICOLON);
            case '.':
                return LenTypePair(1, token::DOT);
            case '"':
            {
                /* Find ending quote that is not escaped with backslash */
                size_t endIndex = std::string::npos;
                size_t startQuoteIndex = i;
                while (true)
                {
                    endIndex = line.find("\"", startQuoteIndex + 1);

                    if (endIndex == std::string::npos)
                        console.lexerError("Invalid STRING LITERAL", line, lineNo, i);

                    if (line[endIndex - 1] != '\\')
                        break;
                    else
                        startQuoteIndex = endIndex + 1;
                }

                return LenTypePair(endIndex - i + 1, token::STR_LITERAL);
            }
            case ' ':
                return LenTypePair(1, token::SPACE);
            default:
                return LenTypePair(1, token::NONE);
        }
    }

    token::TokenType Lexer::typeFromStringMatch(const std::string& string)
    {
        static std::map<std::string, token::TokenType> stringTypeMap = {
            { "use", token::USE },         { "from", token::FROM },
            { "fn", token::FUNCTION },     { "struct", token::STRUCT },
            { "class", token::STRUCT },    { "enum", token::ENUM },
            { "def", token::DEF },         { "var", token::VAR },
            { "const", token::CONST },     { "if", token::IF },
            { "elif", token::ELSEIF },     { "else", token::ELSE },
            { "while", token::WHILE },     { "for", token::FOR },
            { "break", token::BREAK },     { "continue", token::CONTINUE },
            { "return", token::RETURN },   { "mut", token::MUTABLE },
            { "and", token::CONDN_AND },   { "or", token::CONDN_OR },
            { "not", token::CONDN_NOT },   { "void", token::VOID },
            { "sizeof", token::SIZEOF },   { "print", token::PRINT },
            { "println", token::PRINTLN }, { "input", token::INPUT },
            { "exit", token::EXIT },
        };

        auto search = stringTypeMap.find(string);

        if (search != stringTypeMap.end())
            return search->second;

        if (std::regex_match(string, integerRegex))
            return token::INT_LITERAL;
        if (std::regex_match(string, hexRegex))
            return token::HEX_LITERAL;
        if (std::regex_match(string, binRegex))
            return token::BIN_LITERAL;
        if (std::regex_match(string, floatRegex))
            return token::FLOAT_LITERAL;
        if (std::regex_match(string, identifierRegex))
            return token::IDENTIFIER;

        return token::INVALID;
    }

    bool Lexer::pushPrecedingTokenIfIdentifier(std::string& line, int lineNo, size_t startIndex, size_t i)
    {
        /* Returns true if it pushed a token::IDENTIFIER type token */

        if (startIndex == i)
            return false;

        std::string preceedingTokenString = line.substr(startIndex, i - startIndex);

        if (std::regex_match(preceedingTokenString, identifierRegex))
        {
            tokenQueue.push_back(Token(preceedingTokenString, token::IDENTIFIER, startIndex, lineNo));
            return true;
        }

        return false;
    }

    void Lexer::pushPrecedingToken(std::string& line, int lineNo, size_t startIndex, size_t i)
    {
        if (startIndex == i)
            return;

        std::string precedingTokenString = line.substr(startIndex, i - startIndex);
        token::TokenType precedingTokenType = typeFromStringMatch(precedingTokenString);

        tokenQueue.push_back(Token(precedingTokenString, precedingTokenType, startIndex, lineNo));
    }

    void Lexer::consumeLine(std::string& line, int lineNo)
    {
        line += ' '; /* line is not const because of this */

        bool buildingToken = false;
        size_t startIndex = std::string::npos;

        for (size_t i = 0; i < line.length();)
        {
            if (line[i] == '#')
                break;

            if (!buildingToken && line[i] == ' ')
            {
                i++;
                continue;
            }

            if (!buildingToken)
            {
                buildingToken = true;
                startIndex = i;
                continue;
            }

            LenTypePair lenTokenPair = consumePunctuatorOrStringLtrl(line, lineNo, i);

            /* Punctuator tokens include operators, string literals, space */
            size_t punctuatorTokenLen = lenTokenPair.first;
            token::TokenType punctuatorTokenType = lenTokenPair.second;

            if (punctuatorTokenType == token::DOT)
            {
                /* Push dot token if preceding token is an identifier */
                if (pushPrecedingTokenIfIdentifier(line, lineNo, startIndex, i) || startIndex == i)
                {
                    tokenQueue.push_back(Token(line.substr(i, punctuatorTokenLen), punctuatorTokenType, i, lineNo));
                    buildingToken = false;
                }
            }
            else if (punctuatorTokenType != token::NONE)
            {
                /* PUSH the token preceding the punctuator/space */
                pushPrecedingToken(line, lineNo, startIndex, i);

                /* Add punctuator token to queue */
                if (punctuatorTokenType != token::SPACE)
                    tokenQueue.push_back(Token(line.substr(i, punctuatorTokenLen), punctuatorTokenType, i, lineNo));

                buildingToken = false;
            }

            i += punctuatorTokenLen;
        }
    }

    bool Lexer::getToken(Token& symbol)
    {
        /* Return latest token from queue and pop it */

        if (tokenQueue.empty())
            return false;

        symbol = tokenQueue[getTokenIndex++];
        return true;
    }

    Token Lexer::peekToken()
    {
        /* Return latest token without popping */
        return tokenQueue[getTokenIndex];
    }

    Token Lexer::doublePeekToken()
    {
        return tokenQueue[getTokenIndex + 1];
    }
} // namespace lexer
