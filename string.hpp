#ifndef MUSH_STRING_V2_HEADER
#define MUSH_STRING_V2_HEADER

#ifdef MUSH_STRING_HEADER
#warning Old version of string in use, careful.
#endif

#include <vector>
#include <cstdio>
#include <cstdint>
#include <string>
#include <cstring>

namespace mush
{
    template <typename T>
    concept bool IntegerType = std::is_integral<T>::value 
                            && !std::is_same<T,char32_t>::value
                            && !std::is_same<T,char16_t>::value
                            && !std::is_same<T,char>::value;
    
    template <typename T>
    concept bool FloatingType = std::is_floating_point<T>::value;

    template <typename T>
    concept bool BackInsertable = requires(T a)
    {
        { a.push_back() }
    };

    class String;
    inline bool match_char32(char32_t c, const String& chars);

    inline uint8_t read_utf32(char32_t& ref, const char* in)
    {
        char32_t byte;
        byte = *in;
        if (byte < 0x80) { ref = byte; return 1; }
        else if ((byte & 0xe0) == 0xc0)
        {
            ref = (byte & 0x1f) << 6;
            byte = *(in+1);
            if (!((byte & 0xc0) == 0x80)) return 2;
            ref |= (byte & 0x3f);
            return 2;
        }
        else if ((byte & 0xf0) == 0xe0)
        {
            ref = (byte & 0x0f) << 12;
            byte = *(in+1);
            if (!((byte & 0xc0) == 0x80)) return 3;
            ref |= ((byte & 0x3f) << 6);
            byte = *(in+2);
            if (!((byte & 0xc0) == 0x80)) return 3;
            ref |= (byte & 0x3f);
            return 3;
        } else if ((byte & 0xf8) == 0xf0) {
            ref = (byte & 0x07) << 18;
            byte = *(in+1);
            if (!((byte & 0xc0) == 0x80)) return 4;
            ref |= ((byte & 0x3f) << 12);
            byte = *(in+2);
            if (!((byte & 0xc0) == 0x80)) return 4;
            ref |= ((byte & 0x3f) << 6);
            byte = *(in+3);
            if (!((byte & 0xc0) == 0x80)) return 4;
            ref |= (byte & 0x3f);
            return 4;
        }

        return 0;
    }

    template <typename T>
    uint8_t utf32_to_utf8(char32_t cp, T& output)
    {
        if (cp < 0x80)
        {
            output.push_back(static_cast<uint8_t>(cp));
            return 1;
        }
        else if (cp < 0x800)
        {
            output.push_back(static_cast<uint8_t>((cp >> 6)             | 0xc0));
            output.push_back(static_cast<uint8_t>((cp & 0x3f)           | 0x80));
            return 2;
        }
        else if (cp < 0x10000)
        {
            output.push_back(static_cast<uint8_t>((cp >> 12)            | 0xe0));
            output.push_back(static_cast<uint8_t>(((cp >> 6) & 0x3f)    | 0x80));
            output.push_back(static_cast<uint8_t>((cp & 0x3f)           | 0x80));
            return 3;
        }
        else
        {
            output.push_back(static_cast<uint8_t>((cp >> 18)            | 0xe0));
            output.push_back(static_cast<uint8_t>(((cp >> 12) & 0x3f)   | 0x80));
            output.push_back(static_cast<uint8_t>(((cp >> 6) & 0x3f)    | 0x80));
            output.push_back(static_cast<uint8_t>((cp & 0x3f)           | 0x80));
            return 4;
        }
        return 0;
    }

    class String
    {
        private:
            std::vector<char32_t> data;
        public:
            constexpr static char   END_OF_FILE[] = "<MUSH_EOF>";

            typedef char32_t        value_type;
            typedef char32_t&       reference;
            typedef const char32_t& const_reference;
            typedef char32_t*       pointer;
            typedef const char32_t* const_pointer;
            typedef size_t          size_type;

            // Constructors
            String() = default;
            String(const String& other) = default;
            String(String&& other) = default;

            //! Create from C++11 char32_t array
            /*!
                Creates a mush string instance from data provided in C++11 char32_t
                array. (i.e. U("This is an example."))  It is presumed that the
                character array is already UTF32-encoded and no conversions or checks
                are made to ensure this.
            */
            String(const char32_t* in)
            {
                data.clear();
                while(*in != 0x00000000)
                {
                    data.push_back(*in);
                    in++;
                }
            }

