#ifndef DEFINE_HPP
#define DEFINE_HPP

#include "DataType.hpp"

namespace icode
{
    struct Define
    {
        union
        {
            int integer;
            float floating;
        } val;

        DataType dtype;
    };

    Define createIntDefine(int val, DataType dtype);
    Define createFloatDefine(float val, DataType dtype);
}

#endif