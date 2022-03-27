#include "StringReplace.hpp"

std::string stringReplace(std::string str, const std::string& from, const std::string& to)
{
    size_t start = 0;

    while ((start = str.find(from, start)) != std::string::npos)
    {
        str.replace(start, from.length(), to);
        start += to.length();
    }

    return str;
}