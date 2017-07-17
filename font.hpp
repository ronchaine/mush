// additional configuration defines:
//
// for header-only AND implementation (you should probably use compiler flag i.e. -DMUSH_FREETYPE_FONTS)
// MUSH_FREETYPE_FONTS          Add freetype2 support, requires freetype2
//
// for implementation only
// MUSH_FONTBUFFER_SIZE         Change width/height of font buffer, default 1024
// MUSH_FONTBUFFER_CHANNELS     Change number of channels in font buffer, default 4

#ifndef MUSH_FONT_HEADER
#define MUSH_FONT_HEADER

#ifdef MUSH_FREETYPE_FONTS
    #include <ft2build.h>
    #include FT_FREETYPE_H
    #include FT_LCD_FILTER_H
#endif
#ifdef MUSH_BITMAP_FONTS
#endif

#ifdef MUSH_MAKE_IMPLEMENTATIONS
#define MUSH_IMPLEMENT_FONT
#endif

#ifndef MUSH_FONTBUFFER_SIZE
#define MUSH_FONTBUFFER_SIZE 1024
#endif

#ifndef MUSH_FONTBUFFER_CHANNELS
#define MUSH_FONTBUFFER_CHANNELS 4
#endif

#include <cstdint>
#include <cassert>

#include <unordered_map>

#include "string.hpp"
#include "rectpack.hpp"

namespace mush
{
    // I wish there was opaque type for this
    using FontType = uint32_t;
    
    constexpr FontType UNKNOWN_FONT     = 0x00;
    constexpr FontType FREETYPE_FONT    = 0x01;
    constexpr FontType BITMAP_FONT      = 0x02;
    
    template <FontType T> struct dependent_false { static constexpr bool value = false; };

    struct GlyphMetrics
    {
        int32_t advance;
        int32_t vertical_advance;
        int32_t left;
        int32_t width;
        int32_t top;
        int32_t height;
    };

    class Font_Base
    {
        protected:
            Font_Base(const mush::string& prefix) : prefix(prefix) {}
            std::unordered_map<char32_t, GlyphMetrics> metrics;

        public:

            const mush::string prefix;
            
            virtual mush::Rectangle get_glyph(char32_t glyph) const;
            GlyphMetrics get_metrics(char32_t glyph)
            {
                if (metrics.count(glyph) == 0)
                {
                    GlyphMetrics empty;
                    return empty;
                }

                return metrics[glyph];
            }

            virtual float next_line() const { return 0; }

            inline void update_cache(const mush::string& name, uint32_t w, uint32_t h, void* data);

            inline size_t get_fontbuffer_size() const;
            inline size_t get_fontbuffer_channels() const;

            virtual void load_glyph(char32_t c) { return; }

            const void* data() const;

            virtual ~Font_Base() {}
    };

    template <FontType T>
    class Font : public Font_Base
    {
        static_assert(dependent_false<T>::value, "Unsupported font format, did you #define MUSH_BITMAP_FONTS or MUSH_FREETYPE_FONTS?");

        public:
            Font() : Font_Base("") {}
    };

    #ifdef MUSH_FREETYPE_FONTS
    template <>
    class Font<FREETYPE_FONT> : public Font_Base
    {
        private:
            static FT_Library library;
            static uint32_t l_count;

            const float line_spacing;

            FT_Face face;
        
        public:
            void load_glyph(char32_t c)
            {
                if (FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT))
                    return;

                GlyphMetrics m;
                // Fill in metrics TODO: Vertical Advance
                m.advance = face->glyph->advance.x >> 6;
                m.vertical_advance = 0;
                m.left = face->glyph->bitmap_left;
                m.top = face->glyph->bitmap_top;
                m.width = face->glyph->bitmap.width;
                m.height = face->glyph->bitmap.rows;

                metrics[c] = m;

                uint32_t ft_w, ft_h;
                
                // Flip the Y-axis.  Useful for OpenGL
                ft_w = face->glyph->bitmap.width;
                ft_h = face->glyph->bitmap.rows;

                uint8_t remap[ft_h][ft_w];

                for (uint32_t i = 0; i < ft_w; ++i) for (uint32_t j = 0; j < ft_h; ++j)
                    remap[ft_h - j - 1][i] = *(face->glyph->bitmap.buffer + j * ft_w + i);
                
                update_cache(prefix + c, ft_w, ft_h, remap);
            }

