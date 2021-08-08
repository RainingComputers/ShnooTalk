#ifndef TOKEN_TOKEN
#define TOKEN_TOKEN

#include <string>

namespace token
{
    enum TokenType
    {
        NONE,
        SPACE,
        FUNCTION,
        EXTERN_FUNCTION,
        IDENTIFIER,
        LPAREN,
        RPAREN,
        STRUCT,
        ENUM,
        DEF,
        BEGIN,
        END,
        COLON,
        DOUBLE_COLON,
        COMMA,
        DOT,
        VAR,
        CONST,
        STR_LITERAL,
        CHAR_LITERAL,
        INT_LITERAL,
        HEX_LITERAL,
        BIN_LITERAL,
        FLOAT_LITERAL,
        IF,
        ELSEIF,
        ELSE,
        WHILE,
        FOR,
        BREAK,
        CONTINUE,
        RETURN,
        VOID,
        EQUAL,
        PLUS_EQUAL,
        MINUS_EQUAL,
        DIVIDE_EQUAL,
        MULTIPLY_EQUAL,
        OR_EQUAL,
        AND_EQUAL,
        XOR_EQUAL,
        NOT,
        CONDN_NOT,
        CAST,
        OPEN_SQUARE,
        CLOSE_SQUARE,
        OPEN_BRACE,
        CLOSE_BRACE,
        RIGHT_ARROW,
        PLUS,
        MINUS,
        BITWISE_OR,
        BITWISE_XOR,
        RIGHT_SHIFT,
        LEFT_SHIFT,
        CONDN_OR,
        GREATER_THAN,
        LESS_THAN,
        GREATER_THAN_EQUAL,
        LESS_THAN_EQUAL,
        CONDN_EQUAL,
        CONDN_NOT_EQUAL,
        MULTIPLY,
        DIVIDE,
        MOD,
        BITWISE_AND,
        CONDN_AND,
        USE,
        FROM,
        MUTABLE,
        SEMICOLON,
        END_OF_LINE,
        END_OF_FILE,
        INVALID,
        SIZEOF,
        TYPEOF,
        PRINT,
        PRINTLN,
        INPUT
    };
}

class Token
{
    std::string string;
    std::string unescapedString;
    token::TokenType type;
    unsigned int column;
    unsigned int line;

    void initializeUnescapedString();

public:
    int getPrecedence() const;
    bool isBitwiseOperation() const;
    bool isIntLiteral() const;
    int toInt() const;
    float toFloat() const;
    std::string toString() const;
    std::string toUnescapedString() const;
    token::TokenType getType() const;
    std::string getLineColString() const;

    int getLineNo() const;
    int getColumn() const;

    Token(std::string token_str = "",
          token::TokenType tok_type = token::NONE,
          unsigned int column = 0,
          unsigned int linenumber = 0);
};

#endif