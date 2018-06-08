/*!
 * \file buffer.hpp
 * \brief Contains definitions and implementation for the Buffer class and associated functions
 * \author Jari Ronkainen
 * \version 1.2.0-beta1
 *
 * Buffer acts much like std::vector (actually using it as internal storage), but is intended
 * for holding raw data.  In addition to usual vector operations, it is possible to read and
 * write data into the buffer using additonal IO functions that automatically handle endian
 * conversions as well.
 *
 * Buffer needs no #defines or any other magic, just include the header and it's ready to use.
 *
 */

#ifndef MUSH_BUFFER
#define MUSH_BUFFER

#include <vector>
#include <cstdint>
#include <cassert>
#include <memory>

#include <cstring>
#include <fstream>

namespace mush
{ 
    constexpr static size_t MUSH_DEFAULT_LOC = ~0;

    /** 
     * @brief Class for holding raw data
     */
    class Buffer : public std::vector<uint8_t>
    {
        public:
            bool            can_read(size_t bytes) const noexcept;
            size_t          pos() const noexcept;
            const uint8_t*  getptr() const noexcept;

            size_t          hash() const noexcept;

            void            seek(size_t target) const noexcept;
            void            replace(size_t loc, size_t len, const Buffer& src) noexcept; 

            Buffer          copy_until(const uint8_t delim) const noexcept;
            Buffer          copy_bytes(size_t len) const;

            template <typename T>
            void from_stl_type(T&& data);

            template <typename T>
            T read_strval() const;

            template <typename T>
            T read(size_t where = MUSH_DEFAULT_LOC, bool big_endian_mode = false) const noexcept;
            
            template <typename T>
            T read_le(size_t where = MUSH_DEFAULT_LOC) const noexcept;
            
            template <typename T>
            void write(T&& data, bool big_endian_mode = false) noexcept;
            
            template <typename T>
            void write_le(T&& data) noexcept;

            template <typename... T>
            void write_bytes(T... bytes) noexcept;

        private:
            mutable size_t read_ptr = 0;
            
            template <typename... T>
            void write_bytes_impl(uint8_t b, T... bytes) noexcept;
    };

    inline Buffer file_to_buffer(const char* filename);

    // IMPLEMENTATIONS

    // We want to be able to swap endianness
    template <typename T>
    T endian_swap(T&& value) noexcept
    {
        union
        {
            T value;
            uint8_t value_u8[sizeof(T)];
        } src, dst;

        src.value = std::forward<T>(value);

        for (size_t i = 0; i < sizeof(T); ++i)
            dst.value_u8[i] = src.value_u8[sizeof(T)-1-i];

        return dst.value;
    }

    // Required for hash functions
    constexpr bool size_t_x64() noexcept
    { return sizeof(size_t) == 8 ? true : false; }

    //! Checks if the system is big-endian
    /*!
        Performs fast check if the system is big-endian
    */
    inline constexpr bool big_endian() noexcept
    {
        uint16_t t = 0xbe1e;
        if ((uint8_t) *(&t) == 0x1e)
            return false;

        return true;
    }

    // Buffer class implementations

    //! get data pointer for the buffer
    inline const uint8_t* Buffer::getptr() const noexcept { return &this->at(0); }

    //! return the position of read_ptr
    inline size_t Buffer::pos() const noexcept
    {
        return read_ptr;
    }

    //! move the read_ptr
    inline void Buffer::seek(size_t target) const noexcept
    {
        if (target < size()) read_ptr = target;
    }

    /** 
     * @brief Read data until a byte is met
     * 
     * @param delim byte ending the search
     * 
     * @return A new buffer holding all the values up to and including the delim byte.
     */
    inline Buffer Buffer::copy_until(const uint8_t delim) const noexcept
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
     * @brief Read data from a buffer into a new buffer
     * 
     * @param len Number of bytes to be read
     * 
     * @return A new buffer containing len bytes of data.
     */
    inline Buffer Buffer::copy_bytes(size_t len) const
    {
        Buffer rval;
        if (size() - read_ptr < len)
            return rval;

        std::copy(data(), data() + len, back_inserter(rval));
        read_ptr += len;

        return rval;
    }

    /** 
     * @brief Convert from stl type to a buffer
     *
     * Takes a stl type and replaces contents of the buffer with the
     * contents of the type.  Mostly useful for std::string
     * 
     * @param s  input.
     *
     */
    template <typename T>
    inline void Buffer::from_stl_type(T&& s)
    {
        this->clear();
        std::copy(s.begin(), s.end(), back_inserter(*this));
    }

