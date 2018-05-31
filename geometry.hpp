#ifndef MUSH_GEOMETRIC_TYPES_HEADER
#define MUSH_GEOMETRIC_TYPES_HEADER

#include <cstdint>
#include <tuple>
#include <iostream>

#include "core.hpp"

namespace mush
{
    //! 3D Size type
    template <typename T = uint32_t>
    struct Size3D
    {
        using value_type = T;
        
        T width, height, depth;

        Size3D() = default;
        constexpr Size3D(T w, T h, T d) : width(w), height(h), depth(d) {}
        constexpr Size3D(std::tuple<T,T,T> in) : width(std::get<0>(in)),
                                                 height(std::get<1>(in)),
                                                 depth(std::get<2>(in)) {}

        constexpr T total() const { return width * height * depth; }

        operator std::tuple<T,T,T>() const { return std::tuple(width,height,depth); }
    };

    using Size3Df = Size3D<float>;
    using Size3Di = Size3D<uint32_t>;

    //! 2D Size type
    template <typename T = uint32_t>
    struct Size2D
    {
        using value_type = T;

        T width, height;

        Size2D() = default;
        constexpr Size2D(T w, T h) : width(w), height(h) {}
        constexpr Size2D(std::tuple<T,T> in) : width(std::get<0>(in)), height(std::get<1>(in)) {}
        
        operator std::tuple<T,T>() const { return std::tuple(width,height); }
    };
    
    using Size2Df = Size2D<float>;
    using Size2Di = Size2D<uint32_t>;

    //! Point in 3D-space
    template <typename T = int32_t>
    class Point3D : public std::tuple<T, T, T>
    {
        public:
            using std::tuple<T,T,T>::tuple;

            Point3D<T> operator+(std::tuple<T,T,T> other) const
            {
                T a = std::get<0>(*this) + std::get<0>(other);
                T b = std::get<1>(*this) + std::get<1>(other);
                T c = std::get<2>(*this) + std::get<2>(other);

                return std::make_tuple(a,b,c);
            }

            Point3D<T> operator-(std::tuple<T,T,T> other) const
            {
                T a = std::get<0>(*this) - std::get<0>(other);
                T b = std::get<1>(*this) - std::get<1>(other);
                T c = std::get<2>(*this) - std::get<2>(other);

                return std::make_tuple(a,b,c);
            }

            Point3D<T> operator*(std::tuple<T,T,T> other) const
            {
                T a = std::get<0>(*this) * std::get<0>(other);
                T b = std::get<1>(*this) * std::get<1>(other);
                T c = std::get<2>(*this) * std::get<2>(other);

                return std::make_tuple(a,b,c);
            }
            
            Point3D<T> operator/(std::tuple<T,T,T> other) const
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

    //! two-dimensional rectangle
    struct Rectangle
    {
        constexpr static uint32_t dimensions = 2;

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

        constexpr Rectangle() : x(~0), y(~0), w(~0), h(~0) {}

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

    using Dimension_Type = uint32_t;

    struct Vertex_Index_Triplet
    {
        uint32_t i0, i1, i2;

        Vertex_Index_Triplet() = default;
        Vertex_Index_Triplet(uint32_t v0, uint32_t v1, uint32_t v2) : i0(v0), i1(v1), i2(v2) {}
    };

    //! Shape base type
    template <uint32_t Vertex_Count,
              uint32_t Triangle_Count,
              typename Vertex_Type>
    struct Physical_Shape
    {
        constexpr static uint32_t vertex_count = Vertex_Count;
        constexpr static uint32_t triangle_count = Triangle_Count;

        std::array<Vertex_Type, Vertex_Count>                   vertices;
        const std::array<Vertex_Index_Triplet, Triangle_Count>  triangles;
    };

    template <typename Vertex_Type>
    struct Triangle : public Physical_Shape<3, 1, Vertex_Type>
    {
        Triangle()
        {
            *this.triangles[0] = {0, 1, 2};
        }
    };

    template <typename Vertex_Type>
    struct Quad : public Physical_Shape<4, 2, Vertex_Type>
    {
        constexpr static uint8_t TOP_LEFT      = 0;
        constexpr static uint8_t TOP_RIGHT     = 1;
        constexpr static uint8_t BOTTOM_LEFT   = 2;
        constexpr static uint8_t BOTTOM_RIGHT  = 3;

        Quad()
        {
            *this.triangles[0] = {TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT};
            *this.triangles[1] = {BOTTOM_RIGHT, BOTTOM_LEFT, TOP_RIGHT};
        }

        Quad(Vertex_Type tl,
             Vertex_Type tr,
             Vertex_Type bl,
             Vertex_Type br) : Quad()
        {
            *this.vertices[0] = tl;
            *this.vertices[1] = tr;
            *this.vertices[2] = bl;
            *this.vertices[3] = br;
        }
    };

