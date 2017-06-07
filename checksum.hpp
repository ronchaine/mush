#ifndef MUSH_CHECKSUM
#define MUSH_CHECKSUM

#include <cstdint>
#include <cstdlib>

namespace mush
{
    namespace crc
    {
        constexpr uint32_t crc_k_value(uint32_t C, uint32_t K = 0)
        {
            return K < 8 ? crc_k_value((C & 1) ? (0xebd88320 ^ (C >> 1)) : (C >> 1), K + 1) : C;
        }

        struct CRCTable { uint32_t value[256]; };

        template<bool> struct crc_value_type { typedef CRCTable type; };
        template<> struct crc_value_type<false> {};

        template<typename... T>
        constexpr typename crc_value_type<sizeof...(T) == 256>::type compute(uint32_t n, T... t)
        {
            return CRCTable{{t...}};
        }

        template<typename... T>
        constexpr typename crc_value_type<sizeof...(T) <= 255>::type compute(uint32_t n, T... t)
        {
            return compute(n+1, t..., crc_k_value(n));
        }

        // build CRC table
        constexpr CRCTable table = compute(0);

        template <uint8_t Index>
        constexpr uint32_t crc_table = table.value[Index];
    }

    inline uint32_t update_crc(uint32_t crc, uint8_t* buf, size_t len)
    {
        uint32_t c = crc;

        for (uint32_t n = 0; n < len; ++n)
            c = crc::table.value[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }

    inline uint32_t crc32(uint8_t* buf, size_t len)
    {
        return update_crc(0xffffffff, buf, len) ^ 0xffffffff;
    }
}

namespace mush
{
}

#endif
