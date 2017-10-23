#ifndef MUSH_GEOMETRIC_TYPES_HEADER
#define MUSH_GEOMETRIC_TYPES_HEADER

#include <cstdint>
#include <tuple>
#include <iostream>

namespace mush
{
    // I don't want this piece of code to depend on any header,
    // so I just duplicate it.
    #ifndef MUSH_DEPENDENT_FALSE
    #define MUSH_DEPENDENT_FALSE
    template<typename T> struct dependent_false
    {
        constexpr static bool value = false;
        bool operator()() { return false; }
    };
    #endif
}

namespace mush
{
    /** 
     * @brief Point in 3D-space
     */
    template <typename T = int32_t>
    class Point : public std::tuple<T, T, T>
    {
        public:
            using std::tuple<T,T,T>::tuple;

            Point<T> operator+(std::tuple<T,T,T> other) const
            {
                T a = std::get<0>(*this) + std::get<0>(other);
                T b = std::get<1>(*this) + std::get<1>(other);
                T c = std::get<2>(*this) + std::get<2>(other);

                return std::make_tuple(a,b,c);
            }

            Point<T> operator-(std::tuple<T,T,T> other) const
            {
                T a = std::get<0>(*this) - std::get<0>(other);
                T b = std::get<1>(*this) - std::get<1>(other);
                T c = std::get<2>(*this) - std::get<2>(other);

                return std::make_tuple(a,b,c);
            }

            Point<T> operator*(std::tuple<T,T,T> other) const
            {
                T a = std::get<0>(*this) * std::get<0>(other);
                T b = std::get<1>(*this) * std::get<1>(other);
                T c = std::get<2>(*this) * std::get<2>(other);

                return std::make_tuple(a,b,c);
            }
            
            Point<T> operator/(std::tuple<T,T,T> other) const
            {
                T a = std::get<0>(*this) / std::get<0>(other);
                T b = std::get<1>(*this) / std::get<1>(other);
                T c = std::get<2>(*this) / std::get<2>(other);

                return std::make_tuple(a,b,c);
            }

            const std::string to_str()
            {
                T a = std::get<0>(*this);
                T b = std::get<1>(*this);
                T c = std::get<2>(*this);

                std::string rval = "(";

                rval += std::to_string(a);
                rval += ", ";
                rval += std::to_string(b);
                rval += ", ";
                rval += std::to_string(c);
                rval += ")";

                return rval;
            }
    };

    /** 
     * @brief two-dimensional rectangle
     */
    struct Rectangle
    {
        int32_t  x, y;
        uint32_t w, h;

        // required for sorting
        bool operator<(const Rectangle& r) const
        {
            return w*h < r.w * r.h;
        }
        bool operator==(const Rectangle& r) const
        {
            return ((w == r.w) && (h == r.h) && (x == r.x) && (y == r.y));
        }

        Rectangle() : x(~0), y(~0), w(~0), h(~0) {}

        Rectangle(std::initializer_list<int64_t> list)
        {
            if (list.size() < 4)
                return;

            x = *list.begin();
            y = *(list.begin()+1);
            w = *(list.begin()+2);
            h = *(list.begin()+3);
        }
        
        friend inline std::ostream& operator<<(std::ostream& out, const Rectangle& val)
        {
            out << "(" << val.x << "," << val.y << ";" << val.w << "," << val.h << ")";
            return out;
        }
    };
    
    // type traits
    template <typename T> struct is_shape { static constexpr bool value = false; };

    // point is kinda shape
    template <typename T> struct is_shape<Point<T>> { static constexpr bool value = true; };
    template <typename T> struct is_shape<const Point<T>> { static constexpr bool value = true; };
    template <typename T> struct is_shape<Point<T>&> { static constexpr bool value = true; };
    template <typename T> struct is_shape<const Point<T>&> { static constexpr bool value = true; };

    // rectangles are shapes
    template <> struct is_shape<Rectangle> { static constexpr bool value = true;  };
    template <> struct is_shape<Rectangle&> { static constexpr bool value = true;  };
    template <> struct is_shape<const Rectangle> { static constexpr bool value = true;  };
    template <> struct is_shape<const Rectangle&> { static constexpr bool value = true;  };

    // concept for shapes
    template <typename T> concept bool Shape = is_shape<T>::value;

    // ----------------------------------------
    //            OVERLAP TESTS
    // ----------------------------------------

    // make compile-time error if trying to use nonexistant overlap test
    template<Shape First, Shape Second>
    inline bool overlap(First shape, Second shape2)
    {
        static_assert(dependent_false<First>(), "overlap check for shapes requested not implemented");
        return false;
    }

    // need shorthand for taking in const reference
    template <typename T>
    struct as_cref { typedef typename std::add_lvalue_reference<typename std::add_const<T>::type>::type type; };

    // rectangle-rectangle overlap
    inline bool overlap(typename as_cref<Rectangle>::type r1, typename as_cref<Rectangle>::type r2)
    {
        int b1, b2, rg1, rg2;
        b1 = r1.y + r1.h;
        b2 = r2.y + r2.h;

        rg1 = r1.x + r1.w;
        rg2 = r2.x + r2.w;

        if (( b1 > r2.y ) && ( r1.y < b2 ) && ( rg1 > r2.x ) && ( rg2 > r1.x ))
            return true;

        return false;
    }

    // ----------------------------------------
    //          CONTAINMENT TESTS
    // ----------------------------------------

    // make compile-time error if trying to use nonexistant containment test
    template<Shape First, Shape Second>
    inline bool contains(First shape, Second shape2)
    {
        static_assert(dependent_false<First>(), "containment check for shapes requested not implemented");
        return false;
    }
 
    // rectangle containment, true iff first inside second
    inline bool contains(typename as_cref<Rectangle>::type r2, typename as_cref<Rectangle>::type r1)
    {
        if ((r1.x >= r2.x) && (r1.x + r1.w <= r2.x + r2.w) && (r1.y >= r2.y) && (r1.y + r1.h <= r2.y + r2.h))
            return true;

        return false;
    }
}

#endif
/*
 Copyright (c) 2017 Jari Ronkainen

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
