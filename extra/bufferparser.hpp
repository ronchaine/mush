#ifndef MUSH_BUFFERPARSER
#define MUSH_BUFFERPARSER

#include <tuple>
#include "buffer.hpp"

#ifdef MUSH_COMPRESSION
#endif

namespace mush
{
    template<typename... T>
    std::tuple<T...> parse(const Buffer& buf)
    {
    }
}
#endif
