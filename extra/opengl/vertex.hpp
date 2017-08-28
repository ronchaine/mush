#ifndef MUSH_EXTRA_GFX_VERTEX
#define MUSH_EXTRA_GFX_VERTEX

#include <cstdint>
#include <type_traits>
#include <ostream>

#include "../../buffer.hpp"

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

namespace mush::extra::opengl
{
    using VertexTypeCode = uint32_t;
    using VertexTypeFlags = uint32_t;

    constexpr static VertexTypeFlags VERTEX_RGBA_COLOUR = 0x10;
    constexpr static VertexTypeFlags VERTEX_HSV_SHIFT   = 0x20;

    // Vertex position chunk
    template <uint32_t Pos_Components> struct VertexPositionBase
    {
        static_assert(dependent_false<decltype(Pos_Components)>::value && "Vertex Pos_Components argument must be 0-4");
    };
    template <> struct VertexPositionBase<0> {};
    template <> struct VertexPositionBase<1> { float x; };
    template <> struct VertexPositionBase<2> { float x, y; };
    template <> struct VertexPositionBase<3> { float x, y, z; };
    template <> struct VertexPositionBase<4> { float x, y, z, w; };
    
    // Vertex texcoord chunk
    template <uint32_t UV_Count> struct VertexTexCoordBase
    {
        uint16_t u[UV_Count];
        uint16_t v[UV_Count];
    };
    template <> struct VertexTexCoordBase<1>
    {
        uint16_t u;
        uint16_t v;
    };
    template <> struct VertexTexCoordBase<0> {};

    // Vertex colour chunk
    struct VertexRGBAColour
    {
        uint8_t r = 0xff, g = 0xff, b = 0xff, a = 0xff;

        void set_colour(uint32_t rgba)
        {
            r = rgba >> 24;
            g = (0x00ff0000 & rgba) >> 16;
            b = (0x0000ff00 & rgba) >> 8;
            a = 0xff & rgba;
        }
    };
    struct VertexHSVShift { float hue = 0.0f, sat = 1.0f, val = 1.0f; };

    template<typename T> struct Empty {};

    template <VertexTypeFlags Flags> struct VertexFlagsBase
        : std::conditional<Flags & VERTEX_RGBA_COLOUR,  VertexRGBAColour, Empty<VertexRGBAColour>>::type,
          std::conditional<Flags & VERTEX_HSV_SHIFT,    VertexHSVShift, Empty<VertexHSVShift>>::type
    {};

    template <uint32_t Pos_Components, uint32_t UV_Count, VertexTypeFlags Flags = 0>
    class VertexType : public VertexPositionBase<Pos_Components>,
                       public VertexTexCoordBase<UV_Count>,
                       public VertexFlagsBase<Flags>
    {
        public:
            constexpr static uint32_t   dim         = Pos_Components;
            constexpr static uint32_t   uv_count    = UV_Count;
            constexpr static bool       has_rgba    = Flags & VERTEX_RGBA_COLOUR;
            constexpr static bool       has_hsv     = Flags & VERTEX_HSV_SHIFT;

            constexpr static VertexTypeFlags flags  = Flags;

            VertexType() {}

            friend inline std::ostream& operator<<(std::ostream& out, const VertexType& val)
            {
                out << "(";
                if constexpr (Pos_Components >= 1) out << val.x;
                if constexpr (Pos_Components >= 2) out << "," << val.y;
                if constexpr (Pos_Components >= 3) out << "," << val.z;
                if constexpr (Pos_Components == 4) out << "," << val.w;
                out << ")";

                if constexpr (UV_Count == 1) out << " UV:" << val.u << "," << val.v;
                if constexpr (UV_Count >= 2)
                {
                    out << "\n";
                    for (uint32_t i = 0; i < val.uv_count; ++i)
                    {
                        out << " UV" << i << ":" << val.u[i] << "," << val.v[i];
                    }
                    out << "\n";
                }

                if constexpr (has_rgba)
                {
                    uint32_t colour = ((uint32_t)val.r << 24)
                                    + ((uint32_t)val.g << 16)
                                    + ((uint32_t)val.b << 8)
                                    + (uint32_t)val.a;

                    out << "rgba" << colour;
                }

                return out;
            }
    };

    template <typename T>
    concept bool AnyVertexType = requires(T a)
    {
        {
            T::dim,
            T::uv_count,
            T::has_rgba,
            T::has_hsv
        }
    };

    template <AnyVertexType T>
    class VertexBuffer
    {
        private:
            mush::Buffer    vertex_data;

        public:
            typedef T       vertex_type;

            const void*     data() const { return &vertex_data[0]; }
            const size_t    size() const { return vertex_data.size(); }
            const uint32_t  count() const { return vertex_data.size() / sizeof(vertex_type); }

            void clear()
            {
                mush::Buffer new_data;
                vertex_data = new_data;
                assert(vertex_data.size() == 0);
            }

            void add_vertex(const vertex_type& v)
            {
                vertex_data.write(v);
            }

            void add_vertices(const vertex_type& v)
            {
                add_vertex(v);
            }

            template <typename... Vertices>
            void add_vertices(const vertex_type& v, Vertices... rest)
            {
                add_vertex(v);
                add_vertices(rest...);
            }

            void add_quad(const vertex_type& v0,
                          const vertex_type& v1,
                          const vertex_type& v2,
                          const vertex_type& v3)
            {
                add_vertices(v0, v1, v3, v1, v2, v3);
            }
    };
}

#endif
