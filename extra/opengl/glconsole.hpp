#ifndef MUSH_EXTRA_GFX_CONSOLE_HEADER
#define MUSH_EXTRA_GFX_CONSOLE_HEADER

#include "drawingtools.hpp"

#include "../../font.hpp"
#include "../../ansi.hpp"

namespace mush::extra::opengl::console
{
    constexpr static char OPT_TRUE[] = "TRUE";
    constexpr static char OPT_NULL[] = "NULL";
    class Options
    {
        private:
            std::unordered_map<mush::String, mush::String> opts;

        public:
            Options()
            {
                clear();
            }

            void clear()
            {
                opts.clear();
            }

            mush::String operator[](const mush::String& str)
            {
                if (opts.count(str) == 0)
                    return OPT_NULL;
                else
                    return opts[str];
            }

            template<IntegerType T>
            T as_value(const mush::String& str)
            {
                if (opts.count(str) == 0)
                    return 0;

                return strtol(opts[str].std_str().c_str(), nullptr, 0);
            }

            void parse(const mush::String& str)
            {
                for (mush::String opt : str.split(","))
                {
                    for (size_t i = 0; i < opt.length(); ++i)
                    {
                        if (opt[i] == '=')
                        {
                            opts[opt.substr(0,i)] = opt.substr(i+1,999);
                            break;
                        }
                        else if (i == opt.length()-1)
                        {
                            opts[opt] = OPT_TRUE;
                        }
                    }
                }
/*
                std::cout << "Final:\n";
                for (auto& p : opts)
                {
                    std::cout << "  " << p.first << "--" << p.second << "\n";
                }
*/
            }
    };

    struct SeqInfo
    {
            using SeqType = uint32_t;

            SeqType type;

            // CSI seqs
            static constexpr SeqType SET_CURSOR_POS         = 0x00;
            static constexpr SeqType CURSOR_UP              = 0x01;
            static constexpr SeqType CURSOR_DOWN            = 0x02;
            static constexpr SeqType CURSOR_FORWARD         = 0x03;
            static constexpr SeqType CURSOR_BACKWARD        = 0x04;
            static constexpr SeqType SAVE_CURSOR_POS        = 0x05;
            static constexpr SeqType RESTORE_CURSOR_POS     = 0x06;

            static constexpr SeqType ERASE_DISPLAY          = 0x07;
            static constexpr SeqType ERASE_LINE             = 0x08;

            static constexpr SeqType SET_GRAPHICS_MODE      = 0x0A;
            static constexpr SeqType SET_KEYBOARD_STRINGS   = 0x0B;

            // OSC seqs
            static constexpr SeqType ITERM2_SEQUENCE        = 0xA0;
            static constexpr SeqType MTX_SEQUENCE           = 0xA1;

            static constexpr SeqType INVALID_SEQUENCE       = ~0b1;
            static constexpr SeqType UNKNOWN_SEQUENCE       = ~0;

            mush::String seq;
    };