            //! Create from character array
            /*!
                Creates a mush string instance from data provided in character array.
                It is presumed that the character array data is encoded in either
                UTF-8 or ASCII format.
            */
            String(const char* in, size_t length = ~0)
            {
                char32_t utf32;
                uint32_t len, tlen = 0;
                while(*in != 0x00)
                {
                    len = read_utf32(utf32, in);
                    if (len == 0) break;
                    in += len;
                    tlen += len;
                    if (tlen > length)
                        break;
                    data.push_back(utf32);
                }
            }
            String(const unsigned char* in)
            {
                char32_t utf32;
                uint8_t len;
                while(*in != 0x00)
                {
                    len = read_utf32(utf32, (const char*)in);
                    if (len == 0) break;
                    in += len;
                    data.push_back(utf32);
                }
            }

            //! Construct from STL string
            /*!
                Creates wcl string instance from data provided in std::string.
                It is presumed that the std::string data is encoded in UTF-8 or
                ASCII format.
            */
            String(const std::string& in)
            {
                data.clear();
                char32_t utf32;
                size_t i = 0;
                while (i < in.size())
                {
                    i += read_utf32(utf32, in.c_str() + i);
                    data.push_back(utf32);
                }
            }

            String(IntegerType value)
            {
                std::string result = std::to_string(value);
                *this = String(result);
            }

            String(FloatingType value)
            {
                std::string result = std::to_string(value);
                *this = String(result);
            }

            String(const char32_t c) { data.push_back(c); }

            std::vector<char32_t>::iterator begin() { return data.begin(); }
            std::vector<char32_t>::const_iterator begin() const { return data.begin(); }
            std::vector<char32_t>::iterator end() { return data.end(); }
            std::vector<char32_t>::const_iterator end() const { return data.end(); }

            char32_t& operator[](const int index) { return *(data.begin() + index); }
            const char32_t operator[](const int index) const { return *(data.begin() + index); }

            size_t length() const { return data.size(); }
            size_t size() const { return data.size(); }

            const char32_t* ptr() const { return &data[0]; }

            //! Generate std::string
            /*
                The string's contents are converted from UTF-32 format to UTF-8 encoding
                and returned as C++ STL string.
            */
            std::string std_str() const
            {
                std::string result;
                for (auto c : data)
                    utf32_to_utf8(c, result);
                return result;
            }

            //! Copy assignment operator
            String& operator=(const String& other)
            {
                if (this == &other)
                    return *this;

                data.resize(other.data.size());
                memcpy(&data[0], &other.data[0], other.data.size() * sizeof(char32_t));

                return *this;
            }
            //! Move assignment operator
            String& operator=(String&& other)
            {
                if (this == &other)
                    return *this;

                std::swap(data, other.data);
                return *this;
            }

            //! Concatenate (addition) operator
            /*!
                Allows concatenating strings together, may be used with types
                conversible to strings, such as integers and floats.
            */
            String operator+(const String& other) const
            {
                String rval(*this);
                for (auto c : other.data)
                    rval.data.push_back(c);

                return rval;
            }
            String& operator+=(const String& other)
            {
                *this = *this + other;
                return *this;
            }

            bool operator==(const String& other) const
            {
                if (data.size() != other.data.size())
                    return false;

                for (size_t i = 0; i < data.size(); ++i)
                    if (data[i] != other.data[i])
                        return false;

                return true;
            }

            bool operator!=(const String& other) const
            {
                return !(*this == other);
            }

            bool operator<(const String& other) const
            {
                for (size_t i = 0; i < std::min(size(), other.size()); ++i)
                    if (data[i] < other.data[i])
                        return true;
                    else if (data[i] > other.data[i])
                        return false;

                return false;
            }

            bool operator>(const String& other) const
            {
                for (size_t i = 0; i < std::min(size(), other.size()); ++i)
                    if (data[i] < other.data[i])
                        return false;
                    else if (data[i] > other.data[i])
                        return true;

                return true;
            }

            //! Implicit conversion to std::string
            operator std::string() const
            {
                return this->std_str();
            }

            //! Checks if the string is empty
            /*!
            */
            bool empty() const
            {
                return size() == 0;
            }
            //! Clears all data from the string
            /*!
            */
            void clear()
            {
                data.clear();
            }