    /** 
     * @brief Checks if there are still enough data remaining that can be read
     * 
     * @param bytes  How many bytes need still to be read
     * 
     * @return true if size of the buffer is large enough for s bytes of data to be read, otherwise false
     */
    inline bool Buffer::can_read(size_t bytes) const noexcept
    {
        if (size() < pos() + bytes)
            return false;

        return true;
    };

    /** 
     * @brief Read a string that represents an integer
     * 
     * @return Integer of requested type represented by the text data read
     *
     */
    template <typename T>
    inline T Buffer::read_strval() const
    {
        size_t search, start;
        uint8_t v;

        for (search = read_ptr; search < size(); ++search)
        {
            if ((v != '0') && (v <= '9'))
                continue;
            break;
        }

        char val[search - read_ptr];

        start = read_ptr;
        while (read_ptr < search)
        {
            v = this->at(read_ptr);
            val[read_ptr - start] = v;
            if ((v >= '0') && (v <= '9'))
            {
                read_ptr++;
                continue;
            }
            break;
        }
        return atoi(val);
    }
            
    /** 
     * @brief Replace data in a buffer
     * 
     * @param loc   where in buffer the replaced data starts
     * @param len   how many bytes of the data will be replaced
     * @param src   source data buffer
     */
    inline void Buffer::replace(size_t loc, size_t len, const Buffer& src) noexcept
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
    inline T Buffer::read(size_t where, bool big_endian_mode) const noexcept
    {
        if (where == MUSH_DEFAULT_LOC)
            where = read_ptr;

        T rval = *(T*)&(this->at(where));

        if constexpr (std::is_pod<T>::value)
            if ((big_endian() && !big_endian_mode) || (!big_endian() && big_endian_mode))
                rval = endian_swap(rval);

        read_ptr += sizeof(T);

        return rval;
    }

    template <typename T>
    inline T Buffer::read_le(size_t where) const noexcept
    {
        return read<T>(where, true);
    }
            
    /** 
     * @brief Write data of type T to the end of the buffer
     * 
     * @param data  data to be written into the buffer
     */
    template <typename T>
    inline void Buffer::write(T&& data, bool big_endian_mode) noexcept
    {
        if constexpr(std::is_pod<T>::value)
        {
            this->reserve(this->size() + sizeof(data));

            uint8_t* dataptr = nullptr;

            if ((big_endian() && !big_endian_mode) || (!big_endian() && big_endian_mode))
            {
                T data_copy = endian_swap(std::forward<T>(data));
                dataptr = (uint8_t*)&data_copy;
                for (size_t it = 0; it < sizeof(data_copy); ++it)
                    this->push_back(*(dataptr+it));
                return;
            } else {
                dataptr = (uint8_t*)&data;
            }

            for (size_t it = 0; it < sizeof(data); ++it)
                this->push_back(*(dataptr+it));

        } else {
            size_t start = size();
            this->resize(size() + sizeof(T));
            memcpy(this->data() + start, &data, sizeof(T)); 
        }
    }
    
    // Template specialisation for buffer writing
    template<>
    inline void Buffer::write(const Buffer& buf, bool big_endian_mode) noexcept
    {
        this->insert(std::end(*this), std::begin(buf), std::end(buf));
    }

    template <typename T>
    inline void Buffer::write_le(T&& data) noexcept
    {
        write(std::forward<T>(data), true);
    }

    /** 
     * @brief Write series of bytes into the buffer
     * 
     * @param bytes  bytes to be written
     */
    template <typename... Ts>
    inline void Buffer::write_bytes_impl(uint8_t byte, Ts... bytes) noexcept
    {
        write<uint8_t>(std::move(byte));
        if constexpr (sizeof...(bytes) > 0)
            write_bytes_impl(bytes...);
    }
    
    template <typename... Ts>
    inline void Buffer::write_bytes(Ts... bytes) noexcept
    {
        this->reserve(this->size() + sizeof...(bytes));
        this->write_bytes_impl(bytes...);
    }

    // HELPER STUFF IMPLEMENTATIONS

    /** 
     * @brief Read a complete file from the filesystem into the buffer
     * 
     * @param filename  file name
     * 
     * @return Buffer containing the contents of the file
     */
    inline Buffer file_to_buffer(const char* filename)
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

// Extend std::hash with our buffer type
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
