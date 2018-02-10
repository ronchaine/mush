#ifndef MUSH_POOL_CONTAINER
#define MUSH_POOL_CONTAINER

#include <set>
#include <vector>

namespace mush
{
    struct BasePool {};

    //! Sequential container that reuses forgotten elements
    template <typename T>
    class Pool : public BasePool
    {
        friend class iterator;
// TODO:        friend class const_iterator;

        private:
            std::set<size_t>    used_indices;
            std::vector<T>      data;

        public:
            class iterator
            {
                Pool<T>& pool_;
                size_t ptr_;

                public:
                    typedef iterator                    self_type;
                    typedef T                           value_type;
                    typedef T&                          reference;
                    typedef T*                          pointer;
                    typedef std::forward_iterator_tag   iterator_category;
                    typedef int                         difference_type;

                    iterator(Pool<T> pool) : pool_(pool) {}
                    iterator(Pool<T> pool, size_t index) : pool_(pool), ptr_(index) {}

                    self_type operator++() // prefix
                    {
                        ptr_++;
                        while(pool_.data[ptr_].used_indices.count(ptr_)) ptr_++;
                        return *this;
                    }

                    self_type operator++(int) // postfix
                    {
                        self_type i = *this;
                        ptr_++;
                        while(pool_.data[ptr_].used_indices.count(ptr_)) ptr_++;
                        return i;
                    }

                    reference operator*() {
                        T& ref = pool_.data[ptr_];
                        return ref;
                    }

                    pointer operator->() { return &data[ptr_]; }

                    bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
                    bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }
            };

            iterator begin()    { return iterator(*this, 0); }
            iterator end()      { return iterator(*this, data.size()); }

            int32_t get_unused_index()
            {
                for (size_t i = 0; i < data.size(); ++i)
                    if (used_indices.count(i) == 0)
                        return i;

                data.resize(data.size() + 1);
                return data.size() - 1;
            }

            void set_used(size_t index) { used_indices.insert(index); }
            void unset_used(size_t index) { used_indices.erase(index); }

            T& at(const size_t index)
            {
                return data[index];
            }

            ~Pool()
            {
                while(data.size())
                    data.pop_back();
            }
    };
}

#endif
