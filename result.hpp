#ifndef MUSH_RESULT_HPP
#define MUSH_RESULT_HPP

#include <memory>
#include <functional>

namespace mush
{
    template <typename T>
    concept bool SuitableError = requires(T a)
    {
        {
           // a.msg()
            true
        }
    };
    
    struct Basic_Error
    {
    };

    // Result for non-void non-reference type
    template <typename ValueType, SuitableError ErrorType, bool IsReference>
    class Basic_Result
    {
        private:
            ValueType value;
            ErrorType error;

            bool has_result;

        public:
//            constexpr Basic_Result(ErrorType err) { has_result = false; error = err; }
//            constexpr Basic_Result(ValueType val) { has_result = true;  value = val; }

            // default constructor
            Basic_Result() : has_result(false) {}

            // Create from result
            // Basic_Result(Basic_Result&& rhs) = default;
            Basic_Result(const Basic_Result& rhs) : has_result(rhs.has_result)
            {
                if (has_result) value = rhs.value;
                else error = rhs.error;
            }

            // create from values
            Basic_Result(const ValueType& rhs) : value(rhs), has_result(true) {}
            Basic_Result(ValueType&& rhs) : value(std::forward<ValueType>(rhs)), has_result(true) {}

            // create from errors
            Basic_Result(const ErrorType& err) : error(err), has_result(false) {}
            Basic_Result(ErrorType&& err) : error(std::forward<ErrorType>(err)), has_result(false) {}

            // assignments
            Basic_Result& operator=(const ValueType& rhs)
            {
                has_result = true;
                value = rhs;
                return *this;
            }
            Basic_Result& operator=(ValueType&& rhs)
            {
                has_result = true;
                value = std::forward(rhs);
                return *this;
            }
            //Basic_Result& operator=(Result rhs)

            // conversions
            operator bool() { return has_result; }

            // matching
            ValueType&& match(std::function<ValueType(ErrorType&)> handler)
            {
                if (has_result)
                {
                    has_result = false;
                    return std::move(value);
                }

                return handler(error);
            }
    };
    
    // Result for reference type
    template <typename ValueType, SuitableError ErrorType>
    class Basic_Result<ValueType, ErrorType, true>
    {
        private:
            typename std::remove_reference<ValueType>::type* value;
            ErrorType error;

            bool has_result;

        public:
            constexpr Basic_Result(ErrorType err) { has_result = false; error = err; }
            constexpr Basic_Result(ValueType val) { has_result = true;  value = val; }

            // default constructor
            Basic_Result() : has_result(false) {}

            // results
            Basic_Result(const Basic_Result& rhs) : has_result(rhs.has_result)
            {
                if (has_result) value = rhs.value;
                else error = rhs.error;
            }

            // values
            Basic_Result(const ValueType& rhs) : value(rhs), has_result(true) {}
            Basic_Result(ValueType&& rhs) : value(std::forward<ValueType>(rhs)), has_result(true) {}

            // create from errors
            Basic_Result(const ErrorType& err) : error(err), has_result(false) {}
            Basic_Result(ErrorType&& err) : error(std::forward<ErrorType>(err)), has_result(false) {}

            Basic_Result& operator=(const ValueType rhs)
            {
                has_result = true;
                value = &rhs;
            }
            //Basic_Result& operator=(Basic_Result rhs) = default

            operator bool() { return has_result; }
    };

    // Result for void type
    template <SuitableError ErrorType>
    class Basic_Result<void, ErrorType, false>
    {
        private:
            ErrorType error;
            bool has_result;

        public:
            constexpr Basic_Result(bool) : error(), has_result(true) {}
            constexpr Basic_Result(bool, ErrorType err) : error(err), has_result(false) {}

//            constexpr static Basic_Result<void, ErrorType, false> OK = Basic_Result(true);

            Basic_Result() : has_result(false) {}
            
            Basic_Result(const ErrorType& err) : error(err), has_result(false) {}
            Basic_Result(ErrorType&& err) : error(std::move(err)), has_result(false) {}
            
            void match(std::function<void(ErrorType&)> handler)
            {
                if (has_result)
                    return;

                return handler(error);
            }
    };

    template <typename ValueType, SuitableError ErrorType, bool IsReference>
    Basic_Result<ValueType, ErrorType, IsReference> make_result(bool)
    {
    }

    template <typename ValueType, bool IsReference = std::is_reference<ValueType>::value>
    using Result = Basic_Result<ValueType, Basic_Error, IsReference>;
}
#endif
