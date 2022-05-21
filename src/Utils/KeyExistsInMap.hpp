#include <map>

template<class K, class V>
bool keyExistsInMap(const std::map<K, V>& map, const K& key)
{
    auto pair = map.find(key);

    if (pair == map.end())
        return false;

    return true;
}