    SeqInfo get_seq(mush::AnyString str, size_t& pos)
    {
        SeqInfo rval;
        char32_t c;

        rval.type = SeqInfo::UNKNOWN_SEQUENCE;

        if (pos + 1 >= str.length())
            return rval;

        ++pos; c = *(str.begin() + pos);

        // get osc seq
        if (c == ']')
        {
            int opc = -1;
            ++pos; c = *(str.begin() + pos);

            size_t osiz = pos;
            for (; pos < str.length(); pos++)
            {
                if ((str[pos] < '0') || (str[pos] > '9'))
                {
                    if (osiz == pos)
                        break;

                    mush::String ss = str.substr(osiz, pos - osiz);
                    opc = ss.to_value<int>();
                    break;
                }
            }
            
            if (pos + 1 >= str.length())
                return rval;
            pos++;

            if (opc == -1)
            {
                rval.type = SeqInfo::INVALID_SEQUENCE;
                return rval;
            } else if (opc == 1337) {
                rval.type = SeqInfo::ITERM2_SEQUENCE;
            } else if (opc == 667) {
                rval.type = SeqInfo::MTX_SEQUENCE;
            } else {
                rval.type = SeqInfo::UNKNOWN_SEQUENCE;
            }

            osiz = pos;
            for (; pos < str.length(); ++pos)
            {
                if (str[pos] == '\x1b')
                {
                    if (pos+1 < str.length())
                        break;
                    pos++;
                    if (str[pos] == '\\')
                        break;
                }
                if ((str[pos] == 0x07) || (str[pos] == 0x9c)) // BEL or ST
                    break;

                rval.seq += str[pos];
            }

            return rval;
        }
        // get csi seq
        else if (c == '[')
        {
            if (pos + 1 >= str.length()) return rval;
            ++pos; c = *(str.begin() + pos);

            for (; pos < str.length(); ++pos)
            {
                c = *(str.begin() + pos);
                if ((c == 'H') || (c == 'f'))
                {
                    rval.type = SeqInfo::SET_CURSOR_POS;
                    break;
                }
                else if (c == 'A')  { rval.type = SeqInfo::CURSOR_UP;           break; }
                else if (c == 'B')  { rval.type = SeqInfo::CURSOR_DOWN;         break; }
                else if (c == 'C')  { rval.type = SeqInfo::CURSOR_FORWARD;      break; }
                else if (c == 'D')  { rval.type = SeqInfo::CURSOR_BACKWARD;     break; }
                else if (c == 's')  { rval.type = SeqInfo::SAVE_CURSOR_POS;     break; }
                else if (c == 'u')  { rval.type = SeqInfo::RESTORE_CURSOR_POS;  break; }
                else if (c == '2')
                {
                    if (str.begin() + pos + 1 > str.end())
                        continue;
                    if (*(str.begin() + pos + 1) == 'J')
                    {
                        rval.type = SeqInfo::ERASE_DISPLAY; break;
                    }
                }
                else if (c == 'K')  { rval.type = SeqInfo::ERASE_LINE;          break; }
                else if (c == 'm')
                {
                    rval.type = SeqInfo::SET_GRAPHICS_MODE;
                    break;
                }
                else if (c == 'p')  { rval.type = SeqInfo::SET_KEYBOARD_STRINGS;break; }

                rval.seq += c;
            }

            return rval;
        }
        else
        {
            rval.type = SeqInfo::INVALID_SEQUENCE;
            return rval;
        }
    }

    template <AnyVertexType VT>
    class Console
    {
        private:
            VertexBuffer<VT>*   vbuf_ptr;
            SpriteSheet<4>*     spritesheet;

            struct {
                void*           ptr;
                BitmapFormat    fmt;
                FontType        type;
            } font;

            uint16_t            mode_flags;

            struct {
                int32_t         x;
                int32_t         y;

                int32_t         x_saved;
                int32_t         y_saved;

                int32_t         x_start;
                int32_t         y_start;

                int32_t         next_line;
            } cursor;

            struct {
                // VGA colours
                uint32_t        BLACK               = 0x000000ff;
                uint32_t        RED                 = 0xaa0000ff;
                uint32_t        GREEN               = 0x00aa00ff;
                uint32_t        YELLOW              = 0xaa5500ff;
                uint32_t        BLUE                = 0x0000aaff;
                uint32_t        MAGENTA             = 0xaa00aaff;
                uint32_t        CYAN                = 0x00aaaaff;
                uint32_t        WHITE               = 0xaaaaaaff;

                uint32_t        LIGHT_BLACK         = 0x555555ff;
                uint32_t        LIGHT_RED           = 0xff5555ff;
                uint32_t        LIGHT_GREEN         = 0x55ff55ff;
                uint32_t        LIGHT_YELLOW        = 0xaaaa55ff;
                uint32_t        LIGHT_BLUE          = 0x5555ffff;
                uint32_t        LIGHT_MAGENTA       = 0xff55ffff;
                uint32_t        LIGHT_CYAN          = 0x55ffffff;
                uint32_t        LIGHT_WHITE         = 0xffffffff;
            } colours;

