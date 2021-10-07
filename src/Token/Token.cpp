#include "Token.hpp"

using namespace token;

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

Token::Token(std::string tokenString, TokenType tokenType, unsigned int columnNumber, unsigned int lineNumber)
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
        default:
            return 0;
            break;
    }

    return 0;
}

bool Token::isBitwiseOperator() const
{
    return type == BITWISE_AND || type == BITWISE_OR || type == BITWISE_XOR || type == AND_EQUAL || type == OR_EQUAL ||
           type == XOR_EQUAL;
}

bool Token::isConditionalOperator() const
{
    return type == LESS_THAN || type == LESS_THAN_EQUAL || type == GREATER_THAN || type == GREATER_THAN_EQUAL ||
           type == CONDN_EQUAL || type == CONDN_NOT_EQUAL || type == CONDN_AND || type == CONDN_OR;
}

bool Token::isIntLiteral() const
{
    return type == INT_LITERAL || type == HEX_LITERAL || type == BIN_LITERAL;
}

bool Token::isEqualOrLeftArrow() const
{
    return type == EQUAL || type == LEFT_ARROW;
}

long Token::toInt() const
{
    if (string[0] == '0' && string[1] == 'b')
        return std::stoul(string.substr(2), nullptr, 2);

    return std::stoul(string, nullptr, 0);
}

double Token::toFloat() const
{
    return std::stod(string);
}

std::string Token::toString() const
{
    return string;
}

std::string Token::toUnescapedString() const
{
    return unescapedString;
}

TokenType Token::getType() const
{
    return type;
}

std::string Token::getLineColString() const
{
    return "_l" + std::to_string(line) + "_c" + std::to_string(column);
}

int Token::getLineNo() const
{
    return line;
}

int Token::getColumn() const
{
    return column;
}
