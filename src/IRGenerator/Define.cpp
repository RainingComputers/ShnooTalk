#include "DescriptionGenerator/CreateDefine.hpp"

#include "Define.hpp"

void defineFromNode(irgen::ir_generator& ctx, const node::Node& root)
{
    createDefine(ctx, root.children[0].tok, root.children[1].tok);
}