            struct modes {
                static constexpr uint16_t BRIGHT    = 0x01;
                static constexpr uint16_t ITALIC    = 0x02;
                static constexpr uint16_t BOLD      = 0x04;
            };

            uint32_t            colour;

            mush::Rectangle     scissor;


        public:
            Console(VertexBuffer<VT>& buf, SpriteSheet<4>& sheet)
            {
                vbuf_ptr            = &buf;
                font.ptr            = nullptr;

                spritesheet         = &sheet;

                mode_flags          = 0;

                cursor.next_line    = 0;

                colour              = 0xffffffff;
            }

            void set_spritesheet(SpriteSheet<4>& sheet)
            {
                spritesheet = &sheet;
            }

            template <BitmapFormat Format, FontType FT>
            void set_font(Font<Format, FT>& usefont)
            {
                font.ptr  = &usefont;
                font.type = FT;
                font.fmt  = Format;
            }

            void set_colour(uint32_t c)
            {
                colour = c;
            }

            void set_origin(int32_t x, int32_t y)
            {
                cursor.x_start = x;
                cursor.y_start = y;
            }

            void set_cursor(int32_t x, int32_t y)
            {
                cursor.x = x;
                cursor.y = y;
            }

            void save_position()
            {
                cursor.x_saved = cursor.x;
                cursor.y_saved = cursor.y;
            }

            void restore_position()
            {
                cursor.x = cursor.x_saved;
                cursor.y = cursor.y_saved;
            }

            void reset_position()
            {
                cursor.x = cursor.x_start;
                cursor.y = cursor.y_start;
                cursor.next_line = 0;
            }

            void write(const char* tbs)
            {
                write(mush::String(tbs));
            }