            //! Split a string by a delim
            /*!
                Returns a vector of string objects that are substrings cut at given deliminators.
            */
            std::vector<String> split(const String& delim = " \n\t") const
            {
                std::vector<String> rval;
                if (data.size() == 0)
                    return rval;

                size_t pos = 0;

                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (match_char32(data[i], delim))
                    {
                        while(match_char32(data[pos], delim))
                            pos++;

                        String temp = this->substr(pos, i - pos);

                        if (temp.length() != 0)
                        {
                            rval.push_back(temp);
                            pos = i;
                        }
                    }
                }

                while(match_char32(data[pos], delim))
                    pos++;

                String temp = this->substr(pos, data.size() - pos);
                if (temp.length() != 0)
                    rval.push_back(temp);

                return rval;
            }

            template <IntegerType T>
            T to_value() const { return strtol(std_str().c_str(), nullptr, 0); }

            template <FloatingType T>
            T to_value() const { return atof(std_str().c_str()); }
            //! Generate substring
            /*!
                Returns a newly constructed string object with its value initialised to
                a copy of a substring of this object

                The substring is part of the object that starts at character position pos
                and spans len characters or until the end of string.
            */
            String substr(size_t pos, size_t len = ~0) const
            {
                String rval;
                for (uint32_t it = pos; it < pos + len; ++it)
                {
                    if (it >= data.size())
                        return rval;
                    rval.data.push_back(data[it]);
                }
                return rval;
            }

            //! Does a string contain a sequence
            /*!
                Searches the string for a sequence specified by the argument seq
            */
            bool contains(const String& seq) const
            {
                if (this->length() < seq.length())
                    return false;

                size_t lastpos = this->size() - seq.size() + 1;

                for (size_t i = 0; i < lastpos; ++i)
                {
                    if (substr(i, seq.size()) == seq)
                        return true;
                }

                return false;
            }

            friend inline std::ostream& operator<<(std::ostream& out, const mush::String& str)
            {
                out << str.std_str();
                return out;
            }

            friend inline String operator+(const char* left, const mush::String& right)
            {
                return mush::String(left) + right;
            }

            template <size_t SizeSize = sizeof(size_t)>
            inline size_t hash() const noexcept
            {
                std::abort();
            }        
    };
    
    // 64-bit hash implementation
    template<> inline size_t String::hash<8>() const noexcept
    {
        size_t hash = 0xCBF29CE484222325;
        constexpr uint64_t prime = 0x100000001B3;

        uint8_t* bytep = (uint8_t*)ptr();

        for (size_t it = 0; it < length() * 4; ++it)
            hash = (hash ^ *(bytep + it)) * prime;

        return hash;
    };

    // 32-bit version
    template<> inline size_t String::hash<4>() const noexcept
    {
        size_t hash = 0x811C9DC5;
        constexpr uint32_t prime = 0x1000193;

        uint8_t* bytep = (uint8_t*)ptr();

        for (size_t it = 0; it < length() * 4; ++it)
            hash = (hash ^ *(bytep + it)) * prime;

        return hash;
    };
    
    #ifndef MUSH_INTERNAL_REMOVE_CR
    #define MUSH_INTERNAL_REMOVE_CR
    template <typename T> struct remove_cr              { typedef T type; };
    template <typename T> struct remove_cr<T&>          { typedef T type; };
    template <typename T> struct remove_cr<T&&>         { typedef T type; };
    template <typename T> struct remove_cr<const T>     { typedef T type; };
    template <typename T> struct remove_cr<const T&>    { typedef T type; };
    template <typename T> struct remove_cr<const T&&>   { typedef T type; };
    #endif

    template <typename T>
    concept bool AnyString = std::is_same<typename remove_cr<T>::type, mush::String>::value;

    // compares a character to number of other characters
    inline bool match_char32(char32_t c, const String& chars)
    {
        for (char32_t c_c : chars)
            if (c == c_c)
                return true;

        return false;
    }
    
    #ifndef DISABLE_LEGACY
    using string = String;
    #endif
}

namespace std
{
    // std::hash specialisation for mush::string
    template<>
    struct hash<mush::String>
    {
        size_t operator()(const mush::String& __s) const noexcept
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
