#include "Define.hpp"

namespace icode
{
    Define createIntDefine(int val, DataType dtype)
    {
        Define intDefine;
        intDefine.val.integer = val;
        intDefine.dtype = dtype;
        return intDefine;
    }

    Define createFloatDefine(float val, DataType dtype)
    {
        Define floatDefine;
        floatDefine.val.floating = val;
        floatDefine.dtype = dtype;
        return floatDefine;
    }
}
