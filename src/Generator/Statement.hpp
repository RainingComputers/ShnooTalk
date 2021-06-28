#ifndef GENERATOR_STATEMENT
#define GENERATOR_STATEMENT

#include "GeneratorContext.hpp"

void ifStatement(generator::GeneratorContext& ctx,
                 const Node& root,
                 bool isLoopBlock,
                 const icode::Operand& loopLabel,
                 const icode::Operand& breakLabel,
                 const icode::Operand& continueLabel);

void whileLoop(generator::GeneratorContext& ctx, const Node& root);

void forLoop(generator::GeneratorContext& ctx, const Node& root);

void continueStatement(generator::GeneratorContext& ctx,
                       bool isLoopBlock,
                       const icode::Operand& continueLabel,
                       const Token& token);

void breakStatement(generator::GeneratorContext& ctx, bool isLoopBlock, const icode::Operand& breakLabel, const Token& token);

void block(generator::GeneratorContext& ctx,
               const Node& root,
               bool isLoopBlock,
               const icode::Operand& loopLabel,
               const icode::Operand& breakLabel,
               const icode::Operand& continueLabel);

#endif