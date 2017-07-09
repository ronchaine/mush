#ifndef MUSH_BUFFER
#define MUSH_BUFFER

#include <vector>
#include <cstdint>
#include <cassert>
#include <memory>

//#include "common.hpp"
namespace mush
{
    // We want to be able to swap endianness
    template <typename T>
    T endian_swap(T value)
    {
        union
        {
            T value;
            uint8_t value_u8[sizeof(T)];
        } src, dst;

        src.value = value;

        for (size_t i = 0; i < sizeof(T); ++i)
            dst.value_u8[i] = src.value_u8[sizeof(T)-1-i];

        return dst.value;
    }

    // Required for hash functions
    constexpr bool size_t_x64()
    { return sizeof(size_t) == 8 ? true : false; }

    //! Checks if the system is big-endian
    /*!
        Performs fast check if the system is big-endian
    */
    inline constexpr bool big_endian()
    {
        uint16_t t = 0xbe1e;
        if ((uint8_t) *(&t) == 0x1e)
            return false;

        return true;
    }

    class Buffer : public std::vector<uint8_t>
    {
        public:
            Buffer()
            {
                read_ptr = 0;
            }

            // allow reading point to change even if the buffer is const
            mutable size_t read_ptr;
            inline const uint8_t* getptr() const { return &this->at(0); }
            inline size_t hash() const noexcept;

            inline std::string to_stl_string() const
            {
                return std::string(this->begin(), this->end());
            }

            Buffer read_until(const uint8_t delim) const
            {
                Buffer rval;
                uint8_t v;
                while(read_ptr < size())
                {
                    v = this->at(read_ptr);
                    rval.emplace_back(v);
                    read_ptr++;
                    if (delim == v)
                        break;
                }
                return rval;
            };

            inline void from_stl_string(const std::string& s)
            {
                this->clear();
                std::copy(s.begin(), s.end(), back_inserter(*this));
            }

            inline bool can_read(size_t s) const
            {
                if (size() < pos() + s)
                    return false;

                return true;
            };

            template <typename T>
            inline T read_strval() const
            {
                Buffer val;
                uint8_t v;
                while (read_ptr < size())
                {
                    v = this->at(read_ptr);
                    val.emplace_back(v);
                    if ((v >= '0') && (v <= '9'))
                    {
                        read_ptr++;
                        continue;
                    }
                    break;
                }
                return atoi(val.to_stl_string().c_str());
            }

            inline Buffer read_bytes(size_t len) const
            {
                Buffer rval;
                if (size() - read_ptr < len)
                    return rval;

                rval.resize(len);
                std::copy(data(), data() + len, back_inserter(rval));
                read_ptr += len;
                return rval;
            }

            inline void replace(size_t loc, size_t len, const Buffer& src)
            {
            }

            template <typename T>
            inline T read(size_t where) const
            {
                T rval = *(T*)&(this->at(where));

                if (big_endian())
                rval = endian_swap(rval);

                return rval;
            }

            template <typename T>
            inline T read() const
            {
                T rval = *(T*)&(this->at(read_ptr));

                if (big_endian())
                rval = endian_swap(rval);

                read_ptr += sizeof(T);

                return rval;
            }

            template <typename T>
            inline T read_le(size_t where) const
            {
                T rval = *(T*)&(this->at(where));

                if (!big_endian())
                rval = endian_swap(rval);

                return rval;
            }

            template <typename T>
            inline T read_le() const
            {
                T rval = *(T*)&(this->at(read_ptr));

                if (!big_endian())
                rval = endian_swap(rval);

                read_ptr += sizeof(T);

                return rval;
            }


            template <typename T>
            inline void write(const T& data)
            {
                this->reserve(this->size() + sizeof(T));

                uint8_t* dataptr = nullptr;
                T t_val;

                if (big_endian())
                {
                    t_val = endian_swap(data);
                    dataptr = (uint8_t*)&t_val;
                } else {
                    dataptr = (uint8_t*)&data;
                }

                for (size_t it = 0; it < sizeof(T); ++it)
                    this->push_back(*(dataptr+it));
            }

            template <typename T>
            inline void write_le(const T& data)
            {
                this->reserve(this->size() + sizeof(T));

                uint8_t* dataptr = nullptr;
                T t_val;

                if (!big_endian())
                {
                    t_val = endian_swap(data);
                    dataptr = (uint8_t*)&t_val;
                } else {
                    dataptr = (uint8_t*)&data;
                }

                for (size_t it = 0; it < sizeof(T); ++it)
                    this->push_back(*(dataptr+it));
            }

            inline void write_byte()
            {
            }

            template <typename T>
            inline void write_byte(uint8_t byte)
            {
                this->write<uint8_t>(byte);
            }
            template <typename... Ts>
            inline void write_byte(uint8_t byte, Ts... bytes)
            {
                this->write<uint8_t>(byte);
                this->write_byte(bytes...);
            }
            template <typename... Ts>
            inline void write_bytes(Ts... bytes)
            {
                this->reserve(this->size() + sizeof...(bytes));
                this->write_byte(bytes...);
            }

            inline size_t pos() const
            {
                return read_ptr;
            }

            inline void seek(size_t target) const
            {
                if (target < size()) read_ptr = target;
            }
    };

    template<>
    inline void Buffer::write(const Buffer& buf)
    {
        this->insert(std::end(*this), std::begin(buf), std::end(buf));
    }

    namespace hashes
    {
        // This voodoo is to find out whether size_t is 8 or 4 bytes long, it could be
        // easily extended to different sizes, but for now this is enough.
        template<typename T, bool is_x86_64 = size_t_x64()>
        class Hash
        {
            public:
            size_t operator()(const T&)
            {
                assert(0 && "unknown bit depth");
            }
        };

        // 64-bit buffer hash
        template<>
        class Hash<Buffer, true>
        {
            public:
                size_t operator()(const Buffer& s) const noexcept
                {
                    if (s.size() == 0)
                        return 0;

                    size_t hash  = 0xCBF29CE484222325;
                    const uint64_t prime = 0x100000001B3;

                    uint8_t* bytep = (uint8_t*)s.getptr();

                    for (size_t it = 0; it < s.size(); ++it)
                    {
                        hash = (hash ^ *(bytep+it)) * prime;
                    }

                    return hash;
                }
        };

        // 32-bit buffer hash
        template<>
        class Hash<Buffer, false>
        {
            public:
                size_t operator()(const Buffer& s) const noexcept
                {
                    if (s.size() == 0)
                        return 0;

                    size_t hash = 0x811C9DC5;
                    const uint32_t prime = 0x1000193;

                    uint8_t* bytep = (uint8_t*)s.getptr();

                    for (size_t it = 0; it < s.size(); ++it)
                    {
                        hash = (hash ^ *(bytep+it)) * prime;
                    }

                return hash;
            }
        };
    }
}

/*!
Return hash of the string, note that the hash is not currently cached.

\return FNV-1a hash.
*/
inline size_t mush::Buffer::hash() const noexcept
{
    hashes::Hash<Buffer> rval_hash;
    return rval_hash(*this);
}

namespace std
{
    template<>
    struct hash<mush::Buffer>
    { 
        size_t operator()(const mush::Buffer& __s) const noexcept
        {
            return __s.hash();
        }
    };
}

#endif