            void write(mush::AnyString tbs)
            {
                if (tbs.length() == 0)
                    return;

                int32_t space_length;
                int32_t line_spacing;
                int32_t pixel_size;
                mush::String* prefix;

                // prepare variables
                if (font.type == FREETYPE_FONT)
                {
                    Font<RGBA,FREETYPE_FONT>* fptr = (Font<RGBA,FREETYPE_FONT>*)font.ptr;
                    space_length = fptr->space_length;
                    line_spacing = fptr->pixel_size+2;
                    pixel_size = fptr->pixel_size;
                    prefix = &fptr->prefix;
                } else {
                    std::cout << __FILE__ << ";" << __LINE__ << ": unhandled font type or font not set\n";
                    return;
                }

                // draw string
                for (size_t i = 0; i < tbs.length(); ++i)
                {
                    char32_t c = *(tbs.begin() + i);

                    // escape seqs
                    if (c == '\x1b')
                    {
                        SeqInfo si = get_seq(tbs, i);

                        // we got the seq type and its contents
                        if (si.type == SeqInfo::SET_CURSOR_POS) {
                        } else if (si.type == SeqInfo::CURSOR_UP) {
                        } else if (si.type == SeqInfo::CURSOR_DOWN) {
                        } else if (si.type == SeqInfo::CURSOR_FORWARD) {
                        } else if (si.type == SeqInfo::CURSOR_BACKWARD) {
                        } else if (si.type == SeqInfo::SAVE_CURSOR_POS) {
                        } else if (si.type == SeqInfo::RESTORE_CURSOR_POS) {
                        } else if (si.type == SeqInfo::ERASE_DISPLAY) {
                        } else if (si.type == SeqInfo::ERASE_LINE) {
                        } else if (si.type == SeqInfo::SET_GRAPHICS_MODE) {
                            auto codes = si.seq.split(";:");
                            for (size_t j = 0; j < codes.size(); ++j)
                            {
                                uint8_t code = codes[j].to_value<uint8_t>();
                                if (code >= 30 && code < 38) // standard colours
                                {
                                    if      (code == 30) colour = colours.BLACK;
                                    else if (code == 31) colour = colours.RED;
                                    else if (code == 32) colour = colours.GREEN;
                                    else if (code == 33) colour = colours.YELLOW;
                                    else if (code == 34) colour = colours.BLUE;
                                    else if (code == 35) colour = colours.MAGENTA;
                                    else if (code == 36) colour = colours.CYAN;
                                    else if (code == 37) colour = colours.WHITE;
                                } else if (code == 38) { // extended colour switch
                                    if (j + 1 >= codes.size()) continue;
                                    ++j; code = codes[j].to_value<uint8_t>();

                                    if (code == 5) // 256-colour select
                                    {
                                        // get next entry, again
                                        if (j + 1 >= codes.size()) continue;
                                        ++j; code = codes[j].to_value<uint8_t>();
                                        
                                        if      (code == 0x00) colour = colours.BLACK;
                                        else if (code == 0x01) colour = colours.RED;
                                        else if (code == 0x02) colour = colours.GREEN;
                                        else if (code == 0x03) colour = colours.YELLOW;
                                        else if (code == 0x04) colour = colours.BLUE;
                                        else if (code == 0x05) colour = colours.MAGENTA;
                                        else if (code == 0x06) colour = colours.CYAN;
                                        else if (code == 0x07) colour = colours.WHITE;
                                        else if (code == 0x08) colour = colours.LIGHT_BLACK;
                                        else if (code == 0x09) colour = colours.LIGHT_RED;
                                        else if (code == 0x0a) colour = colours.LIGHT_GREEN;
                                        else if (code == 0x0b) colour = colours.LIGHT_YELLOW;
                                        else if (code == 0x0c) colour = colours.LIGHT_BLUE;
                                        else if (code == 0x0d) colour = colours.LIGHT_MAGENTA;
                                        else if (code == 0x0e) colour = colours.LIGHT_CYAN;
                                        else if (code == 0x0f) colour = colours.LIGHT_WHITE;
                                        else if ((code >= 0x10) && (code < 0xe7))
                                        {
                                            //0x10-0xE7:  6 × 6 × 6 cube (216 colors): 16 + 36 × r + 6 × g + b (0 ≤ r, g, b ≤ 5)
                                            uint32_t r,g,b;
                                            r = ((code - 16) / 36) * 0x33;
                                            g = (((code - 16) / 6) % 6) * 0x33;
                                            b = ((code - 16) % 6) * 0x33;
                                            colour = (r << 24) + (g << 16) + (b << 8) + 0xff;
                                        }
                                    } else if (code == 2) { // RGB colour selection
                                        /*
                                         * from xterm
                                         * Actual source:
                                         *   ESC[ … 38;2;<r>;<g>;<b> … m Select RGB foreground color
                                         *   ESC[ … 48;2;<r>;<g>;<b> … m Select RGB background color
                                         *
                                         * Official standard:
                                         *   ESC[ … 38:2:<r>:<g>:<b>:<unused>:<CS tolerance>:<Color-Space: 0="CIELUV"; 1="CIELAB">m Select RGB foreground color
                                         *   ESC[ … 48:2:<r>:<g>:<b>:<unused>:<CS tolerance>:<Color-Space: 0="CIELUV"; 1="CIELAB">m Select RGB background color
                                         */
                                        uint32_t r,g,b;
                                        if (j + 1 >= codes.size()) continue;
                                        ++j; r = codes[j].to_value<uint32_t>();
                                        if (j + 1 >= codes.size()) continue;
                                        ++j; g = codes[j].to_value<uint32_t>();
                                        if (j + 1 >= codes.size()) continue;
                                        ++j; b = codes[j].to_value<uint32_t>();

                                        colour = (r << 24) + (g << 16) + (b << 8) + 0xff;
                                    }
                                }
                            }
                        } else if (si.type == SeqInfo::SET_KEYBOARD_STRINGS) {
                        } else if (si.type == SeqInfo::ITERM2_SEQUENCE) {
                            // iTerm2 extends the xterm protocol with a set of proprietary
                            // escape sequences.  In general, the pattern is:
                            // ESC ] 1337 ; key = value ^G
                            std::cout << si.seq << "\n";
                        } else if (si.type == SeqInfo::MTX_SEQUENCE) {
                            // MTX sequences, format defined in MTX-seq-ref.txt,
                            // it has a lot of similarity to how png works.
                            parse_mtx_seq(si, pixel_size, line_spacing);
                        }  // si.type
                        continue;
                    } // escape sequences

                    if (c == ' ')
                    {
                        cursor.x += space_length;
                        continue;
                    }

                    if (c == '\n')
                    {
                        int32_t nlh = get_nlh(tbs, i, pixel_size);

                        cursor.x = cursor.x_start;
                        if (cursor.next_line == 0)
                            cursor.y += line_spacing;
                        else
                            cursor.y += cursor.next_line;

                        cursor.y += nlh;
                        cursor.next_line = 0;
                        continue;
                    }

                    GlyphMetrics glyph_metrics;

                    if (font.type == FREETYPE_FONT)
                    {
                        Font<RGBA,FREETYPE_FONT>* fptr = (Font<RGBA,FREETYPE_FONT>*)font.ptr;

                        if (!fptr->has_glyph(c))
                            fptr->add_glyph(c);

                        if (!fptr->has_glyph(c))
                            continue;

                        glyph_metrics = fptr->glyph(c).metrics;

                        if (!spritesheet->has(fptr->prefix + c))
                            spritesheet->add(fptr->prefix + c, &fptr->glyph(c).bitmap[0], glyph_metrics.width, glyph_metrics.height);

                    } else {
                        assert(0 && "reached unreachable code at glconsole.hpp:509");
                    }

                    if (!spritesheet->has(*prefix + c))
                        continue;

                    draw::sprite(*vbuf_ptr, (*spritesheet)[*prefix + c], cursor.x, cursor.y - glyph_metrics.top + pixel_size, colour);
                    //draw::scaled_sprite(*vbuf_ptr, (*spritesheet)[*prefix + c], 2.0f, cursor.x, cursor.y - glyph_metrics.top + pixel_size, colour);

                    cursor.x += glyph_metrics.advance - 1;
                    cursor.y += glyph_metrics.vertical_advance;
                }
            }

