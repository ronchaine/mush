#ifndef MUSH_EXTRA_GFX_DRAW_HEADER
#define MUSH_EXTRA_GFX_DRAW_HEADER

#include "vertex.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "spritesheet.hpp"
#include "renderutils.hpp"

#include "../deps/stb_image.h"

namespace mush::extra::opengl
{
    // Loaders
    inline Texture file_to_texture(const char* file)
    {
        Texture rval;
        int w, h, comp;
        uint8_t* data = stbi_load(file, &w, &h, &comp, 0);

        assert(comp == 4);

        if (!data)
        {
            std::cout << "failed to load image '" << file << "'.\n";
            std::abort();
        }

        rval.create((void*)data, w, h, comp);

        stbi_image_free(data);
        return rval;
    }

    template<uint32_t Channels>
    void file_to_spritesheet(SpriteSheet<Channels>& sheet, const char* file)
    {
        int w, h, comp;
        uint8_t* data = stbi_load(file, &w, &h, &comp, 0);

        if (data == nullptr)
            return;

        assert (comp == Channels);

        if (!data)
        {
            std::cout << "failed to load image '" << file << "'.\n";
            std::abort();
        }

        sheet.add(file, (void*)data, w, h);

        stbi_image_free(data);
        return;
    }

    namespace draw
    {
        template <AnyVertexType VT>
        void rectangle(VertexBuffer<VT>& buf,
                       int32_t x,
                       int32_t y,
                       uint32_t w,
                       uint32_t h,
                       uint16_t left,
                       uint16_t right,
                       uint16_t top,
                       uint16_t bottom,
                       uint32_t c0,
                       uint32_t c1,
                       uint32_t c2,
                       uint32_t c3)
        {
            const float x_unit = 2.0f / (float)Screen_Info<0>::width;
            const float y_unit = 2.0f / (float)Screen_Info<0>::height;

            VT v0, v1, v2, v3;
    
            float leftv, rightv, topv, bottomv;

            leftv   = -1.0f + x_unit * (x);
            rightv  = -1.0f + x_unit * (x + w);
            bottomv = -1.0f + y_unit * (Screen_Info<0>::height - y);
            topv    = -1.0f + y_unit * (Screen_Info<0>::height - (y + h));

            // set positions
            v0.x = leftv;
            v0.y = bottomv;
            v1.x = rightv;
            v1.y = v0.y;
            v2.x = v1.x;
            v2.y = topv;
            v3.x = v0.x;
            v3.y = v2.y;

            if constexpr (VT::uv_count == 1)
            {
                v0.u = left;
                v0.v = bottom;
                v1.u = right;
                v1.v = bottom;
                v2.u = right;
                v2.v = top;
                v3.u = left;
                v3.v = top;
            }

            if constexpr (VT::has_rgba)
            {
                v0.a = (0x000000ff & c0);
                v0.b = (0x0000ff00 & c0) >> 8;
                v0.g = (0x00ff0000 & c0) >> 16;
                v0.r = (0xff000000 & c0) >> 24;

                v1.a = (0x000000ff & c1);
                v1.b = (0x0000ff00 & c1) >> 8;
                v1.g = (0x00ff0000 & c1) >> 16;
                v1.r = (0xff000000 & c1) >> 24;

                v2.a = (0x000000ff & c2);
                v2.b = (0x0000ff00 & c2) >> 8;
                v2.g = (0x00ff0000 & c2) >> 16;
                v2.r = (0xff000000 & c2) >> 24;

                v3.a = (0x000000ff & c3);
                v3.b = (0x0000ff00 & c3) >> 8;
                v3.g = (0x00ff0000 & c3) >> 16;
                v3.r = (0xff000000 & c3) >> 24;
            }

            buf.add_quad(v0, v1, v2, v3);
        }
        
        template <AnyVertexType VT>
        void rectangle(VertexBuffer<VT>& buf, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t c0, uint32_t c1, uint32_t c2, uint32_t c3)
        {
            rectangle(buf, x, y, w, h, 0x0000, 0xffff, 0xffff, 0x0000, c0, c1, c2, c3);
        }
        template <AnyVertexType VT>
        void rectangle(VertexBuffer<VT>& buf, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t c0, uint32_t c1)
        {
            rectangle(buf, x, y, w, h, c0, c0, c1, c1);
        }
        template <AnyVertexType VT>
        void rectangle(VertexBuffer<VT>& buf, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t c0 = 0xffffffff)
        {
            rectangle(buf, x, y, w, h, c0, c0, c0, c0);
        }
        template <AnyVertexType VT>
        void sprite(VertexBuffer<VT>& buf,
                    const SpriteInfo& sprite,
                    int32_t x,
                    int32_t y,
                    uint32_t c0,
                    uint32_t c1,
                    uint32_t c2,
                    uint32_t c3)
        {
            rectangle(buf, x, y, sprite.r.w, sprite.r.h, 
                      std::get<0>(sprite.uv),
                      std::get<1>(sprite.uv),
                      std::get<3>(sprite.uv),
                      std::get<2>(sprite.uv),
                      c0,c1,c2,c3
                     );
        }
        template <AnyVertexType VT>
        void sprite(VertexBuffer<VT>& buf,
                    const SpriteInfo& sprite,
                    int32_t x,
                    int32_t y)
        {
            rectangle(buf, x, y, sprite.r.w, sprite.r.h, 
                      std::get<0>(sprite.uv),
                      std::get<1>(sprite.uv),
                      std::get<3>(sprite.uv),
                      std::get<2>(sprite.uv),
                      0xffffffff,
                      0xffffffff,
                      0xffffffff,
                      0xffffffff
                     );
        }
        template <AnyVertexType VT>
        void sprite(VertexBuffer<VT>& buf,
                    const SpriteInfo& sprite,
                    int32_t x,
                    int32_t y,
                    uint32_t colour)
        {
            rectangle(buf, x, y, sprite.r.w, sprite.r.h, 
                      std::get<0>(sprite.uv),
                      std::get<1>(sprite.uv),
                      std::get<3>(sprite.uv),
                      std::get<2>(sprite.uv),
                      colour,
                      colour,
                      colour,
                      colour
                     );
        }
        
        template <AnyVertexType VT>
        void sprite(VertexBuffer<VT>& buf,
                    const SpriteInfo& sprite,
                    int32_t x,
                    int32_t y,
                    uint32_t w,
                    uint32_t h,
                    uint32_t colour)
        {
            rectangle(buf, x, y,
                      w,
                      h,
                      std::get<0>(sprite.uv),
                      std::get<1>(sprite.uv),
                      std::get<3>(sprite.uv),
                      std::get<2>(sprite.uv),
                      colour,
                      colour,
                      colour,
                      colour
                     );
        }

        template <AnyVertexType VT>
        void scaled_sprite(VertexBuffer<VT>& buf,
                    const SpriteInfo& sprite,
                    float scale,
                    int32_t x,
                    int32_t y,
                    uint32_t colour)
        {
            rectangle(buf, x, y,
                      sprite.r.w * scale,
                      sprite.r.h * scale,
                      std::get<0>(sprite.uv),
                      std::get<1>(sprite.uv),
                      std::get<3>(sprite.uv),
                      std::get<2>(sprite.uv),
                      colour,
                      colour,
                      colour,
                      colour
                     );
        }

    } // namespace draw    
}

#endif
