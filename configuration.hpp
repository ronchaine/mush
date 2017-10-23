#ifndef MUSH_CONFIG_PARSER
#define MUSH_CONFIG_PARSER

#include "string.hpp"

#include <unordered_map>
#include <iostream>

char32_t get_utf32_char(std::istream& in)
{
    char32_t rval, byte;
    byte = in.get();
    if (in.eof())
        return EOF;
    if (byte < 0x80) return byte;
    else if ((byte & 0xe0) == 0xc0) {
        rval = (byte & 0x1f) << 6;
        byte = in.get();
        if (!((byte & 0xc0) == 0x80)) return rval;
        rval |= (byte & 0x3f);
    } else if ((byte & 0xf0) == 0xe0) {
        rval = (byte & 0x0f) << 12;
        byte = in.get();
        if (!((byte & 0xc0) == 0x80)) return rval;
        rval |= ((byte & 0x3f) << 6);
        byte = in.get();
        if (!((byte & 0xc0) == 0x80)) return rval;
        rval |= (byte & 0x3f);
    } else if ((byte & 0xf8) == 0xf0) {
        rval = (byte & 0x07) << 18;
        byte = in.get();
        if (!((byte & 0xc0) == 0x80)) return rval;
        rval |= ((byte & 0x3f) << 12);
        byte = in.get();
        if (!((byte & 0xc0) == 0x80)) return rval;
        rval |= ((byte & 0x3f) << 6);
        byte = in.get();
        if (!((byte & 0xc0) == 0x80)) return rval;
        rval |= (byte & 0x3f);
    }

    return rval;
}

namespace mush
{
    mush::String read_stream(std::istream& in, mush::String end = "\n")
    {
        mush::String rval = "";
        char32_t c;
        while(true)
        {
            c = get_utf32_char(in);
            if (c == (char32_t)EOF)
                return mush::String::END_OF_FILE;

            if (match_char32(c, end))
                break;
            else
                rval += c;
        }

        return rval;
    }

    mush::String substr_between(const mush::String& str, char32_t start, char32_t end)
    {
        size_t str_pos = 0, str_endpos = 0;
        for (size_t i = 0; i < str.length(); ++i)
        {
            if (str[i] == start)
            {
                str_pos = i+1;
                start = '\0';
            }
            else if (str[i] == end)
            {
                str_endpos = i;
                break;
            }
        }
        return str.substr(str_pos, str_endpos - str_pos);
    }

    mush::String strip(const mush::String& str, const mush::String& chars)
    {
        mush::String rval;
        for (size_t i = 0; i < str.length(); ++i)
        {
            if (match_char32(str[i], chars))
                continue;
            rval += str[i];
        }

        return rval;
    }

    std::tuple<mush::String, mush::String> divide_to_pair(const mush::String& str, char32_t delim)
    {
        mush::String first, second;
        for (size_t i = 0; i < str.length(); ++i)
        {
            if (str[i] == delim)
            {
                first = str.substr(0, i);
                second = str.substr(i+1, str.length());
                break;
            }
        }
        return std::tie(first, second);
    }

    struct Option
    {
        mush::String    name;
        mush::String    value;        
    };

    class OptionContainer
    {
        private:
            std::vector<Option> options;
        public:
            std::vector<Option>::iterator begin() { return options.begin(); }
            std::vector<Option>::const_iterator begin() const { return options.begin(); }
            std::vector<Option>::iterator end() { return options.end(); }
            std::vector<Option>::const_iterator end() const { return options.end(); }

            void clear()
            {
                options.clear();
            }

            bool has(const mush::String& key)
            {
                for (auto& opt : options)
                    if (opt.name == key)
                        return true;

                return false;
            }

            mush::String& operator[](const mush::String& key)
            {
                for (auto& opt : options)
                    if (opt.name == key)
                        return opt.value;

                Option new_opt;
                new_opt.name = key;
                options.push_back(new_opt);
                return options[options.size()-1].value;
            }
    };
    class Configuration
    {
        private:
            OptionContainer option;

        public:
            std::vector<Option>::iterator begin() { return option.begin(); }
            std::vector<Option>::iterator end()   { return option.end(); }
            std::vector<Option>::const_iterator begin() const { return option.begin(); }
            std::vector<Option>::const_iterator end() const { return option.end(); }
            
            void set(const mush::String& key, const mush::String& value)
            {
                option[key] = value;
            }

            void load_from_ini(std::istream& in)
            {
                mush::String line;
                mush::String section;
                while ((line = read_stream(in, "\n")) != mush::String::END_OF_FILE)
                {
                    if (line.empty())
                        continue;
                    // ignore comments
                    else if (line[0] == ';')
                        continue;
                    else if (line[0] == '[')
                    {
                        line = strip(line, "\t ");
                        section = substr_between(line, '[', ']');
                    }
                    else
                    {
                        if (line.split("=").size() < 2)
                            continue;

                        mush::String key, value;
                        key = section + "." + std::get<0>(divide_to_pair(line, '='));
                        key = strip(key, "\t ");
                        value = std::get<1>(divide_to_pair(line, '='));
                        set(key, value);
                    }
                }     
            }
            
            mush::String operator[](const mush::String& key)
            {
                if (option.has(key) == 0)
                    return "";

                return option[key];
            }

            void clear()
            {
                option.clear();
            }
            
            friend inline std::ostream& operator<<(std::ostream& out, const Configuration& conf)
            {
                for (auto& opt : conf.option)
                {
                    out << opt.name << " = " << opt.value << "\n";
                }
                return out;
            }
    };

    Configuration load_ini(std::istream& in)
    {
        Configuration rval;
        rval.load_from_ini(in);
        return rval;
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
