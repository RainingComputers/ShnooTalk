#include "token.hpp"

namespace token
{
    char toEscapedCharacter(char c)
    {
        switch (c)
        {
            case 'n':
                return '\n';
            case 'b':
                return '\b';
            case 't':
                return '\t';
            case '0':
                return '\0';
            case '\\':
                return '\\';
            default:
                return c;
        }
    }

    void Token::initializeUnescapedString()
    {
        for (size_t i = 1; i < string.length() - 1; i++)
        {
            if (string[i] == '\\')
                unescapedString += toEscapedCharacter(string[++i]);
            else
                unescapedString += string[i];
        }
    }

    Token::Token(std::string tokenString, tokenType tokenType, unsigned int columnNumber, unsigned int lineNumber)
    {
        string = tokenString;
        type = tokenType;
        column = columnNumber;
        line = lineNumber;

        if (type == STR_LITERAL || type == CHAR_LITERAL)
            initializeUnescapedString();
    }

    int Token::getPrecedence() const
    {
        switch (type)
        {
            case MULTIPLY:
            case DIVIDE:
            case MOD:
                return 10;
                break;
            case PLUS:
            case MINUS:
                return 9;
                break;
            case RIGHT_SHIFT:
            case LEFT_SHIFT:
                return 8;
                break;
            case BITWISE_AND:
                return 7;
                break;
            case BITWISE_XOR:
                return 6;
                break;
            case BITWISE_OR:
                return 5;
                break;
            case LESS_THAN:
            case LESS_THAN_EQUAL:
            case GREATER_THAN:
            case GREATER_THAN_EQUAL:
                return 4;
                break;
            case CONDN_EQUAL:
            case CONDN_NOT_EQUAL:
                return 3;
                break;
            case CONDN_AND:
                return 2;
                break;
            case CONDN_OR:
                return 1;
                break;
            /* Not an operator */
            /* Assignments like =, +=, -= etc. also don't count as operators */
            default:
                return 0;
                break;
        }

        return 0;
    }

    bool Token::isBitwiseOperation() const
    {
        return type == BITWISE_AND || type == BITWISE_OR || type == BITWISE_XOR || type == AND_EQUAL ||
               type == OR_EQUAL || type == XOR_EQUAL;
    }

    int Token::toInt() const
    {
        return std::stoi(string);
    }

    float Token::toFloat() const
    {
        return std::stof(string);
    }
} // namespace token
