/** 
 * @file string.hpp
 * @brief String class
 * @author Jari Ronkainen, Nemanja Trifunovic
 * @version 1.0
 * @date 2017-08-22
 *
 * The string class stores a series of char32_t characters in an array.
 * The class can be use in similar manner as the std::string class
 *
 * Strings stored are encoded in UTF-32, they are NOT guaranteed to be
 * null-terminated, so it's not recommended to use raw pointers
 * obtained with ptr().
 *
 */
#include <iostream>

#ifndef MUSH_STRING_HEADER
#define MUSH_STRING_HEADER

#ifdef MUSH_MAKE_IMPLEMENTATIONS
#define MUSH_IMPLEMENT_STRING
#endif

// utf8-stuff Copyright 2006 Nemanja Trifunovic

/*
Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/


#ifndef UTF8_FOR_CPP_CORE_H_2675DCD0_9480_4c0c_B92A_CC14C027B731
#define UTF8_FOR_CPP_CORE_H_2675DCD0_9480_4c0c_B92A_CC14C027B731

#include <iterator>
#include <cstring>

namespace utf8
{
    // The typedefs for 8-bit, 16-bit and 32-bit unsigned integers
    // You may need to change them to match your system.
    // These typedefs have the same names as ones from cstdint, or boost/cstdint
    typedef unsigned char   uint8_t;
    typedef unsigned short  uint16_t;
    typedef unsigned int    uint32_t;

// Helper code - not intended to be directly called by the library users. May be changed at any time
namespace internal
{
    // Unicode constants
    // Leading (high) surrogates: 0xd800 - 0xdbff
    // Trailing (low) surrogates: 0xdc00 - 0xdfff
    const uint16_t LEAD_SURROGATE_MIN  = 0xd800u;
    const uint16_t LEAD_SURROGATE_MAX  = 0xdbffu;
    const uint16_t TRAIL_SURROGATE_MIN = 0xdc00u;
    const uint16_t TRAIL_SURROGATE_MAX = 0xdfffu;
    const uint16_t LEAD_OFFSET         = LEAD_SURROGATE_MIN - (0x10000 >> 10);
    const uint32_t SURROGATE_OFFSET    = 0x10000u - (LEAD_SURROGATE_MIN << 10) - TRAIL_SURROGATE_MIN;

    // Maximum valid value for a Unicode code point
    const uint32_t CODE_POINT_MAX      = 0x0010ffffu;

    template<typename octet_type>
    inline uint8_t mask8(octet_type oc)
    {
        return static_cast<uint8_t>(0xff & oc);
    }
    template<typename u16_type>
    inline uint16_t mask16(u16_type oc)
    {
        return static_cast<uint16_t>(0xffff & oc);
    }
    template<typename octet_type>
    inline bool is_trail(octet_type oc)
    {
        return ((utf8::internal::mask8(oc) >> 6) == 0x2);
    }

    template <typename u16>
    inline bool is_lead_surrogate(u16 cp)
    {
        return (cp >= LEAD_SURROGATE_MIN && cp <= LEAD_SURROGATE_MAX);
    }

    template <typename u16>
    inline bool is_trail_surrogate(u16 cp)
    {
        return (cp >= TRAIL_SURROGATE_MIN && cp <= TRAIL_SURROGATE_MAX);
    }

    template <typename u16>
    inline bool is_surrogate(u16 cp)
    {
        return (cp >= LEAD_SURROGATE_MIN && cp <= TRAIL_SURROGATE_MAX);
    }

    template <typename u32>
    inline bool is_code_point_valid(u32 cp)
    {
        return (cp <= CODE_POINT_MAX && !utf8::internal::is_surrogate(cp));
    }

    template <typename octet_iterator>
    inline typename std::iterator_traits<octet_iterator>::difference_type
    sequence_length(octet_iterator lead_it)
    {
        uint8_t lead = utf8::internal::mask8(*lead_it);
        if (lead < 0x80)
            return 1;
        else if ((lead >> 5) == 0x6)
            return 2;
        else if ((lead >> 4) == 0xe)
            return 3;
        else if ((lead >> 3) == 0x1e)
            return 4;
        else
            return 0;
    }

    template <typename octet_difference_type>
    inline bool is_overlong_sequence(uint32_t cp, octet_difference_type length)
    {
        if (cp < 0x80) {
            if (length != 1) 
                return true;
        }
        else if (cp < 0x800) {
            if (length != 2) 
                return true;
        }
        else if (cp < 0x10000) {
            if (length != 3) 
                return true;
        }

        return false;
    }

    enum utf_error {UTF8_OK, NOT_ENOUGH_ROOM, INVALID_LEAD, INCOMPLETE_SEQUENCE, OVERLONG_SEQUENCE, INVALID_CODE_POINT};

    /// Helper for get_sequence_x
    template <typename octet_iterator>
    utf_error increase_safely(octet_iterator& it, octet_iterator end)
    {
        if (++it == end)
            return NOT_ENOUGH_ROOM;

        if (!utf8::internal::is_trail(*it))
            return INCOMPLETE_SEQUENCE;
        
        return UTF8_OK;
    }

    #define UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(IT, END) {utf_error ret = increase_safely(IT, END); if (ret != UTF8_OK) return ret;}    

    /// get_sequence_x functions decode utf-8 sequences of the length x
    template <typename octet_iterator>
    utf_error get_sequence_1(octet_iterator& it, octet_iterator end, uint32_t& code_point)
    {
        if (it == end)
            return NOT_ENOUGH_ROOM;

        code_point = utf8::internal::mask8(*it);

        return UTF8_OK;
    }

    template <typename octet_iterator>
    utf_error get_sequence_2(octet_iterator& it, octet_iterator end, uint32_t& code_point)
    {
        if (it == end) 
            return NOT_ENOUGH_ROOM;
        
        code_point = utf8::internal::mask8(*it);

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point = ((code_point << 6) & 0x7ff) + ((*it) & 0x3f);

        return UTF8_OK;
    }

    template <typename octet_iterator>
    utf_error get_sequence_3(octet_iterator& it, octet_iterator end, uint32_t& code_point)
    {
        if (it == end)
            return NOT_ENOUGH_ROOM;
            
        code_point = utf8::internal::mask8(*it);

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point = ((code_point << 12) & 0xffff) + ((utf8::internal::mask8(*it) << 6) & 0xfff);

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point += (*it) & 0x3f;

        return UTF8_OK;
    }

    template <typename octet_iterator>
    utf_error get_sequence_4(octet_iterator& it, octet_iterator end, uint32_t& code_point)
    {
        if (it == end)
           return NOT_ENOUGH_ROOM;

        code_point = utf8::internal::mask8(*it);

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point = ((code_point << 18) & 0x1fffff) + ((utf8::internal::mask8(*it) << 12) & 0x3ffff);

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point += (utf8::internal::mask8(*it) << 6) & 0xfff;

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point += (*it) & 0x3f;

        return UTF8_OK;
    }

    #undef UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR

    template <typename octet_iterator>
    utf_error validate_next(octet_iterator& it, octet_iterator end, uint32_t& code_point)
    {
        // Save the original value of it so we can go back in case of failure
        // Of course, it does not make much sense with i.e. stream iterators
        octet_iterator original_it = it;

        uint32_t cp = 0;
        // Determine the sequence length based on the lead octet
        typedef typename std::iterator_traits<octet_iterator>::difference_type octet_difference_type;
        const octet_difference_type length = utf8::internal::sequence_length(it);

        // Get trail octets and calculate the code point
        utf_error err = UTF8_OK;
        switch (length) {
            case 0: 
                return INVALID_LEAD;
            case 1:
                err = utf8::internal::get_sequence_1(it, end, cp);
                break;
            case 2:
                err = utf8::internal::get_sequence_2(it, end, cp);
            break;
            case 3:
                err = utf8::internal::get_sequence_3(it, end, cp);
            break;
            case 4:
                err = utf8::internal::get_sequence_4(it, end, cp);
            break;
        }

        if (err == UTF8_OK) {
            // Decoding succeeded. Now, security checks...
            if (utf8::internal::is_code_point_valid(cp)) {
                if (!utf8::internal::is_overlong_sequence(cp, length)){
                    // Passed! Return here.
                    code_point = cp;
                    ++it;
                    return UTF8_OK;
                }
                else
                    err = OVERLONG_SEQUENCE;
            }
            else 
                err = INVALID_CODE_POINT;
        }

        // Failure branch - restore the original value of the iterator
        it = original_it;
        return err;
    }

    template <typename octet_iterator>
    inline utf_error validate_next(octet_iterator& it, octet_iterator end) {
        uint32_t ignored;
        return utf8::internal::validate_next(it, end, ignored);
    }

} // namespace internal

    /// The library API - functions intended to be called by the users

    // Byte order mark
    const uint8_t bom[] = {0xef, 0xbb, 0xbf};

    template <typename octet_iterator>
    octet_iterator find_invalid(octet_iterator start, octet_iterator end)
    {
        octet_iterator result = start;
        while (result != end) {
            utf8::internal::utf_error err_code = utf8::internal::validate_next(result, end);
            if (err_code != internal::UTF8_OK)
                return result;
        }
        return result;
    }

    template <typename octet_iterator>
    inline bool is_valid(octet_iterator start, octet_iterator end)
    {
        return (utf8::find_invalid(start, end) == end);
    }

    template <typename octet_iterator>
    inline bool starts_with_bom (octet_iterator it, octet_iterator end)
    {
        return (
            ((it != end) && (utf8::internal::mask8(*it++)) == bom[0]) &&
            ((it != end) && (utf8::internal::mask8(*it++)) == bom[1]) &&
            ((it != end) && (utf8::internal::mask8(*it))   == bom[2])
           );
    }
	
    //Deprecated in release 2.3 
    template <typename octet_iterator>
    inline bool is_bom (octet_iterator it)
    {
        return (
            (utf8::internal::mask8(*it++)) == bom[0] &&
            (utf8::internal::mask8(*it++)) == bom[1] &&
            (utf8::internal::mask8(*it))   == bom[2]
           );
    }
} // namespace utf8

#endif // header guard

#ifndef UTF8_FOR_CPP_UNCHECKED_H_2675DCD0_9480_4c0c_B92A_CC14C027B731
#define UTF8_FOR_CPP_UNCHECKED_H_2675DCD0_9480_4c0c_B92A_CC14C027B731

namespace utf8
{
    namespace unchecked 
    {
        template <typename octet_iterator>
        octet_iterator append(uint32_t cp, octet_iterator result)
        {
            if (cp < 0x80)                        // one octet
                *(result++) = static_cast<uint8_t>(cp);  
            else if (cp < 0x800) {                // two octets
                *(result++) = static_cast<uint8_t>((cp >> 6)          | 0xc0);
                *(result++) = static_cast<uint8_t>((cp & 0x3f)        | 0x80);
            }
            else if (cp < 0x10000) {              // three octets
                *(result++) = static_cast<uint8_t>((cp >> 12)         | 0xe0);
                *(result++) = static_cast<uint8_t>(((cp >> 6) & 0x3f) | 0x80);
                *(result++) = static_cast<uint8_t>((cp & 0x3f)        | 0x80);
            }
            else {                                // four octets
                *(result++) = static_cast<uint8_t>((cp >> 18)         | 0xf0);
                *(result++) = static_cast<uint8_t>(((cp >> 12) & 0x3f)| 0x80);
                *(result++) = static_cast<uint8_t>(((cp >> 6) & 0x3f) | 0x80);
                *(result++) = static_cast<uint8_t>((cp & 0x3f)        | 0x80);
            }
            return result;
        }

        template <typename octet_iterator>
        uint32_t next(octet_iterator& it)
        {
            uint32_t cp = utf8::internal::mask8(*it);
            typename std::iterator_traits<octet_iterator>::difference_type length = utf8::internal::sequence_length(it);
            switch (length) {
                case 1:
                    break;
                case 2:
                    it++;
                    cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
                    break;
                case 3:
                    ++it; 
                    cp = ((cp << 12) & 0xffff) + ((utf8::internal::mask8(*it) << 6) & 0xfff);
                    ++it;
                    cp += (*it) & 0x3f;
                    break;
                case 4:
                    ++it;
                    cp = ((cp << 18) & 0x1fffff) + ((utf8::internal::mask8(*it) << 12) & 0x3ffff);                
                    ++it;
                    cp += (utf8::internal::mask8(*it) << 6) & 0xfff;
                    ++it;
                    cp += (*it) & 0x3f; 
                    break;
            }
            ++it;
            return cp;        
        }

        template <typename octet_iterator>
        uint32_t peek_next(octet_iterator it)
        {
            return utf8::unchecked::next(it);    
        }

        template <typename octet_iterator>
        uint32_t prior(octet_iterator& it)
        {
            while (utf8::internal::is_trail(*(--it))) ;
            octet_iterator temp = it;
            return utf8::unchecked::next(temp);
        }

        // Deprecated in versions that include prior, but only for the sake of consistency (see utf8::previous)
        template <typename octet_iterator>
        inline uint32_t previous(octet_iterator& it)
        {
            return utf8::unchecked::prior(it);
        }

        template <typename octet_iterator, typename distance_type>
        void advance (octet_iterator& it, distance_type n)
        {
            for (distance_type i = 0; i < n; ++i)
                utf8::unchecked::next(it);
        }

        template <typename octet_iterator>
        typename std::iterator_traits<octet_iterator>::difference_type
        distance (octet_iterator first, octet_iterator last)
        {
            typename std::iterator_traits<octet_iterator>::difference_type dist;
            for (dist = 0; first < last; ++dist) 
                utf8::unchecked::next(first);
            return dist;
        }

        template <typename u16bit_iterator, typename octet_iterator>
        octet_iterator utf16to8 (u16bit_iterator start, u16bit_iterator end, octet_iterator result)
        {       
            while (start != end) {
                uint32_t cp = utf8::internal::mask16(*start++);
            // Take care of surrogate pairs first
                if (utf8::internal::is_lead_surrogate(cp)) {
                    uint32_t trail_surrogate = utf8::internal::mask16(*start++);
                    cp = (cp << 10) + trail_surrogate + internal::SURROGATE_OFFSET;
                }
                result = utf8::unchecked::append(cp, result);
            }
            return result;         
        }

        template <typename u16bit_iterator, typename octet_iterator>
        u16bit_iterator utf8to16 (octet_iterator start, octet_iterator end, u16bit_iterator result)
        {
            while (start < end) {
                uint32_t cp = utf8::unchecked::next(start);
                if (cp > 0xffff) { //make a surrogate pair
                    *result++ = static_cast<uint16_t>((cp >> 10)   + internal::LEAD_OFFSET);
                    *result++ = static_cast<uint16_t>((cp & 0x3ff) + internal::TRAIL_SURROGATE_MIN);
                }
                else
                    *result++ = static_cast<uint16_t>(cp);
            }
            return result;
        }

        template <typename octet_iterator, typename u32bit_iterator>
        octet_iterator utf32to8 (u32bit_iterator start, u32bit_iterator end, octet_iterator result)
        {
            while (start != end)
                result = utf8::unchecked::append(*(start++), result);

            return result;
        }

        template <typename octet_iterator, typename u32bit_iterator>
        u32bit_iterator utf8to32 (octet_iterator start, octet_iterator end, u32bit_iterator result)
        {
            while (start < end)
                (*result++) = utf8::unchecked::next(start);

            return result;
        }

        // The iterator class
        template <typename octet_iterator>
          class iterator : public std::iterator <std::bidirectional_iterator_tag, uint32_t> { 
            octet_iterator it;
            public:
            iterator () {}
            explicit iterator (const octet_iterator& octet_it): it(octet_it) {}
            // the default "big three" are OK
            octet_iterator base () const { return it; }
            uint32_t operator * () const
            {
                octet_iterator temp = it;
                return utf8::unchecked::next(temp);
            }
            bool operator == (const iterator& rhs) const 
            { 
                return (it == rhs.it);
            }
            bool operator != (const iterator& rhs) const
            {
                return !(operator == (rhs));
            }
            iterator& operator ++ () 
            {
                ::std::advance(it, utf8::internal::sequence_length(it));
                return *this;
            }
            iterator operator ++ (int)
            {
                iterator temp = *this;
                ::std::advance(it, utf8::internal::sequence_length(it));
                return temp;
            }  
            iterator& operator -- ()
            {
                utf8::unchecked::prior(it);
                return *this;
            }
            iterator operator -- (int)
            {
                iterator temp = *this;
                utf8::unchecked::prior(it);
                return temp;
            }
          }; // class iterator

    } // namespace utf8::unchecked

    using namespace unchecked;
} // namespace utf8 


#endif // header guard



#include <vector>
#include <memory>
#include <string_view>

namespace mush
{
    template <typename T>
    concept bool IntegralType = std::is_integral<T>::value;
    
    template <typename T>
    concept bool FloatingType = std::is_floating_point<T>::value;

    template <typename T>
    concept bool StringConvertable = requires(T a)
    {
        { a.to_string() } -> std::string;
    };

    class string
    {
        private:
            std::vector<char32_t> data;

        public:
            typedef char32_t        value_type;
            typedef char32_t&       reference;
            typedef const char32_t& const_reference;
            typedef char32_t*       pointer;
            typedef const char32_t* const_pointer;
            typedef size_t          size_type;

            // Constructors
            string();

            // Copy & Move
            string(const string& other);
            string(string&& other);

            // Conversions
            string(const char*);
            string(const unsigned char*);
            string(const std::string&);
            string(const char32_t*);
            string(const char*, size_t);


            // This template SHOULD work for
            // any integer
            // float / double
            // string_view
            //
            // ...or anything that has to_string that return std::string
            /*
            template <StringConvertable T>
            string(T value)
            {
                std::string result = std::to_string(value);
                std::cout << "convert:" << value << ", result: " << result << "size: " << result.size() << "\n";
                utf8::utf8to32(result.begin(), result.end(), back_inserter(data));
            }
            */

            string(IntegralType value)
            {
                std::string result = std::to_string(value);
                utf8::utf8to32(result.begin(), result.end(), back_inserter(data));
            }
            
            string(FloatingType value)
            {
                std::string result = std::to_string(value);
                utf8::utf8to32(result.begin(), result.end(), back_inserter(data));
            }

            // Convenience (stupid) constructors
            string(const char32_t c) { data.push_back(c); }

            // STL compatibility
            std::vector<char32_t>::iterator begin() { return data.begin(); }
            std::vector<char32_t>::const_iterator begin() const { return data.begin(); }
            std::vector<char32_t>::iterator end() { return data.end(); }
            std::vector<char32_t>::const_iterator end() const { return data.end(); }

            // Operator overloads
            string& operator=(string&& other);
            string& operator=(const string& other);
            string operator+(const string& other) const;
            string& operator+=(const string& other);

            bool operator==(const string& other) const;
            bool operator!=(const string& other) const;
            bool operator<(const string& other) const;
            bool operator>(const string& other) const;

            char32_t operator[](const int index) const
            {
                return *(data.begin() + index);
            }

            // string pointer
            inline const char32_t* ptr() const { return &data[0]; }

            // implicit conversion to std::string
            operator std::string() const;

            // copy to C string
            void to_c_str(char* array) const;

            // return std string
            const std::string std_str() const;

            // split the string
            std::vector<string> split(const string& delim = " \n\t") const;

            // return integer value
            template<IntegralType T>
            T to_value()
            {
                return strtoul(this->std_str().c_str(), nullptr, 0);
            }
            
            // return floating point value
            template<FloatingType T>
            T to_value()
            {
                return atof(this->std_str().c_str());
            }

            // return string size
            inline size_t length() const { return data.size(); }

            // substring
            string substr(size_t start, size_t length) const;

            // content checks
            bool contains(const string&) const;
            bool empty() const;
            
            // empty data
            void clear();

            // friends
            friend inline std::ostream& operator<<(std::ostream& out, const mush::string& str)
            {
                out << str.std_str();
                return out;
            }

            friend inline string operator+(const char* left, const mush::string& right)
            {
                return mush::string(left) + right;
            }

            // FVN-1a
            template<size_t SizeSize = sizeof(size_t)>
            inline size_t hash() const noexcept
            {
                std::abort();
            };
    };

    // 64-bit version
    template<> inline size_t string::hash<8>() const noexcept
    {
        size_t hash = 0xCBF29CE484222325;
        constexpr uint64_t prime = 0x100000001B3;

        uint8_t* bytep = (uint8_t*)ptr();

        for (size_t it = 0; it < length() * 4; ++it)
            hash = (hash ^ *(bytep + it)) * prime;

        return hash;
    };
    
    // 32-bit version
    template<> inline size_t string::hash<4>() const noexcept
    {
        size_t hash = 0x811C9DC5;
        constexpr uint32_t prime = 0x1000193;

        uint8_t* bytep = (uint8_t*)ptr();

        for (size_t it = 0; it < length() * 4; ++it)
            hash = (hash ^ *(bytep + it)) * prime;

        return hash;
    };

    // compares a character to number of other characters
    inline bool match_char32(char32_t c, const string& chars)
    {
        for (char32_t c_c : chars)
            if (c == c_c)
                return true;

        return false;
    }

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
    concept bool AnyString = std::is_same<typename remove_cr<T>::type, mush::string>::value;
}

