#include "OperatorTokenToInstruction.hpp"

using namespace icode;

Instruction tokenToBinaryOperator(const generator::GeneratorContext& ctx, const Token& tok)
{
    switch (tok.getType())
    {
        case token::MULTIPLY:
        case token::MULTIPLY_EQUAL:
            return MUL;
        case token::DIVIDE:
        case token::DIVIDE_EQUAL:
            return DIV;
        case token::MOD:
            return MOD;
        case token::PLUS:
        case token::PLUS_EQUAL:
            return ADD;
        case token::MINUS:
        case token::MINUS_EQUAL:
            return SUB;
        case token::RIGHT_SHIFT:
            return RSH;
        case token::LEFT_SHIFT:
            return LSH;
        case token::BITWISE_AND:
        case token::AND_EQUAL:
            return BWA;
        case token::BITWISE_XOR:
        case token::XOR_EQUAL:
            return BWX;
        case token::BITWISE_OR:
        case token::OR_EQUAL:
            return BWO;
        case token::CONDN_AND:
        case token::CONDN_OR:
        case token::LESS_THAN:
        case token::LESS_THAN_EQUAL:
        case token::GREATER_THAN:
        case token::GREATER_THAN_EQUAL:
        case token::CONDN_EQUAL:
        case token::CONDN_NOT_EQUAL:
            ctx.console.compileErrorOnToken("Did not expect conditional operator", tok);
        default:
            ctx.console.internalBugErrorOnToken(tok);
    }
}

Instruction tokenToCompareOperator(const generator::GeneratorContext& ctx, const Token& tok)
{
    switch (tok.getType())
    {
        case token::LESS_THAN:
            return LT;
        case token::LESS_THAN_EQUAL:
            return LTE;
        case token::GREATER_THAN:
            return GT;
        case token::GREATER_THAN_EQUAL:
            return GTE;
        case token::CONDN_EQUAL:
            return EQ;
        case token::CONDN_NOT_EQUAL:
            return NEQ;
        default:
            ctx.console.compileErrorOnToken("Invalid conditional expression", tok);
    }
}

std::string tokenToCustomOperatorString(const generator::GeneratorContext& ctx, const Token& tok)
{
    switch (tok.getType())
    {
        case token::MULTIPLY:
        case token::MULTIPLY_EQUAL:
            return "multiply";
        case token::DIVIDE:
        case token::DIVIDE_EQUAL:
            return "divide";
        case token::MOD:
            return "mod";
        case token::PLUS:
        case token::PLUS_EQUAL:
            return "add";
        case token::MINUS:
        case token::MINUS_EQUAL:
            return "subtract";
        case token::RIGHT_SHIFT:
            return "rightShift";
        case token::LEFT_SHIFT:
            return "leftShift";
        case token::BITWISE_AND:
        case token::AND_EQUAL:
            return "bitwiseAnd";
        case token::BITWISE_XOR:
        case token::XOR_EQUAL:
            return "bitwiseXor";
        case token::BITWISE_OR:
        case token::OR_EQUAL:
            return "bitwiseOr";
        case token::LESS_THAN:
            return "lessThan";
        case token::LESS_THAN_EQUAL:
            return "lessThanOrEqual";
        case token::GREATER_THAN:
            return "greaterThan";
        case token::GREATER_THAN_EQUAL:
            return "greaterThanOrEqual";
        case token::CONDN_EQUAL:
            return "isEqual";
        case token::CONDN_NOT_EQUAL:
            return "isNotEqual";
        case token::IN:
            return "in";
        default:
            ctx.console.internalBugErrorOnToken(tok);
    }
}
