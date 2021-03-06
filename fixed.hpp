/*! 
 * \file fixed.hpp
 * \brief Fixed point type
 * \author Jari Ronkainen
 * \version 1.0.2
 *
 * Adds fixed point arithmetic type compatible with STL functions
 *
 */
#ifndef MUSH_FIXED_POINT
#define MUSH_FIXED_POINT

#include <cstdint>
#include <ostream>

#include <type_traits>


#ifdef NO_CONCEPTS
#define SuitableBaseType typename
#define ArithmeticType typename
#ifndef MUSH_DEPENDENT_FALSE
#define MUSH_DEPENDENT_FALSE
namespace mush
{
    template<typename T> struct dependent_false
    {
        constexpr static bool value = false;
        constexpr bool operator()() { return false; }
    };
}
#endif
#else
#include "core.hpp"
#endif

namespace mush
{
    //! Fixed point type
    template <uint32_t Precision, SuitableBaseType Basetype = int32_t>
    class Fixed
    {
        private:
            Basetype value;

        public:

            // Constructors
            Fixed() : value(0) {}
            Fixed(const Fixed& other) : value(other.value) {}

            #ifndef NO_CONCEPTS
            template <ArithmeticType T>
            Fixed(T other) : value(other * (1 << Precision)) {}

            // Basic conversions
            template <IntegralType T>       operator T() { return (T)(value >> Precision); }
            template <FloatingPointType T>  operator T() { return (T)((T)value) / (1 << Precision); }
            
            template <IntegralType T>       operator T() const { return (T)(value >> Precision); }
            template <FloatingPointType T>  operator T() const { return (T)((T)value) / (1 << Precision); }
            #else

            // Use this untested horror if no concepts are available
            template <typename T, typename std::enable_if_t<std::is_arithmetic<T>::value> = 0> operator T() const
            {
                if constexpr(std::is_integral<T>::value)
                    return (T)(value >> Precision);
                else if constexpr(std::is_floating_point<T>::value)
                    return (T)((T)value) / (1 << Precision);
                else 
                    static_assert(dependent_false<T>(), "Unknown arithmetic type for fixed point conversion");
            }
            #endif

            // Conversions between Fixeds
            template <uint32_t P, SuitableBaseType T>
            operator Fixed<P>()
            {
                if (P > Precision)
                {
                    return (value >> (P - Precision));
                } else {
                    return (value << (Precision - P));
                }
            }

            constexpr bool operator<(const Fixed<Precision, Basetype>& rhs) const
            {
                return value < rhs.value;
            }

            constexpr bool operator==(const Fixed<Precision, Basetype>& rhs) const
            {
                return value == rhs.value;
            }
            
            constexpr bool operator>(const Fixed<Precision, Basetype>& rhs) const
            {
                return value > rhs.value;
            }

            // Addition / substraction
            Fixed<Precision, Basetype>& operator+=(const Fixed<Precision, Basetype>& rhs)
            {
                value += rhs.value;
                return *this;
            }
            template <ArithmeticType T>
            Fixed<Precision, Basetype>& operator+=(const T& rhs)
            {
                *this += (Fixed<Precision, Basetype>)rhs;
                return *this;
            }

            Fixed<Precision, Basetype> operator+(const Fixed<Precision, Basetype>& rhs)
            {
                Fixed<Precision, Basetype> rval(*this);
                return rval += rhs;
            }
            template <ArithmeticType T>
            Fixed<Precision, Basetype> operator+(const T& rhs)
            {
                Fixed<Precision, Basetype> rval;
                rval = *this + (Fixed<Precision, Basetype>)rhs;
                return rval;
            }

            Fixed<Precision, Basetype>& operator-=(const Fixed<Precision, Basetype>& rhs)
            {
                value -= rhs.value;
                return *this;
            }
            template <ArithmeticType T>
            Fixed<Precision, Basetype>& operator-=(const T& rhs)
            {
                *this -= (Fixed<Precision, Basetype>)rhs;
                return *this;
            }
            
            Fixed<Precision, Basetype> operator-(const Fixed<Precision, Basetype>& rhs)
            {
                Fixed<Precision, Basetype> rval(*this);
                return rval -= rhs;
            }
            template <ArithmeticType T>
            Fixed<Precision, Basetype> operator-(const T& rhs)
            {
                Fixed<Precision, Basetype> rval;
                rval = *this - (Fixed<Precision, Basetype>)rhs;
                return rval;
            }

            // Multiplication / division
            Fixed<Precision, Basetype>& operator*=(const Fixed<Precision, Basetype>& rhs)
            {
                value *= rhs.value;
                return *this;
            }
            template <ArithmeticType T>
            Fixed<Precision, Basetype>& operator*=(const T& rhs)
            {
                *this *= (Fixed<Precision, Basetype>)rhs;
                return *this;
            }

            Fixed<Precision, Basetype> operator*(const Fixed<Precision, Basetype>& rhs)
            {
                Fixed<Precision, Basetype> rval(*this);
                return rval *= rhs;
            }
            template <ArithmeticType T>
            Fixed<Precision, Basetype> operator*(const T& rhs)
            {
                Fixed<Precision, Basetype> rval;
                rval = *this * (Fixed<Precision, Basetype>)rhs;
                return rval;
            }

            // Friends
            friend inline std::ostream& operator<<(std::ostream& out, const Fixed<Precision, Basetype>& val)
            {
                constexpr uint32_t divider = (1 << Precision);
                constexpr uint32_t mask = divider - 1;

                out << (int)val << " + " << (mask & val.value) << "/" << divider << "(" << (double)val << ")";
                return out;
            }
    };
}

namespace std
{
    template <uint32_t Precision, typename Basetype>
    struct is_arithmetic<mush::Fixed<Precision, Basetype>>
    {
        constexpr static bool value = true;

        operator bool()
        {
            return value;
        }
    };
}

#ifdef NO_CONCEPTS
#undef SuitableBaseType
#undef ArithmeticType
#endif

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
