#include <map>

template<class K, class V>
bool getMapElement(std::map<K, V>& map, const K& key, V& returnValue)
{
    auto pair = map.find(key);

    if (pair == map.end())
        return false;

    returnValue = pair->second;
    return true;
}