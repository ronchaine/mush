/** 
 * @file encoding.hpp
 * @brief Contains base64 encoder/decoder
 * @author Jari Ronkainen
 * @version 0.3
 * @date 2017-08-22
 */
#ifndef MUSH_ENCODING
#define MUSH_ENCODING

#include <cstdint>

#include "buffer.hpp"
#include "string.hpp"

namespace mush
{
    constexpr char base64_characters[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    namespace base64
    {
        uint32_t get_b64_id(char32_t c)
        {
            if (c == '+')
                return 62;
            else if (c == '/')
                return 63;
            else if (c < 'A')
                return c - '0' + 52;
            else if (c < 'a')
                return c - 'A';
            else
                return c - 'a' + 26;
        }

        bool valid_char(char32_t c)
        {
            return std::isalnum(c) || c == '+' || c == '/' || c == '=';
        }

        string encode(const mush::Buffer& in_buf)
        {
            string rval;

            int val = 0, valb = -6;
            for (uint8_t c : in_buf)
            {
                val = (val << 8) + c;
                valb += 8;
                while (valb >= 0)
                {
                    rval += string(base64_characters[(val>>valb) & 0x3f]);
                    valb -= 6;
                }
            }
            if (valb > -6)
                rval += base64_characters[((val<<8)>>(valb+8)) & 0x3f];
            while (rval.length() % 4)
                rval += '=';

            return rval;
        }

        Buffer decode(const mush::string& in_str)
        {
            Buffer rval;

            int val = 0;
            int valb = -8;

            for (char32_t c : in_str)
            {
                if (!valid_char(c))
                    break;

                val = (val << 6) + get_b64_id(c);
                valb += 6;
                if (valb > 0)
                {
                    rval.push_back((val >> valb) & 0xff);
                    valb -= 8;
                }
            }
            return rval;
        }
    }
}

#endif
