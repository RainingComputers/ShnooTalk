#include "Define.hpp"

namespace icode
{
    DefineDescription createIntDefineDescription(int val, DataType dtype)
    {
        DefineDescription intDefine;
        intDefine.val.integer = val;
        intDefine.dtype = dtype;
        return intDefine;
    }

    DefineDescription createFloatDefineDescription(float val, DataType dtype)
    {
        DefineDescription floatDefine;
        floatDefine.val.floating = val;
        floatDefine.dtype = dtype;
        return floatDefine;
    }
}
