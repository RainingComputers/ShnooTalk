#ifndef DEFINE_HPP
#define DEFINE_HPP

#include "DataType.hpp"

namespace icode
{
    struct DefineDescription
    {
        union
        {
            int integer;
            float floating;
        } val;

        DataType dtype;
    };

    DefineDescription createIntDefineDescription(int val, DataType dtype);
    DefineDescription createFloatDefineDescription(float val, DataType dtype);
}

#endif