namespace std
{
    // std::hash specialisation for mush::string
    template<>
    struct hash<mush::string>
    {
        size_t operator()(const mush::string& __s) const noexcept
        {
            return __s.hash();
        }
    };
}

#ifdef MUSH_IMPLEMENT_STRING

namespace mush
{
    //! default constructor, it does nothing
    string::string() {}

    //! default copy constructor
    string::string(const string& other)
    {
        *this = other;
    }

    //! default move constructor
    string::string(string&& other)
    {
        *this = std::move(other);
    }
    
    //! Create from c++11 char32_t array
    /*!
        Creates a wcl string instance from data provided in C++11 char32_t
        array.  (i.e. U("This is an example."))  It is presumed that the
        character array is already UTF32- encoded and no conversions or
        checks are made to ensure this.
    */
    string::string(const char32_t* in)
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
        Creates wcl string instance from data provided in character array.
        It is presumed that the character array data is encoded in either
        UTF-8 or ASCII format.
    */
    string::string(const char* in)
    {
        data.clear();

        //std::string result;

        //utf8::replace_invalid(in, in + strlen(in), back_inserter(result));
        //utf8::utf8to32(result.begin(), result.end(), back_inserter(data));
        utf8::utf8to32(in, in + strlen(in), back_inserter(data));
    }
    string::string(const unsigned char* in_unsigned)
    {
        data.clear();

//        std::string result;

        const char* in = (const char*)in_unsigned;
//        utf8::replace_invalid(in, in + strlen(in), back_inserter(result));
//        utf8::utf8to32(result.begin(), result.end(), back_inserter(data));
        utf8::utf8to32(in, in + strlen(in), back_inserter(data));
    }
   
