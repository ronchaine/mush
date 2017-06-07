#ifndef MUSH_RESULT_HPP
#define MUSH_RESULT_HPP

#include <memory>
#include <functional>

#include "error.hpp"

class EmptyClass {};

template <bool B, class T = EmptyClass>
struct derive_if {};

template <class T>
struct derive_if<true, T> : public T {};

template <typename Value,
          bool IsReference = std::is_reference<Value>::value>
          //typename Base = derive_if<!std::is_fundamental<Value>::value, Value>>
class Result// : public Base
{
    private:
        Value value;
        Error error;
        
        bool has_result;

    public:
        Result() : error(get_error_type<err_uninitialised_value>()), has_result(false) {}

        Result(const Value& rhs) : value(rhs), has_result(true) {}
        Result(Value&& rhs) : value(std::move(rhs)), has_result(true) {}
        Result(const Result& rhs) : has_result(rhs.has_result)
        {
            if (has_result) new (&value) Value(rhs.value);
            else new (&error) Error(rhs.error);
        } 

        Result(Result&& rhs) : has_result(rhs.has_result)
        {
            if (has_result) new (&value) Value(std::move(rhs.value));
            else new (&error) Error(std::move(rhs.error));
        }

        Result(const Error& err) : error(err), has_result(false) {}
        Result(Error&& err) : error(std::move(err)), has_result(false) {}

        Result& operator=(const Value& rhs)
        {
            has_result = true;
            value = rhs;

            return *this;
        }

        Result& operator=(Value&& rhs)
        {
            has_result = true;
            value = std::move(rhs);

            return *this;
        }

        Result& operator=(Result rhs)
        {
            rhs.swap(*this);
            return *this;
        }

        ~Result()
        {
            if (has_result)
                value.~Value();
            else
                error.~Error();
        }

        operator bool()
        {
            return has_result;
        }
        
        // Rust unwrap()
        operator Value&()
        {
            // use unwrap() with nontrivial types
            static_assert(std::is_trivial<Value>::value && "Non-trivial types require you to use unwrap()");
            if (has_result)
                return value;
            else
            {
                std::cout << "aborted trying to use invalid value: " << error.what() << "\n";
                std::abort();
            }
        }

        // Rust unwrap() for constants
        operator const Value&() const
        {
            // use unwrap() with nontrivial types
            static_assert(std::is_trivial<Value>::value && "Non-trivial types require you to use unwrap()");
            if (has_result)
                return value;
            else
            {
                std::cout << "aborted trying to use invalid value: " << error.what() << "\n";
                std::abort();
            }
        }

        bool is_ok() const { return has_result; }
        bool is_err() const { return !has_result; }

        const Error& get_error() const { return error; }

        Value match(std::function<Value(Error&)> handler)
        {
            if (has_result)
                return value;

            return handler(error);
        }

        Value value_or(Value v)
        {
            if (has_result)
                return value;
            
            return v;
        }

        Value unwrap()
        {
            if (has_result)
                return std::move(value);
            else
            {
                std::cout << "aborted trying to use invalid value: " << error.what() << "\n";
                std::abort();
            }
        }
        
        void swap(Result& rhs)
        {
            if (has_result)
            {
                if (rhs.has_result)
                {
                    std::swap(value, rhs.value);
                } else {
                    auto t = std::move(rhs.error);
                    new(&rhs.value) Value(std::move(value));
                    new(&error) Error(t);
                    std::swap(value, rhs.value);
                    std::swap(has_result, rhs.has_result);
                }
            } else {
                if (rhs.has_result)
                {
                    rhs.swap(*this);
                } else {
                    std::swap(error, rhs.error);
                    std::swap(value, rhs.value);
                }
            }
        }
};

// Result for reference types
template <typename Value>
class Result<Value, true>
{
    private:
        typename std::remove_reference<Value>::type* value;
        Error error;

        bool has_result;
        
    public:
        Result() : error(get_error_type<err_uninitialised_value>()), has_result(false) {}
        
        Result(const Value rhs) : value(&rhs), has_result(true) {}
        
        Result(const Result& rhs) : has_result(rhs.has_result)
        {
            if (has_result) value = rhs.value;
            else new (&error) Error(rhs.error);
        } 
        
        Result(const Error& err) : error(err), has_result(false) {}
        Result(Error&& err) : error(std::move(err)), has_result(false) {}

        Result& operator=(const Value rhs)
        {
            has_result = true;
            value = &rhs;

            return *this;
        }

        Result& operator=(Result rhs)
        {
            rhs.swap(*this);
            return *this;
        }

        ~Result()
        {
            if (!has_result)
                error.~Error();
        }

        operator bool()
        {
            return has_result;
        }

        // Rust unwrap()
        operator Value&()
        {
            if (has_result)
                return *value;
            else
            {
                std::cout << "aborted trying to use invalid value: " << error.what() << "\n";
                std::abort();
            }
        }

        // Rust unwrap() for constants
        operator const Value&() const
        {
            if (has_result)
                return *value;
            else
            {
                std::cout << "aborted trying to use invalid value: " << error.what() << "\n";
                std::abort();
            }
        }

        bool is_ok() const { return has_result; }
        bool is_err() const { return !has_result; }

        const Error& get_error() const { return error; }

        Value match(std::function<Value(Error&)> handler)
        {
            if (has_result)
                return *value;

            return handler(error);
        }

        Value value_or(Value v)
        {
            if (has_result)
                return *value;
            
            return v;
        }

        Value& unwrap()
        {
            if (has_result)
                return *value;
            else
            {
                std::cout << "aborted trying to use invalid value: " << error.what() << "\n";
                std::abort();
            }
        }

        void swap(Result& rhs)
        {
            if (has_result)
            {
                if (rhs.has_result)
                {
                    std::swap(value, rhs.value);
                } else {
                    auto t = std::move(rhs.error);
                    new(&rhs.value) Value(std::move(value));
                    new(&error) Error(t);
                    std::swap(value, rhs.value);
                    std::swap(has_result, rhs.has_result);
                }
            } else {
                if (rhs.has_result)
                {
                    rhs.swap(*this);
                } else {
                    std::swap(error, rhs.error);
                    std::swap(value, rhs.value);
                }
            }
        }
};

template <>
class Result<void>
{
    private:
        Error error;
        
        bool has_result;

        Result() : has_result(true) {}

    public:
        constexpr Result(const char* msg, bool value = false) : error(false), has_result(value) {
            internal::errormsg<0> = msg;
        }
        
        Result(const Result& rhs) : has_result(rhs.has_result)
        {
            if (!has_result) new (&error) Error(rhs.error);
        } 
        Result(Result&& rhs) : has_result(rhs.has_result)
        {
            if (!has_result) new (&error) Error(std::move(rhs.error));
        }

        Result(const Error& err) : error(err), has_result(false) {}
        Result(Error&& err) : error(std::move(err)), has_result(false) {}

        ~Result()
        {
            if (!has_result)
                error.~Error();
        }
        
        bool is_ok() const { return has_result; }
        bool is_err() const { return !has_result; }
        
        Result<void>& match(std::function<void(Error&)> handler)
        {
            if (has_result)
                return *this;

            handler(error);
            has_result = true;
            return *this;
        }
};

const static auto result_ok = Result<void>("no error", true);

#endif
