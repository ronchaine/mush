#ifndef MUSH_SHAPES
#define MUSH_SHAPES

#include <cstdint>
#include <tuple>

template<typename T>
constexpr bool dependent_false()
{
    return false;
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
