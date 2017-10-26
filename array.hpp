#ifndef MUSH_MARRAY_HEADER
#define MUSH_MARRAY_HEADER

#include <iterator>
#include <algorithm>

namespace mush
{
    template<typename... Sizes>
    constexpr inline size_t get_marray_size(Sizes... dim)
    {
        return (dim * ...) == 0 ? 1 : (dim * ...);
    }

    template<typename T, size_t... Dimensions>
    struct Array
    {
        typedef T                                       value_type;
        typedef T&                                      reference;
        typedef const T&                                const_reference;
        typedef size_t                                  size_type;
        typedef ptrdiff_t                               difference_type;
        typedef T*                                      pointer;
        typedef const T*                                const_pointer;
        typedef T*                                      iterator;
        typedef const T*                                const_iterator;
        typedef std::reverse_iterator<iterator>         reverse_iterator;
        typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

        template <size_type Depth>
        struct array_access_proxy
        {
            Array&      ref;
            size_type   cindex;

            constexpr explicit array_access_proxy(Array& arr, size_t cindex) : ref(arr), cindex(cindex) {}

            constexpr auto operator[](size_type index)
            {
                if constexpr (Depth == sizeof...(Dimensions))
                {
                    return cindex * ref.dims[Depth - 1] + index;
                }
                else
                {
                    return array_access_proxy<Depth+1>(ref, index + (cindex * ref.dims[Depth-1]));
                }
            }
        };

        value_type      element_container[get_marray_size(Dimensions...)];
        const size_type dims[sizeof...(Dimensions)];

        Array() : dims{ Dimensions... } {}

        template<typename... Args>
        Array(Args... args) : element_container{ args... }, dims{ Dimensions... } {}

        void fill(const value_type& f)
        { std::fill_n(element_container, get_marray_size(Dimensions...), f); }

        void swap(Array& other)
        { std::swap(*this, other); }

        // iterators
        constexpr iterator begin() noexcept { return iterator(element_container); }
        constexpr iterator end() noexcept
        { return iterator(element_container + get_marray_size(Dimensions...)); }

        constexpr const_iterator begin() const noexcept { return iterator(element_container); }
        constexpr const_iterator end() const noexcept
        { return iterator(element_container + get_marray_size(Dimensions...)); }

        constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
        constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
        constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
        constexpr reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

        constexpr const_iterator cbegin() const noexcept { return begin(); }
        constexpr const_iterator cend() const noexcept { return end(); }
        constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        constexpr const_reverse_iterator crend() const noexcept { return rend(); }

        // capacity
        constexpr size_type size() const noexcept { return get_marray_size(Dimensions...); }
        constexpr size_type max_size() const noexcept { return get_marray_size(Dimensions...); }
        constexpr bool empty() const noexcept { return false; }

        // element access
        template <size_type Depth = 1>
        constexpr auto operator[](size_type index)
        {
            if constexpr (Depth == sizeof...(Dimensions))
                return reference(element_container[index]);
            else
                return array_access_proxy<Depth+1>(*this, index);
        }
        constexpr reference front() { return element_container[0]; }
        constexpr const_reference front() const { return element_container[0]; }
        constexpr reference back()
        { 
            return element_container[get_marray_size(Dimensions...) > 0 ? 
                get_marray_size(Dimensions...) - 1 : 0
            ];
        }
        constexpr const_reference back() const
        { 
            return element_container[get_marray_size(Dimensions...) > 0 ? 
                get_marray_size(Dimensions...) - 1 : 0
            ];
        }

        constexpr pointer data() noexcept { return element_container; }
        constexpr const pointer data() const noexcept { return element_container; }
    };
}

#endif
