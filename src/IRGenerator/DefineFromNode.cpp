#include "DescriptionGenerator/CreateDefine.hpp"

#include "DefineFromNode.hpp"

void defineFromNode(irgen::ir_generator& ctx, const node::node& root)
{
    createDefine(ctx, root.children[0].tok, root.children[1].tok);
}