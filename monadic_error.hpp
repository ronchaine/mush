#ifndef MUSH_MONADIC_ERROR_HPP
#define MUSH_MONADIC_ERROR_HPP

#include <utility>
#include <functional>
#include <new>

namespace mush
{
    template <typename FlagType>
    struct Result_Flags
    {
        constexpr static FlagType CLEAR_FLAGS   = 0x00;
        constexpr static FlagType HAS_VALUE     = 0x01;
        constexpr static FlagType NEED_CLEANUP  = 0x02;

        FlagType flags = 0;

        void set_value()    { flags |= HAS_VALUE; }
        void unset_value()  { flags &= ~HAS_VALUE; }
        void set_dirty()    { flags |= NEED_CLEANUP; }
        void set_clean()    { flags &= ~NEED_CLEANUP; }
        void clear_flags()  { flags = 0; }

        bool has_value()    const { return flags & HAS_VALUE; }
        bool is_dirty()     const { return flags & NEED_CLEANUP; }

        Result_Flags(bool value = false) { if (value) set_value(); }
    };

    template <typename ValueType, typename ErrorType, typename FlagType, bool ValueIsRef>
    struct Result_Storage
    {
        union {
            ValueType value;
            ErrorType error;
        };

        Result_Flags<FlagType> flags;

        constexpr Result_Storage() {}

        template <typename V = ValueType, typename std::enable_if_t<!std::is_same<V,ErrorType>::value, int> = 0>
        constexpr Result_Storage(ValueType value) : value(std::move(value)), flags(true) {}
        constexpr Result_Storage(ErrorType error) : error(std::move(error)), flags() {}
        
        void clean()
        {
            if (flags.is_dirty())
            {
                if (flags.has_value()) value.~ValueType();
                else error.~ErrorType();
            }
        }
        ~Result_Storage() { clean(); }
    };
    
    template <typename ValueType, typename ErrorType, typename FlagType>
    struct Result_Storage<ValueType, ErrorType, FlagType, true>
    {
        union {
            typename std::remove_reference<ValueType>::type* value;
            ErrorType error;
        };

        Result_Flags<FlagType> flags;

        constexpr Result_Storage(ValueType& ref) : flags(true) { value = &ref; }
        constexpr Result_Storage(ErrorType error) : error(std::move(error)), flags() {}
        
        void clean()
        {
            if (flags.has_value()) return;
            if (flags.is_dirty())
               error.~ErrorType();
        }
        ~Result_Storage() { clean(); }
    };
    
    template <typename ErrorType, typename FlagType, bool ValueIsRef>
    struct Result_Storage<void, ErrorType, FlagType, ValueIsRef>
    {
        ErrorType error;
        Result_Flags<FlagType> flags;

        constexpr Result_Storage() {}
        constexpr Result_Storage(ErrorType error) : error(std::move(error)), flags(this->CLEAR_FLAGS) {}
        
        void clean()
        {
            if (flags.is_dirty())
                error.~ErrorType();
        }
        ~Result_Storage() { clean(); }
    };

    template <typename ValueType, typename ErrorType, typename FlagType = unsigned char,
              bool ValueIsRef = std::is_reference<ValueType>::value>
    class Basic_Result
    {
        private:
            Result_Storage<ValueType, ErrorType, FlagType, ValueIsRef> stored;

        public:
            constexpr Basic_Result() {}
            
            template <typename V = ValueType,
                      typename std::enable_if_t<!std::is_same<V,ErrorType>::value, int> = 0,
                      typename std::enable_if_t<!std::is_void<V>::value, int> = 0>
            constexpr Basic_Result(V value) : stored(std::forward<V>(value)) {}
            constexpr Basic_Result(ErrorType error) : stored(std::forward<ErrorType>(error)) {}

