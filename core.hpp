/*!
 * \file core.hpp
 * \brief Contains basic stuff that other headers are allowed to depend on
 * \author Jari Ronkainen
 * \version 1.0
 *
 */

#ifndef MUSH_CORE
#define MUSH_CORE

#include <type_traits>
#include <memory>

namespace mush
{
    using ColourFormat = uint32_t;

    constexpr ColourFormat RGBA             = 0x00;
    constexpr ColourFormat BGRA             = 0xa0;
    constexpr ColourFormat XYZ              = 0xa1;
    constexpr ColourFormat HSV              = 0xa2;
    constexpr ColourFormat ALPHA            = 0x01;
    constexpr ColourFormat PALETTE_ALPHA    = 0x02;
    constexpr ColourFormat UNKNOWN          = 0xfe;

    #ifndef NO_CONCEPTS
    template <typename T> concept bool PODType = std::is_pod<T>::value;
    
    template <typename T> concept bool ArithmeticType = std::is_arithmetic<T>::value;
    template <typename T> concept bool IntegralType = std::is_integral<T>::value;
    template <typename T> concept bool FloatingPointType = std::is_floating_point<T>::value;
    template <typename T> concept bool SuitableBaseType = std::is_arithmetic<T>::value && (sizeof(T) >= 4);
    
    template <typename T>
    concept bool IntegerType = std::is_integral<T>::value 
                            && !std::is_same<T,char32_t>::value
                            && !std::is_same<T,char16_t>::value
                            && !std::is_same<T,char>::value;
    
    template <typename T>
    concept bool FloatingType = std::is_floating_point<T>::value;

    template <typename T>
    concept bool BackInsertable = requires(T a)
    {
        { a.push_back() }
    };
    #endif

    template <typename T>
    struct wrap_reverse
    {
        T& iterable;
    };

    template <typename T>
    auto begin (wrap_reverse<T> w) { return std::rbegin(w.iterable); }
    
    template <typename T>
    auto end (wrap_reverse<T> w) { return std::rend(w.iterable); }

    template <typename T>
    wrap_reverse<T> reverse_adapter (T&& iterable) { return {iterable}; }

    #ifndef MUSH_DEPENDENT_FALSE
    #define MUSH_DEPENDENT_FALSE
    template<typename T> struct dependent_false
    {
        constexpr static bool value = false;
        constexpr bool operator()() { return false; }
    };
    #endif

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
