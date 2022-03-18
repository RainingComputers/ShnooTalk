#include <vector>
#include <algorithm>

template <class T>
bool itemInList(const T& item, const std::vector<T>& list)
{
    return std::find(list.begin(), list.end(), item) != list.end();
}