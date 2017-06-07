#ifndef MUSH_FLAT_HASH
#define MUSH_FLAT_HASH

#include <vector>
#include <set>
#include <unordered_map>

template <typename T1, typename T2>
class flat_hash
{
    std::vector<T2> data;
    std::unordered_map<T1, size_t> access;
    std::set<size_t> unused;

    public:
        class const_iterator;
        class iterator: public std::iterator<std::random_access_iterator_tag, T2>
        {};


        T2& operator[](const T1& idx)
        {
            return data[access[idx]];
        }

        void add(T1 key, T2 value)
        {
        }

/*
        void emplace_back(T1 key, T2 value)
        {
            if (access.count(key) != 0)
                std::abort();

            data.size()
            data.emplace_back(std::forward<T2>(value));
        }
*/
};

#endif
