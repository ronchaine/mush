#ifndef MUSH_BUILDING_BLOCKS
#define MUSH_BUILDING_BLOCKS

#include <memory>

namespace mush
{
    template<size_t Current, typename Container, size_t Dimension>
    struct Access_Proxy
    {
        Container&                              ref;
        size_t                                  index_sum;
        size_t                                  dims[Dimension];

        typedef typename Container::value_type  inner_type;
        typedef inner_type&                     rval_reference;

        typedef Access_Proxy<Current + 1, Container, Dimension> next_level_type;

        // The enable_if is on first argument is a hack to allow mixing
        // between sizeof..., constuctors and enable if
        template <typename... Args>
        constexpr Access_Proxy(typename std::enable_if<sizeof...(Args) != 1, Container&>::type container,
                               size_t sum, Args... args)
            : ref(container), index_sum(sum), dims{args...} {}

        // This constructor is plain evil
        constexpr Access_Proxy(Container& container, size_t sum, size_t* in_dims)
            : ref(container), index_sum(sum)
        {
            for (size_t i = 0; i < Dimension; ++i)
                dims[i] = *(in_dims + i);
        } 

        // It would be really neat if this could be done by just if
        // constexpr and auto, but auto doesn't handle references
        template <bool Last = (Current == Dimension), typename = std::enable_if_t<!Last>>
        constexpr auto operator[](size_t index)
        {
            return Access_Proxy<Current + 1, Container, Dimension>(ref, index_sum * dims[Current-1] + index, dims);
        }

        // So we need the last in line to return a reference
        template <bool Last = (Current == Dimension), typename = std::enable_if_t<Last>>
        constexpr auto& operator[](size_t index)
        {
            return ref[index_sum * dims[Current-1] + index];
        }
    };

    template<size_t Current, typename Container>
    struct Access_Proxy<Current, Container, 1> { Access_Proxy() = delete; };
}

#endif