            int get_nlh(const mush::String& str, size_t pos, size_t pixel_size)
            {
                int rval = 0;
                for (size_t i = pos; i < str.length(); ++i)
                {
                    if (str[i] == '\n')
                    {
                        Options opts;
                        for (size_t j = i + 1; j < str.length(); ++j)
                        {
                            if (str[j] == '\x1b')
                            {
                                opts.clear();

                                SeqInfo si = get_seq(str, j);
                                if (si.type != SeqInfo::MTX_SEQUENCE)
                                    continue;
                                
                                auto seq_blocks = si.seq.split(";");

                                // FIXME: This is probably wrong
                                if (seq_blocks[0] == "move")
                                    return 0;

                                if (seq_blocks[0] != "cimg")
                                    continue;

                                if (seq_blocks.size() > 1)
                                {
                                    auto val_opt = seq_blocks[1].split(":");
                                    if (!spritesheet->has(val_opt[0]))
                                        continue;

                                    if (!(val_opt.size() > 1))
                                        continue;

                                    opts.parse(val_opt[1]);

                                    // Calculate the Y offset
//                                    int32_t ypos = cursor.y;
                                    int32_t ysiz = (*spritesheet)[val_opt[0]].r.h;
                                    int32_t yoff = 0;

                                    if (opts["resize_fit"] == OPT_TRUE)
                                    {
 //                                       float ratio = (float)pixel_size / (float)ysiz;
                                        ysiz = pixel_size;
                                    }
                                    if (opts["height"] != OPT_NULL)
                                        ysiz = opts.as_value<uint32_t>("height");
                                    if (opts["align"] != OPT_NULL)
                                    {
                                        if (opts["align"] == "centre" || opts["align"] == "center")
                                            yoff = ysiz / 2 - pixel_size / 2;
                                        else if (opts["align"] == "top")
                                            yoff = ysiz - pixel_size;
                                    }

                                    if (rval < yoff)
                                        rval = yoff;
                                }
                            }
                            
                            if (str[j] == '\n')
                                break;
                        }
                        break;
                    }
                }
                return rval;
            }

