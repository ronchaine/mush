#ifndef MUSH_BUFFERPARSER
#define MUSH_BUFFERPARSER

#include <tuple>
#include "../buffer.hpp"
#include <iostream>

#ifdef MUSH_COMPRESSION
#endif

namespace mush
{
    bool parse_buffer(const Buffer& buf)
    {
        std::cout << "f0\n";
    }

    template<typename T, typename... Rest>
    bool parse_buffer(const Buffer& buf, T& value, Rest... rest)
    {
        std::cout << "f2&\n";

        parse_buffer(buf, rest...);
    }
    
    template<typename T, typename... Rest>
    bool parse_buffer(const Buffer& buf, const T& value, Rest... rest)
    {
        std::cout << "f2\n";

        parse_buffer(buf, rest...);
    }
}
#endif
