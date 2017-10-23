/**
 * @file buffer.hpp
 * @brief Contains definitions and implementation for the Buffer class and associated functions
 * @author Jari Ronkainen
 * @version 1.1
 * @date 2017-08-22
 *
 * Buffer acts much like std::vector (actually using it as internal storage), but is intended
 * for holding raw data.  In addition to usual vector operations, it is possible to read and
 * write data into the buffer using additonal IO functions that automatically handle endian
 * conversions as well.
 *
 * Buffer needs no #defines or any other magic, just include the header and it's ready to use.
 *
 */

#include <iostream>

#ifndef MUSH_BUFFER
#define MUSH_BUFFER

#include <vector>
#include <cstdint>
#include <cassert>
#include <memory>

#include <cstring>
#include <fstream>

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

    template <typename T> concept bool PODType = std::is_pod<T>::value;

    #ifndef MUSH_INTERNAL_REMOVE_CR
    #define MUSH_INTERNAL_REMOVE_CR
    template <typename T> struct remove_cr              { typedef T type; };
    template <typename T> struct remove_cr<T&>          { typedef T type; };
    template <typename T> struct remove_cr<T&&>         { typedef T type; };
    template <typename T> struct remove_cr<const T>     { typedef T type; };
    template <typename T> struct remove_cr<const T&>    { typedef T type; };
    template <typename T> struct remove_cr<const T&&>   { typedef T type; };
    #endif

    /** 
     * @brief Class for holding raw data
     */
    class Buffer : public std::vector<uint8_t>
    {
        public:
            /** 
             * @brief Default constructor
             *
             * The default (and only9 constructor for Buffer class,
             * sets read_ptr to begining of the Buffer and returns
             */
            Buffer() : read_ptr(0) {}

            // allow reading point to change even if the buffer is const
            //! read_ptr is the position in the Buffer where reads will take place,
            //! expressed in bytes from the start, where 0 is the start of the buffer
            mutable size_t read_ptr;

            //! get data pointer for the buffer
            inline const uint8_t* getptr() const { return &this->at(0); }

            //! get buffer FNV-1a hash
            inline size_t hash() const noexcept;

            //! convert the buffer to a std::string, sometimes useful for textual data
            inline std::string to_string() const
            {
                return std::string(this->begin(), this->end());
            }

            /** 
             * @brief Read data until a byte is met
             * 
             * @param delim byte ending the search
             * 
             * @return A new buffer holding all the values up to and including the delim byte.
             */
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

            /** 
             * @brief Convert from std::string to a buffer
             *
             * Takes a string and replaces contents of the buffer with the
             * contents of the string
             * 
             * @param s  input string.
             *
             */
            inline void from_stl_string(const std::string& s)
            {
                this->clear();
                std::copy(s.begin(), s.end(), back_inserter(*this));
            }

            /** 
             * @brief Checks if there are still enough data remaining that can be read
             * 
             * @param s  How many bytes need still to be read
             * 
             * @return true if size of the buffer is large enough for s bytes of data to be read, otherwise false
             */
            inline bool can_read(size_t s) const
            {
                if (size() < pos() + s)
                    return false;

                return true;
            };

            /** 
             * @brief Read a string that represents an integer
             * 
             * @return Integer of requested type represented by the text data read
             */
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
                return atoi(val.to_string().c_str());
            }

            /** 
             * @brief Read data from a buffer into a new buffer
             * 
             * @param len Number of bytes to be read
             * 
             * @return A new buffer containing len bytes of data.
             */
            inline Buffer read_bytes(size_t len) const
            {
                Buffer rval;
                if (size() - read_ptr < len)
                    return rval;

                std::copy(data(), data() + len, back_inserter(rval));
                read_ptr += len;

                return rval;
            }

            /** 
             * @brief Replace data in a buffer
             * 
             * @param loc   where in buffer the replaced data starts
             * @param len   how many bytes of the data will be replaced
             * @param src   source data buffer
             */
            inline void replace(size_t loc, size_t len, const Buffer& src)
            {
                // make sure the buffer is large enough
                if (size() - loc < len)
                    resize(loc + len);

                // do not read more than we can
                if (src.size() > len)
                    len = src.size();

                memcpy(data() + loc, src.data(), len);
            }

            /** 
             * @brief Read data of type T from the buffer
             * 
             * @param where the position where to read, if omitted, read_ptr is used as the position
             * 
             * @return Data read as type T.
             */
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

                if constexpr (std::is_pod<T>::value)
                {
                    if (big_endian())
                    rval = endian_swap(rval);
                }

                read_ptr += sizeof(T);

                return rval;
            }
            
            /** 
             * @brief Read little-endian data of type T from the buffer
             * 
             * @param where the position where to read, if omitted, read_ptr is used as the position
             * 
             * @return Data read as type T.
             */
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

            /** 
             * @brief Write data of type T to the end of the buffer
             * 
             * @param data  data to be written into the buffer
             */
            template <typename T>
            inline void write(const T& data)
            {
                size_t start = size();
                this->resize(size() + sizeof(T));
                memcpy(this->data() + start, &data, sizeof(T)); 
            }

            #ifndef NO_CONCEPTS
            inline void write(const PODType& data)
            {
                this->reserve(this->size() + sizeof(data));

                uint8_t* dataptr = nullptr;
                typename remove_cr<decltype(data)>::type t_val;

                if (big_endian())
                {
                    t_val = endian_swap(data);
                    dataptr = (uint8_t*)&t_val;
                } else {
                    dataptr = (uint8_t*)&data;
                }

                for (size_t it = 0; it < sizeof(data); ++it)
                    this->push_back(*(dataptr+it));
            }

            inline void write_le(const PODType& data)
            {
                this->reserve(this->size() + sizeof(data));

                uint8_t* dataptr = nullptr;
                typename remove_cr<decltype(data)>::type t_val;

                if (!big_endian())
                {
                    t_val = endian_swap(data);
                    dataptr = (uint8_t*)&t_val;
                } else {
                    dataptr = (uint8_t*)&data;
                }

                for (size_t it = 0; it < sizeof(data); ++it)
                    this->push_back(*(dataptr+it));
            }
            #endif

            // make no-op 
            inline void write_byte() {}

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


            /** 
             * @brief Write series of bytes into the buffer
             * 
             * @param bytes  bytes to be written
             */
            template <typename... Ts>
            inline void write_bytes(Ts... bytes)
            {
                this->reserve(this->size() + sizeof...(bytes));
                this->write_byte(bytes...);
            }

            //! return the position of read_ptr
            inline size_t pos() const
            {
                return read_ptr;
            }

            //! move the read_ptr
            inline void seek(size_t target) const
            {
                if (target < size()) read_ptr = target;
            }
    };

    /** 
     * @brief Read a complete file from the filesystem into the buffer
     * 
     * @param filename  file name
     * 
     * @return Buffer containing the contents of the file
     */
    inline Buffer file_to_buffer(const std::string& filename)
    {
        Buffer rval;
        std::ifstream input(filename, std::ifstream::binary);
        if (!input)
            return rval;

        size_t size;
        input.seekg(0, input.end);
        size = input.tellg();
        input.seekg(0, input.beg);

        rval.resize(size);
        input.read((char*)(&rval[0]), size);
        input.close();

        return rval;
    }

    /** 
     * @brief Write a buffer into the buffer
     * 
     * @param buf   the buffer to be appended
     */
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
