#ifndef MUSH_FONT_HEADER
#define MUSH_FONT_HEADER

#ifdef MUSH_FREETYPE_FONTS
    #include <ft2build.h>
    #include FT_FREETYPE_H
    #include FT_LCD_FILTER_H
#endif

#ifdef MUSH_BITMAP_FONTS
#endif

#include <cstdint>
#include <unordered_map>

#include "string.hpp"
#include "buffer.hpp"

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
    // I wish there was opaque type for this
    using FontType = uint32_t;

    constexpr FontType UNKNOWN_FONT     = 0x00;
    constexpr FontType FREETYPE_FONT    = 0x01;
    constexpr FontType BITMAP_FONT      = 0x02;

    using BitmapFormat = uint32_t;

    constexpr BitmapFormat RGBA             = 0x00;
    constexpr BitmapFormat ALPHA            = 0x01;
    constexpr BitmapFormat PALETTE_ALPHA    = 0x02;

    struct GlyphMetrics
    {
        int32_t left;
        int32_t width;
        int32_t top;
        int32_t height;

        int32_t advance;
        int32_t vertical_advance;
    };

    struct Glyph
    {
        GlyphMetrics    metrics;
        Buffer          bitmap;
        BitmapFormat    format;
    };

    template <size_t Size, BitmapFormat Format, FontType T>
    class Font
    {
        #ifdef MUSH_FREETYPE_FONTS
            static FT_Library library;
            static uint32_t l_count;
            static FT_Face face;
        #endif

        protected:
            std::unordered_map<char32_t, Glyph> cache;
            mush::Buffer font_data;

            Font(const mush::string& in_prefix,
                 uint32_t in_size,
                 Buffer data,
                 mush::string load_chars
                )
            {
                // Common for all font types
                font_data = data;

                // Freetype Font specific
                if constexpr(T == FREETYPE_FONT)
                {
                    #ifdef MUSH_FREETYPE_FONTS
                    prefix = "freetype/" + in_prefix + "/" + in_size;

                    if (l_count == 0) if (FT_Init_FreeType(&library))
                            assert(0 && "FreeType init failed");

                    l_count++;

                    if (FT_New_Memory_Face(library, &font_data[0], data.size(), 0, &face))
                        assert(0 && "Freetype error: couldn't create new face from memory buffer");

                    FT_Select_Charmap(face, ft_encoding_unicode);
                    FT_Set_Pixel_Sizes(face, 0, Size);
        
                    line_spacing = (face->height >> 6);

                    FT_Load_Char(face, ' ', FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
                    space_length = face->glyph->advance.x >> 6;

                    for (char32_t c : load_chars)
                        add_glyph(c);
                    #else
                    static_assert(dependent_false<decltype(T)>(), "Freetype fonts not available, define MUSH_FREETYPE_FONTS?");
                    #endif
                }

                // Bitmap font specific
                else if constexpr (T == BITMAP_FONT)
                    #ifdef MUSH_FREETYPE_FONTS
                    prefix = "bitmap/" + in_prefix + "/" + in_size;
                    #else
                    static_assert(dependent_false<decltype(T)>(), "Bitmap fonts not available, define MUSH_BITMAP_FONTS?");
                    #endif
                else 
                    prefix = in_prefix;
            }

        public:
            const mush::string  prefix;

            int32_t             line_spacing;
            int32_t             space_length;

           ~Font()
            {
                if constexpr (T == FREETYPE_FONT)
                {
                    #ifdef MUSH_FREETYPE_FONTS
                    FT_Done_Face(face);
                    l_count--;

                    if (l_count == 0)
                        FT_Done_FreeType(library);
                    #endif
                }
            }

            Glyph glyph(char32_t c)
            {
                return cache[c];
            }

            bool has_glyph(char32_t c)
            {
                return (cache.count(c) != 0);
            }

            bool add_glyph(char32_t c)
            {
                if constexpr(T == FREETYPE_FONT)
                {
                    #ifdef MUSH_FREETYPE_FONTS
                    assert(face);
                    if (FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT))
                    {
                        std::cout << "Can't load glyph '" << mush::string(c) << "'!\n";
                        return false;
                    }

                    glyph[c].metrics.advance = face->glyph->advance.x >> 6;
                    glyph[c].vertical_advance = 0;
                    glyph[c].left = face->glyph->bitmap_left;
                    glyph[c].top = face->glyph->bitmap_top;
                    glyph[c].width = face->glyph->bitmap.width;
                    glyph[c].height = face->glyph->bitmap.rows;

                    return update_glyph_data(c, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);
                    #else
                    static_assert(dependent_false<decltype(T)>(), "Freetype fonts not available, define MUSH_FREETYPE_FONTS when building?");
                    #endif
                }
                else if constexpr(T == BITMAP_FONT)
                {
                    static_assert(dependent_false<decltype(T)>(), "Loading bitmap fonts not implemented");
                }
                else
                {
                    static_assert(dependent_false<decltype(T)>(), "Unknown font format");
                }
            }

            bool update_glyph_data(char32_t c, int w, int h, int ch, void* data)
            {
                if (ch == 0)
                    return false;
                if (w * h == 0)
                    return false;
                if (cache.count(c) != 0)
                    return false;

                Buffer bmdata;

                if constexpr(Format == RGBA)
                {
                    // we can take in either monochrome, RGB or RGBA
                    for (int row = 0; row < h; ++row)
                    for (int col = 0; col < w; ++col)
                    {
                        if (ch == 1)
                        {
                            bmdata.push_back(0xff);
                            bmdata.push_back(0xff);
                            bmdata.push_back(0xff);
                            bmdata.push_back(*((uint8_t*)(data) + col + row * w));
                        }
                        else if (ch == 3)
                        {
                            bmdata.push_back(*((uint8_t*)(data) + 0 + col * 3 + row * w));
                            bmdata.push_back(*((uint8_t*)(data) + 1 + col * 3 + row * w));
                            bmdata.push_back(*((uint8_t*)(data) + 2 + col * 3 + row * w));
                            bmdata.push_back(0xff);
                        }
                        else if (ch == 4)
                        {
                            bmdata.push_back(*((uint8_t*)(data) + 0 + col * 4 + row * w));
                            bmdata.push_back(*((uint8_t*)(data) + 1 + col * 4 + row * w));
                            bmdata.push_back(*((uint8_t*)(data) + 2 + col * 4 + row * w));
                            bmdata.push_back(*((uint8_t*)(data) + 3 + col * 4 + row * w));
                        }
                        else 
                            return false;
                    }
                }
                else if constexpr(Format == ALPHA)
                {
                    // accept only 1-channel source data
                    if (ch != 1) return false;
                    for (int row = 0; row < h; ++row)
                    for (int col = 0; col < w; ++col)
                    {
                        bmdata.push_back(*((uint8_t*)(data) + col + row * w));
                    }
                }
                else if constexpr(Format == PALETTE_ALPHA)
                {
                    // we want either 1 or 2 channels with palette_alpha
                    for (int row = 0; row < h; ++row)
                    for (int col = 0; col < w; ++col)
                    {
                        if (ch == 1)
                        {
                            bmdata.push_back(0xff);
                            bmdata.push_back(*((uint8_t*)(data) + 1 + col * 2 + row * w));
                        }
                        else if (ch == 2)
                        {
                            bmdata.push_back(*((uint8_t*)(data) + 0 + col * 2 + row * w));
                            bmdata.push_back(*((uint8_t*)(data) + 1 + col * 2 + row * w));
                        }
                        else
                            return false;
                    }
                }
                else
                    return false;

                cache[c].bitmap = bmdata;
                cache[c].format = Format;

                return true;
            }

        #ifdef MUSH_FREETYPE_FONTS
        template <size_t Siz, BitmapFormat Fmt = RGBA>
        static mush::Font<Siz, Fmt, FREETYPE_FONT> load_freetype(const mush::string& file)
        {
            return mush::Font<Siz, Fmt, FREETYPE_FONT>(file, file_to_buffer(file), "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZzÅåÄäÖö.,:;-+=?!_*\"$£€<>()'");
        }
        #endif
    };
}

#endif