            template <typename T>
            constexpr Basic_Result& operator=(T in_value)
            {
                if constexpr(std::is_convertible<T, ValueType>::value)
                {
                    stored.clean();
                    new (&stored.value) ValueType(std::move(in_value));
                    stored.flags.set_value();
                    if constexpr(std::is_trivial<ValueType>::value) stored.flags.set_dirty();
                }
                else if constexpr(std::is_convertible<T, ErrorType>::value)
                {
                    stored.clean();
                    new (&stored.error) ErrorType(std::move(in_value));
                    stored.flags.unset_value();
                    if constexpr(!std::is_trivial<ErrorType>::value) stored.flags.set_dirty();
                }
                else
                    static_assert(std::is_convertible<T, ValueType>::value || std::is_convertible<T, ErrorType>::value);

                return *this;
            }

            constexpr operator bool() const { return stored.flags.has_value(); }

            constexpr ValueType copy_value() const { if (*this) return stored.value; return ValueType(); }
            constexpr ErrorType copy_error() const { if (*this) return stored.value; return ValueType(); }

            // Stuff to get to the actual result value
            template <typename V = ValueType, typename E = ErrorType,
                      typename std::enable_if_t<!std::is_void<V>::value, int> = 0,
                      typename std::enable_if_t<!std::is_void<E>::value, int> = 0>
            ValueType catch_error(std::function<ValueType(E&)> handler)
            {
                if (*this) return unwrap();             
                return std::forward<ValueType>(handler(stored.error));
            }

            template <typename V = ValueType,
                      typename std::enable_if_t<!std::is_void<V>::value, int> = 0>
            ValueType unwrap()
            {
                if (*this) {
                    stored.flags.clear_flags();
                    return std::move(stored.value);
                }

                stored.clean();
                std::abort();
            }

            template <typename V = ValueType, typename std::enable_if_t<!std::is_void<V>::value, int> = 0>
            ValueType value_or(V rval)
            {
                if (*this) return unwrap();
                else return std::move(rval);
            }
    };


    template <typename ValueType, typename ErrorType, typename FlagType>
    class Basic_Result<ValueType, ErrorType, FlagType, true>
    {
        private:
            Result_Storage<ValueType, ErrorType, FlagType, true> stored;

        public:
            constexpr Basic_Result() {}

            constexpr Basic_Result(ValueType& value) : stored(value) {}
            constexpr Basic_Result(ErrorType error) : stored(std::forward<ErrorType>(error)) {}

            template <typename T>
            constexpr Basic_Result& operator=(T&& in_value)
            {
                if constexpr(std::is_convertible<T, ValueType>::value)
                {
                    stored.clean();
                    stored.flags.set_value();
                }
                else if constexpr(std::is_convertible<T, ErrorType>::value)
                {
                    stored.clean();
                    new (&stored.error) ErrorType(std::move(in_value));
                    stored.flags.unset_value();
                    if constexpr(!std::is_trivial<ErrorType>::value) stored.flags.set_dirty();
                }
                else
                    static_assert(std::is_convertible<T, ValueType>::value || std::is_convertible<T, ErrorType>::value);

                return *this;
            }

            constexpr operator bool() const { return stored.flags.has_value(); }

            // Stuff to get to the actual result value
            template <typename V = ValueType, typename E = ErrorType,
                      typename std::enable_if_t<!std::is_void<E>::value, int> = 0>
            ValueType& catch_error(std::function<ValueType(E&)> handler)
            {
                if (*this) return unwrap();             
                return std::forward<ValueType>(handler(stored.error));
            }

            ValueType& unwrap()
            {
                if (*this) {
                    stored.flags.clear_flags();
                    return *stored.value;
                }

                stored.clean();
                std::abort();
            }

            ValueType& value_or(ValueType& rval)
            {
                if (*this) return unwrap();
                else return rval;
            }
    };
}
/*
    Discarded snippets

    From class Basic_Result

            // Copy from other result
            constexpr Basic_Result(const Basic_Result& x)
            {
                stored.flags = x.stored.flags;
                if (*this) stored.value = x.value;
                else stored.error = x.error;
            }
            // Move from other result
            constexpr Basic_Result(Basic_Result&&) = default;
            constexpr Basic_Result& operator=(Basic_Result&&) = default;
*/

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