    //! Create from character array, with length
    /*!
      Creates wcl string instance from data provided in character array.
      It is presumed that the character array data is encoded in either
      UTF-8 or ASCII format.
    */
    string::string(const char* in, size_t length)
    {
        data.clear();

//        std::string result;

//        utf8::replace_invalid(in, in + length, back_inserter(result));
//        utf8::utf8to32(result.begin(), result.end(), back_inserter(data));
        utf8::utf8to32(in, in + length, back_inserter(data));
    }

    
    //! Construct from STL string
    /*!
        Creates wcl string instance from data provided in std::string.
        It is presumed that the std::string data is encoded in UTF-8 or
        ASCII format.
    */
    string::string(const std::string& in)
    {
        data.clear();

        utf8::utf8to32(in.begin(), in.end(), back_inserter(data));
    }
   
    //! Generate std::string
    /*
        The string's contents are converted from internal UTF-32 format to an UTF-8 encoding
        and returned as C++ STL string.

        \return <code>const std::string</code> containing the string in UTF-8 format.
    */
    const std::string string::std_str() const
    {
        std::string result;
        utf8::utf32to8(data.begin(), data.end(), std::back_inserter(result));
        return result;
    }

    //! Implicit conversion to std::string
    string::operator std::string() const
    {
        return this->std_str();
    }
   
