#ifndef CONTROL_STATEMENT_HPP
#define CONTROL_STATEMENT_HPP

#include "../irgen_old.hpp"

void ifStatement(irgen::ir_generator& ctx,
                 const Node& root,
                 bool isLoopBlock,
                 const icode::Operand& loopLabel,
                 const icode::Operand& breakLabel,
                 const icode::Operand& continueLabel);

void whileLoop(irgen::ir_generator& ctx, const Node& root);

void forLoop(irgen::ir_generator& ctx, const Node& root);

void continueStatement(irgen::ir_generator& ctx,
                       bool isLoopBlock,
                       const icode::Operand& continueLabel,
                       const Token& token);

void breakStatement(irgen::ir_generator& ctx, bool isLoopBlock, const icode::Operand& breakLabel, const Token& token);

#endif