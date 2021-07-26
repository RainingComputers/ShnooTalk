#include "JSONMapUtil.hpp"

void printStringMap(const std::map<std::string, std::string>& keyValueMap, FlatJSONPrinter& jsonp)
{
    jsonp.begin();

    for (auto pair : keyValueMap)
        jsonp.printString(pair.first, pair.second);

    jsonp.end();
}