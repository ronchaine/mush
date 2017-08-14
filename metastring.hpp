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

    template <size_t Size>
    struct simple_array
    {
        char values[Size+1];

        simple_array()
        {
            for (int i = 0; i < Size; ++i)
                values[i] = '\0';
        }
    };

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

    template <size_t A, size_t B>
    struct make_indices_range : expand_indices<A, B-1>::indices_type {};

    template <size_t A>
    struct make_indices_range<A, A> : indices<> {};

    template <metastring_detail::IntegralType Num>
    constexpr size_t get_num_size(Num n)
    {
        size_t digits = 0;
        while(n)
        {
            n /= 10;
            ++digits;
        }

        return digits;
    }

    template <ssize_t Integer, size_t IntLen, size_t... Is>
    constexpr char get_digits()
    {
    }

    template <metastring_detail::IntegralType Num>
    constexpr char nthdigit(Num x, int n)
    {
        while(n--)
            x /= 10;

        return (x % 10) + '0';
    }

    template <size_t N>
    class metastring
    {
        private:
            template <size_t I, size_t... Indices>
            constexpr metastring(const char(&str)[I], indices<Indices...>) : data{str[Indices]..., '\0'} {}

            template <size_t... Is, size_t... Js, size_t I>
            constexpr metastring<N+I-1> append(indices<Is...>, indices<Js...>, const char(&cstr)[I]) const
            {
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
                return append(make_indices<N>(), make_indices<I-1>(), cstr);
            }

            template <size_t I>
            constexpr metastring<N+I> append(const metastring<I>& mstr) const
            {
                return append(make_indices<N>(), make_indices<I>(), mstr.c_str());
            }

            constexpr metastring<N + 1> push_back(char c) const
            {
                return push_back(c, make_indices<N>());
            }
            
            friend inline std::ostream& operator<<(std::ostream& out, const metastring& val)
            {
                out << val.c_str();
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

    template <metastring_detail::IntegralType Integer, size_t... Indices, size_t Size = sizeof...(Indices)>
    constexpr simple_array<Size> create_array(Integer n, indices<Indices...>)
    {
        simple_array<Size> rval;

        for (size_t i = 0; i < Size; ++i)
            rval.values[Size-i-1] = nthdigit(n, i);

        return rval;
    }

    template <ssize_t Integer, size_t Size = get_num_size(Integer)>
    constexpr metastring<Size> integer_to_metastring()
    {
        simple_array<Size> a = create_array(Integer, make_indices<Size>());
        return metastring<Size>(a.values);
    }
}

#endif
