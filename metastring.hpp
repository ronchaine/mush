/** 
 * @file metastring.hpp
 * @brief Compile time string implementation
 * @author Jari Ronkainen
 * @version 1.0
 * @date 2017-08-22
 */
#ifndef MUSH_METASTRING_HEADER
#define MUSH_METASTRING_HEADER

#include <cstdint>
#include <memory>

namespace mush
{
    namespace metastring_detail { template <typename T> concept bool IntegralType = std::is_integral<T>::value; }

    template <size_t... Indices> struct indices {};

    template <typename A, typename B> struct concatenate_indices;
    template <size_t, size_t, typename = void> struct expand_indices;

    template <size_t... Is, size_t... Js>
    struct concatenate_indices<indices<Is...>, indices<Js...>>
    {
        using indices_type = indices<Is..., Js...>;
    };

    template <size_t A, size_t B>
    struct expand_indices<A, B, typename std::enable_if<A == B>::type>
    {
        using indices_type = indices<A>;
    };

    template <size_t A, size_t B>
    struct expand_indices<A, B, typename std::enable_if<A != B>::type>
    {
        static_assert(A < B, "A > B");
        using indices_type = typename concatenate_indices<
            typename expand_indices<A, (A + B) / 2>::indices_type,
            typename expand_indices<(A + B) / 2 + 1, B>::indices_type
        >::indices_type;
    };

    template <size_t A>
    struct make_indices : expand_indices<0, A-1>::indices_type {};

    template <>
    struct make_indices<0> : indices<> {};

    template <size_t A, size_t B>
    struct make_indices_range : expand_indices<A, B-1>::indices_type {};

    template <size_t A>
    struct make_indices_range<A, A> : indices<> {};

    template <metastring_detail::IntegralType Num>
    constexpr size_t get_num_size(Num n)
    {
        if (n == 0)
            return 1;

        size_t digits = 0;
        while(n)
        {
            n /= 10;
            ++digits;
        }

        return digits;
    }

    template <metastring_detail::IntegralType Num>
    constexpr char nthdigit(Num x, int n)
    {
        while(n--)
            x /= 10;

        return (x % 10) + '0';
    }
    
    template <char... Data>
    struct array_holder
    {
        constexpr static char data[sizeof...(Data)] = {Data...};
        constexpr static size_t size = sizeof...(Data);
    };

    constexpr metastring_detail::IntegralType abs_val(metastring_detail::IntegralType x)
    {
        return x < 0 ? -x : x; 
    }

    constexpr ssize_t digit_count(metastring_detail::IntegralType x)
    {
        return x < 0 ? 1 + digit_count(-x) : x < 10 ? 1 : 1 + digit_count(x/10);
    }

    template <size_t Size, ssize_t Num, char... Args>
    struct numeric_builder
    {
        typedef typename numeric_builder<Size - 1, Num / 10, '0' + abs_val(Num) % 10, Args...>::type type;
    };

    template <ssize_t Num, char... Args>
    struct numeric_builder<2, Num, Args...>
    {
        typedef array_holder<Num < 0 ? '-' : '0' + Num / 10, '0' + abs_val(Num) % 10, Args...> type;
    };

    template <ssize_t Num, char... Args>
    struct numeric_builder<1, Num, Args...>
    {
        typedef array_holder<'0' + Num, Args...> type;
    };

    template <ssize_t N>
    struct numeric_string
    {
        typedef typename numeric_builder<digit_count(N), N, '\0'>::type type;
        constexpr static type value {};
    };

    template<ssize_t N>
    constexpr typename numeric_string<N>::type numeric_string<N>::value;

    template <size_t N>
    class metastring
    {
        private:
            template <size_t I, size_t... Indices>
            constexpr metastring(const char(&str)[I], indices<Indices...>) : data{str[Indices]..., '\0'} {}

            template <size_t... Is, size_t... Js, size_t I>
            constexpr metastring<N+I-1> append(indices<Is...>, indices<Js...>, const char(&cstr)[I]) const
            {
                static_assert(I > 0);
                const char new_data[] = {data[Is]..., cstr[Js]..., '\0'};
                return metastring<N+I-1>(new_data);
            }

            template <size_t... Indices>
            constexpr metastring<N+1> push_back(char c, indices<Indices...>) const
            {
                const char new_data[] = {data[Indices]..., c, '\0'};
                return metastring<N+1>(new_data);
            }

        public:
            char data[N + 1];

            using data_type = const char(&)[N + 1];

            constexpr int size() const { return N; }
            constexpr data_type c_str() const { return data; }

            constexpr metastring(const char (&str)[N+1]) : metastring(str,make_indices<N>()) {}

            constexpr char& operator[](size_t idx) { return data[idx]; }

            template <size_t I>
            constexpr metastring<N+I-1> append(const char(&cstr)[I]) const
            {
                static_assert(I > 0);
                return append(make_indices<N>(), make_indices<I-1>(), cstr);
            }

            template <size_t I>
            constexpr metastring<N+I> append(const metastring<I>& mstr) const
            {
                static_assert(I >= 0);
                return append(make_indices<N>(), make_indices<I>(), mstr.c_str());
            }

            template <bool Enabled, size_t I>
            constexpr auto append_if(const char(&cstr)[I]) const
            {
                if constexpr (Enabled == false)
                {
                    return *this;
                } else {
                    static_assert(I > 0);
                    return append(make_indices<N>(), make_indices<I-1>(), cstr);
                }
            }
            
            template <bool Enabled, size_t I>
            constexpr auto append_if(const metastring<I>& mstr) const
            {
                if constexpr (Enabled == false)
                {
                    return *this;
                } else {
                    static_assert(I > 0);
                    return append(make_indices<N>(), make_indices<I>(), mstr.c_str());
                }
            }

            constexpr metastring<N + 1> push_back(char c) const
            {
                return push_back(c, make_indices<N>());
            }
            
            friend inline std::ostream& operator<<(std::ostream& out, const metastring& val)
            {
                out << val.c_str();

                return out;
            }
    };

    template <size_t N>
    constexpr auto make_string(const char(&cstr)[N])
    {
        return metastring<N-1>(cstr);
    }

    template <size_t I, size_t J>
    constexpr metastring<I+J> operator+(const metastring<I>& first, const metastring<J>& second)
    {
        return first.append(second);
    }

    template <ssize_t Integer, size_t Size = get_num_size(Integer)>
    constexpr metastring<Size> integer_to_metastring()
    {
        return make_string(numeric_string<Integer>::value.data);
    }
}

#endif