    //! Generate c-style string
    /*
        The C-style string is written to array given as a parameter, no allocation is made
        so the array must be pre-allocated.

        \return nothing
    */
    void string::to_c_str(char* array) const
    {
        std::vector<char> utf8_result;

        utf8::utf32to8(data.begin(), data.end(), std::back_inserter(utf8_result));

        utf8_result.push_back('\0');
        memcpy(array, &utf8_result[0], utf8_result.size());
    }
   
    //! Split a string by a delim
    /*!
        Returns a vector of string objects that are substrings cut at given deliminators.

        \param   delim A string of characters that are to be used as deliminators.

        \return  A vector of string objects computed by splitting the string around given deliminator.
    */
    std::vector<string> string::split(const string& delim) const
    {
        std::vector<string> rval;
        if (data.size() == 0)
            return rval;

        size_t pos = 0;

        for (size_t i = 0; i < data.size(); ++i)
        {
            if (match_char32(data[i], delim))
            {
                while(match_char32(data[pos], delim))
                    pos++;

                string temp = this->substr(pos, i - pos);

                if (temp.length() != 0)
                {
                    rval.push_back(temp);
                    pos = i;
                }
            }
        }

        while(match_char32(data[pos], delim))
            pos++;

        string temp = this->substr(pos, data.size() - pos);
        if (temp.length() != 0)
            rval.push_back(temp);

        return rval;
    }

