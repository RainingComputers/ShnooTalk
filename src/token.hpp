#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

namespace token
{
    enum token_type
    {
        NONE,
        SPACE,
        FUNCTION,
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
        OPEN_SQAURE,
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
        INPUT,
        EXIT
    };

    int get_precedence(token_type type);

    char to_backspace_char(char c);

    struct token
    {
        std::string str;
        token_type type;
        unsigned int col;
        unsigned int lineno;

        token(std::string token_str = "",
              token_type tok_type = NONE,
              unsigned int column = 0,
              unsigned int linenumber = 0);
    };
}

#endif