            void parse_mtx_seq(SeqInfo& si, int32_t pixel_size, int32_t line_spacing)
            {
                auto seq_blocks = si.seq.split(";");

                int ypos;
                int yoff;
                int xsiz;
                int ysiz;

                // rgba
                // RRGGBBAA
                if (seq_blocks[0] == "rgba")
                {
                    if (seq_blocks.size() < 2)
                        return;
                    if (seq_blocks[1].length() != 8)
                        return;
                    
                    colour = strtoul(seq_blocks[1].std_str().c_str(), nullptr, 16);
                    return;
                }
                // cimg sequence:
                // "filename":option1,option2,etc...;"filename2"...
                else if (seq_blocks[0] == "cimg")
                {
                    Options opts;

                    std::vector<mush::String> values;
                    // each part after first is a name of the cached image

                    for (size_t i = 1; i < seq_blocks.size(); ++i)
                    {
                        values = seq_blocks[i].split(":");
                        if (values.size() != 1)
                            opts.parse(values[1]);

                        if (!spritesheet->has(values[0]))
                            continue;

                        ypos = cursor.y;
                        xsiz = (*spritesheet)[values[0]].r.w;
                        ysiz = (*spritesheet)[values[0]].r.h;
                        yoff = 0;

                        if (opts["resize_fit"] == OPT_TRUE)
                        {
                            float ratio = (float)pixel_size / (float)(*spritesheet)[values[0]].r.h;
                            ysiz = pixel_size;
                            xsiz *= ratio;
                        }
                        if (opts["width"] != OPT_NULL)
                            xsiz = opts.as_value<uint32_t>("width");
                        if (opts["height"] != OPT_NULL)
                            ysiz = opts.as_value<uint32_t>("height");
                        if (opts["align"] != OPT_NULL)
                        {
                            if (opts["align"] == "centre" || opts["align"] == "center")
                                yoff = ysiz / 2 - pixel_size / 2;
                            else if (opts["align"] == "top")
                                yoff = ysiz - pixel_size;
                        }

                        draw::sprite(*vbuf_ptr, (*spritesheet)[values[0]],
                                     cursor.x,
                                     ypos - yoff,
                                     xsiz,
                                     ysiz,
                                     colour
                                    );
                        cursor.x += xsiz;

                        if (cursor.next_line < ysiz - yoff)
                            cursor.next_line = ysiz - yoff;
                    }
                } else {
                    std::cout << "unknown mtx sequence\n";
                }
            }

            template <typename... Ts>
            void write(Ts... t)
            {
                mush::String cs = (... + mush::String(t));
                write(cs);
            }

    };

    inline mush::String colour(uint32_t c)
    {
        mush::String r;
        r = "\x1b]667;rgba;";
        char values[9];
        sprintf(&values[0], "%08x", c);
        r += values;
        r += ";\x07";
        return r;
    }

    template <typename... Args>
    inline mush::String image(const mush::String& s, Args... args)
    {
        mush::String r = "\x1b]667;cimg;";
        r += s;
        if (sizeof...(args) > 0)
        {
            r += ":";
            r += (... + mush::String(args));
        }
        r += ";\x07";
        return r;
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
