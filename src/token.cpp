#include "token.hpp"

namespace token
{
    int get_precedence(token_type type)
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
            /* Addignments like =, +=, -= etc. also don't count as operators */
            default:
                return 0;
                break;
        }

        return 0;
    }

    token::token(std::string token_str, token_type tok_type, unsigned int column, unsigned int linenumber)
    {
        str = token_str;
        type = tok_type;
        col = column;
        lineno = linenumber;
    }
} // namespace token