            Font(const mush::string& name,
                 const mush::Buffer& data,
                 uint32_t size,
                 float line_spacing = 1.0f)
            : Font_Base("freetype/" + name + "/" + size), line_spacing(line_spacing)
            {
                if (l_count == 0)
                {
                    if (FT_Init_FreeType(&library))
                        assert(0 && "FreeType Init failed");
                }

                // add to refcounter
                l_count++;

                if (FT_New_Memory_Face(library, &data[0], data.size(), 0, &face))
                    assert(0 && "Freetype error: couldn't create new face from memory buffer");

                FT_Select_Charmap(face, ft_encoding_unicode);
                FT_Set_Pixel_Sizes(face, 0, size);

                const mush::string precache = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz.,!?1234567890+-=:;";

                for (char32_t c : precache)
                    load_glyph(c);
            }

           ~Font()
            {
                FT_Done_Face(face);

                l_count--;
                if (l_count == 0)
                    FT_Done_FreeType(library);
            }

            float next_line() const
            {
                return line_spacing * face->height / 64.0f;
            }

            int32_t get_advance(char32_t glyph)
            {
                return face->glyph->advance.x >> 6;
            }
    
            mush::Rectangle glyph_metrics(char32_t glyph) const;
    };
    #endif
    
    #ifdef MUSH_BITMAP_FONTS
    template <>
    class Font<BITMAP_FONT> : public Font_Base
    {
        public:
            Font(const mush::string& name,
                 const mush::Buffer& data,
                 uint32_t size,
                 uint32_t spacelen,
                 uint32_t descent)
            : Font_Base("bitmap/" + name + "/" + size)
            {}
    };
    #endif

    /*===========================================================
     *
     *                      IMPLEMENTATION
     *
     *==========================================================*/

    #ifdef MUSH_IMPLEMENT_FONT

    #ifdef MUSH_FREETYPE_FONTS
    FT_Library Font<FREETYPE_FONT>::library;
    uint32_t Font<FREETYPE_FONT>::l_count;
            
    mush::Rectangle Font<FREETYPE_FONT>::glyph_metrics(char32_t glyph) const
    {
        mush::Rectangle rval;
        rval.x =  face->glyph->bitmap_left;
        rval.y = -face->glyph->bitmap.rows + face->glyph->bitmap_top;
        rval.w =  face->glyph->bitmap.width;
        rval.h =  face->glyph->bitmap.rows;
        return rval;
    }

    #endif

    // struct to hold font bitmap data and information about it
    struct font_info_t {
        uint8_t data[MUSH_FONTBUFFER_SIZE][MUSH_FONTBUFFER_SIZE][MUSH_FONTBUFFER_CHANNELS];

        RectanglePack atlas;
        std::unordered_map<mush::string, Rectangle> stored;

        font_info_t()
        {
            atlas.width = MUSH_FONTBUFFER_SIZE;
            atlas.height = MUSH_FONTBUFFER_SIZE;

            atlas.reset();
        }
    } font_info;
    //std::unique_ptr<font_info_t> font_info = std::make_unique<font_info_t>();

    // return position in the bitmap
    mush::Rectangle Font_Base::get_glyph(char32_t glyph) const
    {
        return font_info.stored[prefix + glyph];
    }
 
    inline size_t Font_Base::get_fontbuffer_size() const
    {
        return MUSH_FONTBUFFER_SIZE;
    }
    inline size_t Font_Base::get_fontbuffer_channels() const
    {
        return MUSH_FONTBUFFER_CHANNELS;
    }
            
    const void* Font_Base::data() const
    {
        return font_info.data;
    }

    // add to bitmap data
    inline void Font_Base::update_cache(const mush::string& name, uint32_t w, uint32_t h, void* data_ptr)
    {
        if (font_info.stored.count(name) != 0)
        {
//            std::cout << "trying to load multiple instances of '" << name << "'\n";
            return;
        }
        
        mush::Rectangle r = font_info.atlas.fit(w, h);

        if (r == mush::Rectangle{0,0,0,0})
            assert(0 && "font atlas out of memory");

        font_info.atlas.prune(r);
        font_info.stored[name] = r;

        if constexpr(MUSH_FONTBUFFER_CHANNELS == 4)
        {
            for (uint32_t xwr = 0; xwr < r.w; ++xwr) for (uint32_t ywr = 0; ywr < r.h; ++ywr)
            {
                font_info.data[r.y + ywr][r.x + xwr][0] = 0xff;
                font_info.data[r.y + ywr][r.x + xwr][1] = 0xff;
                font_info.data[r.y + ywr][r.x + xwr][2] = 0xff;
                font_info.data[r.y + ywr][r.x + xwr][3] = *((uint8_t*)(data_ptr) + xwr + ywr * r.w);
            }
        }
    }

    #endif
}

#endif
