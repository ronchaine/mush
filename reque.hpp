#ifndef MUSH_REQUE
#define MUSH_REQUE

#include <deque>
#include <set>

namespace mush
{
    struct Reque_Base {};

    template <typename T, typename A = std::allocator<T>>
    class Reque : public Reque_Base
    {
        private:
            std::deque<T>           storage;
            std::set<size_t>        unused;

        public:
            typedef T               value_type;
            typedef T&              reference;
            typedef const T&        const_reference;
            typedef T*              pointer;
            typedef const T*        const_pointer;
            typedef size_t          size_type;

            class iterator
            {
                private:
                    size_t pos;

                    std::deque<T>&      storage_ref;
                    std::set<size_t>&   unused_ref;

                public:
                    typedef typename A::difference_type difference_type;
                    typedef typename A::value_type      value_type;
                    typedef typename A::reference       reference;
                    typedef typename A::pointer         pointer;

                    typedef std::bidirectional_iterator_tag iterator_category;

                    iterator(size_t sp, std::deque<T>& dr, std::set<size_t>& s)
                        : pos(sp), storage_ref(dr), unused_ref(s) {}
                   ~iterator() {}

                    iterator& operator++()
                    {
                        while(pos < storage_ref.size())
                        {
                            pos++;
                            if (unused_ref.count(pos) == 0)
                                return *this;
                        }
                        return *this;
                    }
                    reference operator*() const
                    {
                        return storage_ref[pos];
                    }
                    pointer operator->() const
                    {
                        return &storage_ref[pos];
                    }
                    bool operator==(const iterator& rhs) const
                    {
                        return pos == rhs.pos;
                    }
                    bool operator!=(const iterator& rhs) const
                    {
                        return pos != rhs.pos;
                    }
            };

            iterator begin()
            {
                return iterator(0, storage, unused);
            }

            iterator end()
            {
                return iterator(storage.size(), storage, unused);
            }

            size_type size()
            {
                return storage.size() - unused.size();
            }

            std::deque<T&>      all()
            {
                std::deque<T&> rval;
                for (size_t i = 0; i < storage.size(); ++i)
                {
                    if (unused.count(i) == 0)
                        rval.emplace_back(i);
                }

                return rval;
            }

            size_t insert(T value)
            {
                size_t rval;
                if (unused.empty())
                {
                    rval = storage.size();
                    storage.emplace_back(value);
                } else {
                    rval = *unused.begin();
                    storage[rval] = value;
                    unused.erase(unused.begin());
                }

                return rval;
            }

            void remove(size_t index)
            {
                if (index >= storage.size())
                    return;

                unused.insert(index);
            }

            const T& operator[](const size_t idx) const
            {
                return storage[idx];
            }

            // get unused index and set it used
            int32_t get_unused_index()
            {
                if (unused.size() != 0)
                {
                    int32_t value = *unused.begin();
                    unused.erase(unused.begin());
                    return value;
                }

                storage.resize(storage.size() + 1);
                return storage.size() - 1;
            }
    };
}

#endif
/*
 Copyright (c) 2018 Jari Ronkainen

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose, including
    commercial applications, and to alter it and redistribute it freely, subject to
    the following restrictions:

    1. The origin of this software must not be misrepresented; you must not claim
       that you wrote the original software. If you use this software in a product,
       an acknowledgment in the product documentation would be appreciated but is
       not required.

    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/
