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
        EXTERN_C,
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
        DO,
        FOR,
        LOOP,
        BREAK,
        CONTINUE,
        RETURN,
        VOID,
        EQUAL,
        WALRUS,
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
        PTR_CAST,
        ARRAY_PTR_CAST,
        OPEN_SQUARE,
        CLOSE_SQUARE,
        OPEN_BRACE,
        CLOSE_BRACE,
        EMPTY_SUBSCRIPT,
        RIGHT_ARROW,
        LEFT_ARROW,
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
        IN,
        BITWISE_AND,
        CONDN_AND,
        USE,
        GENERIC,
        AS,
        FROM,
        MUTABLE,
        SEMICOLON,
        END_OF_LINE,
        END_OF_FILE,
        INVALID,
        SIZEOF,
        MAKE,
        ADDR,
        TYPEOF,
        PRINT,
        PRINTLN,
        INPUT,
        GENERATED
    };
}

class Token
{
    std::string string;
    std::string unescapedString;
    token::TokenType type;
    unsigned int column;
    unsigned int line;

    std::string file;

    void initializeUnescapedString();

public:
    int getPrecedence() const;
    bool isBitwiseOperator() const;
    bool isConditionalOperator() const;
    bool isIntLiteral() const;
    bool isEqualOrLeftArrow() const;
    bool isBinaryOperator() const;
    long toInt() const;
    double toFloat() const;
    std::string toString() const;
    std::string toFunctionNameString() const;
    std::string toUnescapedString() const;
    token::TokenType getType() const;
    std::string getLineColString() const;

    int getLineNo() const;
    int getColumn() const;

    std::string getFileName() const;

    Token(std::string fileName = "",
          std::string tokenString = "",
          token::TokenType tokenType = token::NONE,
          unsigned int column = 0,
          unsigned int linenumber = 0);
};

#endif