#ifndef PP_JSON_MAP_UTIL
#define PP_JSON_MAP_UTIL

#include <map>
#include <string>

#include "FlatJSONPrinter.hpp"

template<class V>
void printNumberMap(const std::map<std::string, V>& keyValueMap, FlatJSONPrinter& jsonp)
{
    jsonp.begin();

    for (auto pair : keyValueMap)
        jsonp.printNumber(pair.first, pair.second);

    jsonp.end();
}

void printStringMap(const std::map<std::string, std::string>& keyValueMap, FlatJSONPrinter& jsonp);

#endif