    template <typename Vertex_Type>
    struct Box : public Physical_Shape<8, 12, Vertex_Type>
    {
        constexpr static uint16_t FRONT_TOP_LEFT        = 0;
        constexpr static uint16_t FRONT_TOP_RIGHT       = 1;
        constexpr static uint16_t FRONT_BOTTOM_LEFT     = 2;
        constexpr static uint16_t FRONT_BOTTOM_RIGHT    = 3;
        constexpr static uint16_t BACK_TOP_LEFT         = 4;
        constexpr static uint16_t BACK_TOP_RIGHT        = 5;
        constexpr static uint16_t BACK_BOTTOM_LEFT      = 6;
        constexpr static uint16_t BACK_BOTTOM_RIGHT     = 7;

        bool axis_aligned = false;

        constexpr Box()
        {
            // *this is required because we're inheriting from a template
            // top face
            *this.triangles[0] = {FRONT_TOP_LEFT, FRONT_TOP_RIGHT, BACK_TOP_LEFT};
            *this.triangles[1] = {BACK_TOP_LEFT, FRONT_TOP_RIGHT, BACK_TOP_RIGHT};
            // front face
            *this.triangles[2] = {FRONT_BOTTOM_LEFT, FRONT_TOP_RIGHT, FRONT_TOP_LEFT};
            *this.triangles[3] = {FRONT_BOTTOM_LEFT, FRONT_BOTTOM_RIGHT, FRONT_TOP_RIGHT};
            // left face
            *this.triangles[4] = {BACK_BOTTOM_LEFT, FRONT_TOP_LEFT, BACK_TOP_LEFT};
            *this.triangles[5] = {BACK_BOTTOM_LEFT, FRONT_BOTTOM_LEFT, FRONT_TOP_LEFT};
            // bottom face
            *this.triangles[6] = {FRONT_BOTTOM_RIGHT, FRONT_BOTTOM_LEFT, BACK_BOTTOM_LEFT};
            *this.triangles[7] = {FRONT_BOTTOM_RIGHT, BACK_BOTTOM_LEFT, BACK_BOTTOM_RIGHT};
            // back face
            *this.triangles[8] = {BACK_TOP_LEFT, BACK_TOP_RIGHT, BACK_BOTTOM_RIGHT};
            *this.triangles[9] = {BACK_TOP_LEFT, BACK_BOTTOM_LEFT, BACK_BOTTOM_RIGHT};
            // right face
            *this.triangles[10] = {BACK_BOTTOM_RIGHT, BACK_TOP_RIGHT, FRONT_TOP_RIGHT};
            *this.triangles[11] = {BACK_BOTTOM_RIGHT, FRONT_TOP_RIGHT, FRONT_BOTTOM_RIGHT};
        }

        constexpr Box(Vertex_Type ftl,
            Vertex_Type ftr,
            Vertex_Type fbl,
            Vertex_Type fbr,
            Vertex_Type btl,
            Vertex_Type btr,
            Vertex_Type bbl,
            Vertex_Type bbr) : Box()
        {
            *this.vertices[FRONT_TOP_LEFT]      = ftl;
            *this.vertices[FRONT_TOP_RIGHT]     = ftr;
            *this.vertices[FRONT_BOTTOM_LEFT]   = fbl;
            *this.vertices[FRONT_BOTTOM_RIGHT]  = fbr;
            *this.vertices[BACK_TOP_LEFT]       = btl;
            *this.vertices[BACK_TOP_RIGHT]      = btr;
            *this.vertices[BACK_BOTTOM_LEFT]    = bbl;
            *this.vertices[BACK_BOTTOM_RIGHT]   = bbr;
        }

        constexpr Box(Vertex_Type first_corner, Vertex_Type second_corner)
        {
            *this.vertices[FRONT_TOP_LEFT] = first_corner;
            *this.vertices[BACK_BOTTOM_RIGHT] = second_corner;
        }
    };

    // type traits, for starters, anything descended from Physical_Shape is a shape
    template <typename T> struct is_shape
    { static constexpr bool value = std::is_base_of_v<Physical_Shape, T>; };

    // point is kinda shape
    template <typename T> struct is_shape<Point3D<T>> { static constexpr bool value = true; };
    template <typename T> struct is_shape<const Point3D<T>> { static constexpr bool value = true; };
    template <typename T> struct is_shape<Point3D<T>&> { static constexpr bool value = true; };
    template <typename T> struct is_shape<const Point3D<T>&> { static constexpr bool value = true; };

    // rectangles are shapes
    template <> struct is_shape<Rectangle> { static constexpr bool value = true;  };
    template <> struct is_shape<Rectangle&> { static constexpr bool value = true;  };
    template <> struct is_shape<const Rectangle> { static constexpr bool value = true;  };
    template <> struct is_shape<const Rectangle&> { static constexpr bool value = true;  };

    // concept for shapes
    #ifndef NO_CONCEPTS
    template <typename T> concept bool Shape = is_shape<T>::value;
    #else
    #define Shape typename
    #endif

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
