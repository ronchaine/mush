#ifndef MUSH_EXTRA_GFX_SPRITESHEET
#define MUSH_EXTRA_GFX_SPRITESHEET

#include <cstdint>

#include "../../string.hpp"
#include "../../rectpack.hpp"
#include "texture.hpp"

#include <unordered_map>

namespace mush::extra::opengl
{
    struct SpriteInfo
    {
        mush::Rectangle r;
        std::tuple<uint16_t, uint16_t, uint16_t, uint16_t> uv;
    };

    template <uint32_t Channels = 4>
    class SpriteSheet
    {
        private:
            Texture         texture;
            RectanglePack   atlas;

            std::unordered_map<mush::string, mush::Rectangle> stored;

        public:
            SpriteSheet(uint32_t w = 1024, uint32_t h = 1024)
            {
                texture.init(w, h, Channels);
                atlas.width = w;
                atlas.height = h;
                atlas.reset();
            }

            uint32_t width()
            {
                return atlas.width;
            }

            uint32_t height()
            {
                return atlas.height;
            }

            size_t size()
            {
                return stored.size();
            }

            SpriteInfo operator[](const mush::string& name)
            {
                SpriteInfo rval;
                rval.r = get_rect(name);
                rval.uv = get_uv(name);
                return rval;
            }

            mush::Rectangle get_rect(const mush::string& name)
            {
                mush::Rectangle rval;
                if (stored.count(name) != 0)
                    rval = stored[name];

                return rval;
            }

            auto get_uv(const mush::string& name, uint32_t flags = 0)
            {
                uint16_t left;
                uint16_t right;
                uint16_t top;
                uint16_t bottom;

                mush::Rectangle r = get_rect(name);
                
                left    = ((float)(r.x / (float)width())) * 0xffff;
                right   = ((float)((r.x + r.w) / (float)width())) * 0xffff;
                top     = ((float)(r.y / (float)height())) * 0xffff;
                bottom  = ((float)((r.y + r.h) / (float)height())) * 0xffff;

                // this fails with std::tie in g++, but not in clang, why?
                return std::make_tuple(left, right, top, bottom);
            }

            void add(const mush::string& name, void* data, uint32_t w, uint32_t h, uint32_t ch = 4)
            {
                mush::Rectangle r = atlas.fit(w, h);
                if (r == mush::Rectangle{0,0,0,0})
                {
                    grow(w+1,h+1);
                    r = atlas.fit(w,h);
                    if (r == mush::Rectangle{0,0,0,0})
                        assert(0 && "image atlas out of memory");
                }   

                atlas.prune(r);
                stored[name] = r;

                texture.update(data, r.x, r.y, r.w, r.h);
            }

            bool has(const mush::string& name)
            {
                return (stored.count(name) != 0);
            }

            void bind(GLint texture_unit)
            {
                texture.bind(texture_unit);
            }

            void grow(uint32_t dw, uint32_t dh)
            {
                RectanglePack resized;

                resized.width = atlas.width + dw;
                resized.height = atlas.height + dh;
                resized.reset();

                // keep old mappings
                resized.mapped = atlas.mapped;

                for (auto entry : resized.mapped)
                {
                    resized.split(entry); // make new splits according to what we have
                    resized.prune(entry); // and remove what we don't need
                }

                texture.grow(resized.width, resized.height);

                atlas = resized;
            }
    };
}
#endif
