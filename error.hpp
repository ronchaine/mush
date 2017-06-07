#ifndef MUSH_ERROR_HPP
#define MUSH_ERROR_HPP

#include <cstdint>
#include <string>
#include <atomic>
#include <thread>
#include <unordered_map>

#include <cassert>

using ErrorIndex = uint32_t;
using ErrorMap = std::unordered_map<ErrorIndex, std::string>;

#include <iostream>

// builtin error types
struct err_uninitialised_value { constexpr static auto explanation = "usage of uninitialised value"; };
struct err_invalid_argument { constexpr static auto explanation = "unaccepted argument value"; };
struct err_out_of_range { constexpr static auto explanation = "access out of range"; };
struct err_overflow { constexpr static auto explanation = "value overflow"; };
struct err_underflow { constexpr static auto explanation = "value underflow"; };
struct err_resource_unavailable { constexpr static auto explanation = "resource unavailable"; };

struct err_logic { constexpr static auto explanation = "logic error"; };
struct err_undefined { constexpr static auto explanation = "undefined error"; };
struct err_blameothers { constexpr static auto explanation = "library error"; };

// FIXME: Remove #ifdefs once clang supports concepts
#ifndef __clang__
template <typename T>
concept bool ErrorType = requires(T)
{
    {
        T::explanation
    }
};
#else
#define ErrorType typename
#endif

namespace internal
{
    static std::atomic<ErrorIndex> ETCounter;
    static ErrorMap errormap;

    template<uint32_t Thread = 0>
    static std::string errormsg;
}

template <ErrorType T>
auto explain()
{
    return T::explanation;
}

template <ErrorType T>
inline ErrorIndex get_error_type()
{
    static ErrorIndex id = ++internal::ETCounter;
    assert(id != 0);

    if (internal::errormap.find(id) == internal::errormap.end())
        internal::errormap[id] = explain<T>();
    return id;
}

template <>
inline constexpr ErrorIndex get_error_type<err_undefined>()
{
    return 0;
}

struct Error
{
    private:
        ErrorIndex value;

    public:
        constexpr Error(bool) : value(get_error_type<err_undefined>()) {}
        Error(ErrorIndex v) : value(v) {}
        Error() : value(0) {}

        const ErrorIndex type() const { return value; }
        const char* what() const {
            if (internal::errormap.find(0) == internal::errormap.end())
                internal::errormap[0] = explain<err_undefined>();

            return internal::errormap[value].c_str();
        }
};

template <ErrorType T>
struct create_error : public Error
{
    create_error(const char* msg = "") : Error(get_error_type<T>())
    {
        if (internal::errormap.count(get_error_type<T>()) == 0)
            internal::errormap[get_error_type<T>()] = explain<T>();

        internal::errormsg<0> = msg;
    }
};

static inline const char* get_error_message() { return internal::errormsg<0>.c_str(); }

#endif
