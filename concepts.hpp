/*!
 * \file concepts.hpp
 * \brief Contains basic concepts
 * \author Jari Ronkainen
 * \version 1.0
 *
 */

#ifndef MUSH_CONCEPTS
#define MUSH_CONCEPTS

#include <type_traits>

namespace mush
{
    template <typename T> concept bool PODType = std::is_pod<T>::value;
    
    template <typename T> concept bool ArithmeticType = std::is_arithmetic<T>::value;
    template <typename T> concept bool IntegralType = std::is_integral<T>::value;
    template <typename T> concept bool FloatingPointType = std::is_floating_point<T>::value;
    template <typename T> concept bool SuitableBaseType = std::is_arithmetic<T>::value && (sizeof(T) >= 4);
    
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

}

#endif
