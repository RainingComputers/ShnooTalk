#include "../Generator/Generic.hpp"
#include "../Utils/StringReplace.hpp"

#include "FormatType.hpp"

size_t findMatchedCloseSquareBracket(const std::string& str, size_t pos)
{
    unsigned int balance = 0;

    for (size_t i = pos; i < str.size(); i += 1)
    {
        if (str[i] == '[')
            balance += 1;

        if (str[i] == ']')
        {
            balance -= 1;
            if (balance == 0)
                return i;
        }
    }

    return std::string::npos;
}

std::string formatGenericType(const std::string& typeStr);

std::string formatTypeParams(const std::string& typeParams)
{
    std::string str;

    size_t start = 0;
    size_t end = 0;

    while (true)
    {
        end = findMatchedCloseSquareBracket(typeParams, start);

        if (end == std::string::npos)
            break;

        std::string singleTypeParam = typeParams.substr(start + 1, end - start - 1);

        if (start == 0)
            str = formatGenericType(singleTypeParam);
        else
            str += ", " + formatGenericType(singleTypeParam);

        start = end + 1;
    }

    return "[" + str + "]";
}

std::string formatGenericType(const std::string& typeStr)
{
    std::string str = typeStr;

    size_t start = str.find(".shtk[");
    size_t end = str.rfind("]::");

    if (start == std::string::npos)
        return str;

    start += 5;
    size_t typeParamLen = end - start + 1;

    std::string typeParams = str.substr(start, typeParamLen);
    str.erase(start, typeParamLen);

    return str + formatTypeParams(typeParams);
}

std::string formatType(const icode::TypeDescription& type)
{
    std::string str = typeDescriptionToString(type);
    str = stringReplace(str, "@", "::");
    str = stringReplace(str, "~", "*");

    return formatGenericType(str);
}
