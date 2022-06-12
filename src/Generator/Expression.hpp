#ifndef GENERATOR_EXPRESSION
#define GENERATOR_EXPRESSION

#include "GeneratorContext.hpp"

Unit createCallFunctionMust(generator::GeneratorContext& ctx,
                            const std::vector<Token>& actualParamTokens,
                            const std::vector<Unit>& actualParams,
                            const icode::FunctionDescription& callee);

Unit createCallFunction(generator::GeneratorContext& ctx,
                        const std::vector<Token>& actualParamTokens,
                        const std::vector<Unit>& actualParams,
                        const Token& errorToken,
                        const icode::FunctionDescription& callee);

Unit term(generator::GeneratorContext& ctx, const Node& root);

Unit ordinaryExpression(generator::GeneratorContext& ctx, const Node& root);

void truthyOperator(generator::GeneratorContext& ctx,
                    const Unit& LHS,
                    const Token& expressionToken,
                    const icode::Operand& trueLabel,
                    const icode::Operand& falseLabel,
                    bool trueFall);

void createJumps(generator::GeneratorContext& ctx,
                 const icode::Operand& trueLabel,
                 const icode::Operand& falseLabel,
                 bool trueFall);

void conditionalExpression(generator::GeneratorContext& ctx,
                           const Node& root,
                           const icode::Operand& trueLabel,
                           const icode::Operand& falseLabel,
                           bool trueFall);

Unit expression(generator::GeneratorContext& ctx, const Node& root);

Unit binaryOperator(generator::GeneratorContext& ctx,
                    const Token& binaryOperator,
                    const Token& LHSToken,
                    const Token& RHSToken,
                    const Unit& LHS,
                    const Unit& RHS);

#endif