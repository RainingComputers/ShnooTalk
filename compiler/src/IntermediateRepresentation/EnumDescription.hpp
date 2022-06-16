#ifndef IR_ENUM_DESCRIPTION
#define IR_ENUM_DESCRIPTION

#include <string>

namespace icode
{
    struct EnumDescription
    {
        std::string dtypeName;
        long value;
        std::string moduleName;
    };

}

#endif