    //! Generate substring
    /*!
        Returns a newly constructed string object with its value initialised to
        a copy of substring of this object.

        The substring is part of the object that starts at character position pos
        and spans len characters.

        \param   pos   Position of the first character to be copied as a substring
        \param   len   Number of characters to include in the substring

        \return A string object with a substring of this object.
    */
    string string::substr(size_t pos, size_t len) const
    {
        string rval;
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
        Searches a string for a sequence specified by argument seq.

        \return <code>true</code> if string contains sequence.
    */
    bool string::contains(const string& seq) const
    {
        if (this->length() < seq.length())
        return false;

        size_t lastpos = this->length() - seq.length() + 1;

        for (size_t i = 0; i < lastpos; ++i)
        {
            if (this->substr(i, seq.length()) == seq)
                return true;
        }

        return false;
    }

    //! Checks if the string is empty
    /*!
        \return <code>true</code> if the string is empty.
    */
    bool string::empty() const
    {
        if (this->length() == 0)
            return true;

        return false;
    }

    //! Clears all data from the string
    /*!
    */
    void string::clear()
    {
        data.clear();
    }

    //! Copy assignment operator
    string& string::operator=(const string& other)
    {
        if (this == &other)
            return *this;

        data.resize(other.data.size());
        memcpy(&data[0], &other.data[0], other.data.size() * sizeof(char32_t));

        return *this;
    }

    //! Move assignment operator
    string& string::operator=(string&& other)
    {
        if (this != &other)
        {
            std::swap(data, other.data);
        }

        return *this;
    }

    //! Concatenate (addition) operator
    /*!
        Allows concatenating strings together, may be used with
        types conversible to strings, such as integers and floats.

        \return Concatenated string

        Example usage:
        \code
         string s1 = U"First string";
         string s2 = U"Second string";

         int32_t i = 252;

         string s = s1 + s2 + " " + i + U" is a number";

         std::cout << s.std_str().c_str() << "\n";
        \endcode

        Results in
        \code
         First stringSecond String 252 is a number
        \endcode
    */
    string string::operator+(const string& other) const
    {
        string rval(*this);
        for (auto c : other.data)
            rval.data.push_back(c);

        return rval;
    }

    string& string::operator+=(const string& other)
    {
        *this = *this + other;
        return *this;
    }

    bool string::operator==(const string& other) const
    {
        if (data.size() != other.data.size())
            return false;

        for (size_t i = 0; i < data.size(); ++i)
            if (data[i] != other.data[i])
                return false;

        return true;
    }

    bool string::operator!=(const string& other) const
    {
        return !(*this == other);
    }

    bool string::operator<(const string& other) const
    {
        for (size_t i = 0; i < std::min(length(), other.length()); ++i)
            if (data[i] < other.data[i])
                return true;
            else if (data[i] > other.data[i])
                return false;

        return false;
    }

    bool string::operator>(const string& other) const
    {
        for (size_t i = 0; i < std::min(length(), other.length()); ++i)
            if (data[i] < other.data[i])
                return false;
            else if (data[i] > other.data[i])
                return true;

        return true;
    }
}


#endif
#endif
