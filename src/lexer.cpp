#include "lexer.hpp"

static std::regex identifier_regex(R"([a-zA-Z_][a-zA-Z0-9_]*)");
static std::regex integer_regex(R"([0-9]+)");
static std::regex hex_regex(R"(0x[a-fA-f0-9]+)");
static std::regex bin_regex(R"(0b[0-1]+)");
static std::regex float_regex(R"([0-9]*\.[0-9]+)");

namespace lexer
{
    lexical_analyser::lexical_analyser(const std::string& name, std::ifstream& ifile) { feed(name, ifile); }

    void lexical_analyser::feed(const std::string& name, std::ifstream& ifile)
    {
        file_name = name;
        front_idx = 0;

        std::string line;
        int lineno = 0;
        while (true)
        {
            try
            {
                getline(ifile, line);
            }
            catch (const std::ifstream::failure& e)
            {
                break;
            }

            lineno++;
            feed_line(line, lineno);
        }

        token_q.push_back(token::token("", token::END_OF_FILE, line.length(), lineno));
    }

    void lexical_analyser::feed_line(std::string& line, int lineno)
    {
        line += ' '; /* line is not const beacuse of this */

        bool token_start = false;
        size_t start_idx = std::string::npos;

        for (size_t i = 0; i < line.length();)
        {
            if (!token_start)
            {
                /* Ignore whitespace*/
                if (line[i] == ' ')
                    i++;
                else
                {
                    token_start = true;
                    start_idx = i;
                }
            }
            /* Ignore comments */
            else if (line[i] == '#')
                break;
            else
            {
                /* Check for punctuator/operator/space */
                token::token_type type = token::INVALID;
                size_t len = 1;

                switch (line[i])
                {
                    case '(':
                        type = token::LPAREN;
                        break;
                    case ')':
                        type = token::RPAREN;
                        break;
                    case '{':
                        type = token::OPEN_BRACE;
                        break;
                    case '}':
                        type = token::CLOSE_BRACE;
                        break;
                    case ':':
                        switch (line[i + 1])
                        {
                            case ':':
                                type = token::DOUBLE_COLON;
                                len = 2;
                                break;
                            default:
                                type = token::COLON;
                                break;
                        }
                        break;
                    case ',':
                        type = token::COMMA;
                        break;
                    case '\'':
                        if (line[i + 2] == '\'')
                        {
                            type = token::CHAR_LITERAL;
                            len = 3;
                        }
                        else if (line[i + 1] == '\\' && line[i + 3] == '\'')
                        {
                            type = token::CHAR_LITERAL;
                            len = 4;
                        }
                        else
                            type = token::CAST;

                        break;
                    case '=':
                        switch (line[i + 1])
                        {
                            case '=':
                                type = token::CONDN_EQUAL;
                                len = 2;
                                break;
                            default:
                                type = token::EQUAL;
                                break;
                        }
                        break;
                    case '!':
                        switch (line[i + 1])
                        {
                            case '=':
                                type = token::CONDN_NOT_EQUAL;
                                len = 2;
                                break;
                            default:
                                type = token::CONDN_NOT;
                                break;
                        }
                        break;
                    case '~':
                        type = token::NOT;
                        break;
                    case '[':
                        type = token::OPEN_SQAURE;
                        break;
                    case ']':
                        type = token::CLOSE_SQUARE;
                        break;
                    case '+':
                        switch (line[i + 1])
                        {
                            case '=':
                                type = token::PLUS_EQUAL;
                                len = 2;
                                break;
                            default:
                                type = token::PLUS;
                                break;
                        }
                        break;
                    case '-':
                        switch (line[i + 1])
                        {
                            case '=':
                                type = token::MINUS_EQUAL;
                                len = 2;
                                break;
                            case '>':
                                type = token::RIGHT_ARROW;
                                len = 2;
                                break;
                            default:
                                type = token::MINUS;
                                break;
                        }
                        break;
                    case '|':
                        switch (line[i + 1])
                        {
                            case '=':
                                type = token::OR_EQUAL;
                                len = 2;
                                break;
                            default:
                                type = token::BITWISE_OR;
                                break;
                        }
                        break;
                    case '^':
                        switch (line[i + 1])
                        {
                            case '=':
                                type = token::XOR_EQUAL;
                                len = 2;
                                break;
                            default:
                                type = token::BITWISE_XOR;
                                break;
                        }
                        break;
                    case '<':
                        switch (line[i + 1])
                        {
                            case '<':
                                type = token::LEFT_SHIFT;
                                len = 2;
                                break;
                            case '=':
                                type = token::LESS_THAN_EQUAL;
                                len = 2;
                                break;
                            default:
                                type = token::LESS_THAN;
                                break;
                        }
                        break;
                    case '>':
                        switch (line[i + 1])
                        {
                            case '>':
                                type = token::RIGHT_SHIFT;
                                len = 2;
                                break;
                            case '=':
                                type = token::GREATER_THAN_EQUAL;
                                len = 2;
                                break;
                            default:
                                type = token::GREATER_THAN;
                                break;
                        }
                        break;
                    case '*':
                        switch (line[i + 1])
                        {
                            case '=':
                                type = token::MULTIPLY_EQUAL;
                                len = 2;
                                break;
                            default:
                                type = token::MULTIPLY;
                                break;
                        }
                        break;
                    case '/':
                        switch (line[i + 1])
                        {
                            case '=':
                                type = token::DIVIDE_EQUAL;
                                len = 2;
                                break;
                            default:
                                type = token::DIVIDE;
                                break;
                        }
                        break;
                    case '%':
                        type = token::MOD;
                        break;
                    case '&':
                        switch (line[i + 1])
                        {
                            case '=':
                                type = token::AND_EQUAL;
                                len = 2;
                                break;
                            default:
                                type = token::BITWISE_AND;
                                break;
                        }
                        break;
                    case ';':
                        type = token::SEMICOLON;
                        break;
                    case '.':
                        type = token::DOT;
                        break;
                    case '"':
                    {
                        /* Find ending quote that is not escaped with backslash */
                        size_t end_idx = std::string::npos;
                        size_t quote_pos = i;
                        while (true)
                        {
                            end_idx = line.find("\"", quote_pos + 1);

                            if (end_idx == std::string::npos)
                            {
                                miklog::println("MODULE " + file_name);
                                miklog::error_line("Invalid STRING LITERAL", line, lineno, i);
                                throw miklog::compile_error();
                            }

                            if (line[end_idx - 1] != '\\')
                                break;
                            else
                                quote_pos = end_idx + 1;
                        }

                        len = end_idx - i + 1;
                        type = token::STR_LITERAL;
                        break;
                    }
                    case ' ':
                        type = token::SPACE;
                        break;
                    default:
                        break;
                }

                bool push_dot = false;

                if (type == token::DOT)
                {
                    /* Get the token preceding the dot */
                    if (start_idx != i)
                    {
                        std::string prec_token_str = line.substr(start_idx, i - start_idx);
                        token::token_type prec_type = token::NONE;

                        /* Check if identifier */
                        if (std::regex_match(prec_token_str, identifier_regex))
                        {
                            prec_type = token::IDENTIFIER;
                            push_dot = true;
                        }

                        /* If identifier was found, push it */
                        if (push_dot)
                            token_q.push_back(token::token(prec_token_str, prec_type, start_idx, lineno));
                    }
                    else
                        push_dot = true;

                    /* If preceding token was an identifier, push dot */
                    if (push_dot)
                    {
                        token_q.push_back(token::token(line.substr(i, len), type, i, lineno));
                        token_start = false;
                    }
                }
                else if (type != token::INVALID)
                {
                    /* Get the token preceding the punctuator/space */
                    if (start_idx != i)
                    {
                        std::string prec_token_str = line.substr(start_idx, i - start_idx);
                        token::token_type prec_type = token::NONE;

                        /* Check for keywords */
                        if (prec_token_str == "use")
                            prec_type = token::USE;
                        else if (prec_token_str == "from")
                            prec_type = token::FROM;
                        else if (prec_token_str == "fn")
                            prec_type = token::FUNCTION;
                        else if (prec_token_str == "struct")
                            prec_type = token::STRUCT;
                        else if (prec_token_str == "class")
                            prec_type = token::STRUCT;
                        else if (prec_token_str == "enum")
                            prec_type = token::ENUM;
                        else if (prec_token_str == "def")
                            prec_type = token::DEF;
                        else if (prec_token_str == "var")
                            prec_type = token::VAR;
                        else if (prec_token_str == "const")
                            prec_type = token::CONST;
                        else if (prec_token_str == "if")
                            prec_type = token::IF;
                        else if (prec_token_str == "elif")
                            prec_type = token::ELSEIF;
                        else if (prec_token_str == "else")
                            prec_type = token::ELSE;
                        else if (prec_token_str == "while")
                            prec_type = token::WHILE;
                        else if (prec_token_str == "for")
                            prec_type = token::FOR;
                        else if (prec_token_str == "break")
                            prec_type = token::BREAK;
                        else if (prec_token_str == "continue")
                            prec_type = token::CONTINUE;
                        else if (prec_token_str == "return")
                            prec_type = token::RETURN;
                        else if (prec_token_str == "mut")
                            prec_type = token::MUTABLE;
                        else if (prec_token_str == "and")
                            prec_type = token::CONDN_AND;
                        else if (prec_token_str == "or")
                            prec_type = token::CONDN_OR;
                        else if (prec_token_str == "not")
                            prec_type = token::CONDN_NOT;
                        else if (prec_token_str == "void")
                            prec_type = token::VOID;
                        else if (prec_token_str == "sizeof")
                            prec_type = token::SIZEOF;
                        else if (prec_token_str == "print")
                            prec_type = token::PRINT;
                        else if (prec_token_str == "println")
                            prec_type = token::PRINTLN;
                        else if (prec_token_str == "input")
                            prec_type = token::INPUT;
                        else if (prec_token_str == "exit")
                            prec_type = token::EXIT;

                        /* Check for literals */
                        else if (std::regex_match(prec_token_str, integer_regex))
                            prec_type = token::INT_LITERAL;
                        else if (std::regex_match(prec_token_str, hex_regex))
                            prec_type = token::HEX_LITERAL;
                        else if (std::regex_match(prec_token_str, bin_regex))
                            prec_type = token::BIN_LITERAL;
                        else if (std::regex_match(prec_token_str, float_regex))
                            prec_type = token::FLOAT_LITERAL;

                        /* Check if identifier */
                        else if (std::regex_match(prec_token_str, identifier_regex))
                            prec_type = token::IDENTIFIER;

                        /* No match */
                        else
                            prec_type = token::INVALID;

                        token_q.push_back(token::token(prec_token_str, prec_type, start_idx, lineno));
                    }

                    /* Add punctuator token to queue */
                    if (type != token::SPACE)
                        token_q.push_back(token::token(line.substr(i, len), type, i, lineno));

                    token_start = false;
                }

                /* Next char */
                i += len;
            }
        }
    }

    bool lexical_analyser::get_token(token::token& symbol)
    {
        /* Return latest token from queue
            and pop it */

        if (token_q.empty())
            return false;

        symbol = token_q[front_idx++];
        return true;
    }

    token::token lexical_analyser::peek_token()
    {
        /* Return latest token without popping */
        return token_q[front_idx];
    }

    token::token lexical_analyser::dpeek_token() { return token_q[front_idx + 1]; }
} // namespace lexer
