#include <map>

template<class K, class V>
bool GetMapElement(std::map<K, V>& map, const K& key, V& val)
{
    auto pair = map.find(key);

    if (pair != map.end())
    {
        val = pair->second;
        return true;
    }

    return false;
}