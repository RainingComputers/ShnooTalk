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
            return "__multiply";
        case token::DIVIDE:
        case token::DIVIDE_EQUAL:
            return "__divide";
        case token::MOD:
            return "__mod";
        case token::PLUS:
        case token::PLUS_EQUAL:
            return "__add";
        case token::MINUS:
        case token::MINUS_EQUAL:
            return "__subtract";
        case token::RIGHT_SHIFT:
            return "__rightShift";
        case token::LEFT_SHIFT:
            return "__leftShift";
        case token::BITWISE_AND:
        case token::AND_EQUAL:
            return "__bitwiseAnd";
        case token::BITWISE_XOR:
        case token::XOR_EQUAL:
            return "__bitwiseXor";
        case token::BITWISE_OR:
        case token::OR_EQUAL:
            return "__bitwiseOr";
        case token::LESS_THAN:
            return "__lessThan";
        case token::LESS_THAN_EQUAL:
            return "__lessThanOrEqual";
        case token::GREATER_THAN:
            return "__greaterThan";
        case token::GREATER_THAN_EQUAL:
            return "__greaterThanOrEqual";
        case token::CONDN_EQUAL:
            return "__isEqual";
        case token::CONDN_NOT_EQUAL:
            return "__isNotEqual";
        case token::IN:
            return "__in";
        default:
            ctx.console.internalBugErrorOnToken(tok);
    }
}
