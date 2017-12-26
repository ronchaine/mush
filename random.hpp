#ifndef MUSH_RANDOM
#define MUSH_RANDOM

#include <random>

namespace mush
{
    template <typename ResultType>
    class Distribution
    {
        typedef ResultType  result_type;

        private:
            template <bool Integers = std::is_integral<ResultType>::value>
            class bounds_type
            {
                result_type a;
                result_type b;

                public:
                    typedef Distribution distribution_type;

                    template <bool Enable = Integers, std::enable_if_t<Enable, int> = 0>
                    explicit bounds_type(result_type a = 0, result_type b = std::numeric_limits<result_type>::max())
                        : a(a), b(b) {}
                    
                    template <bool Enable = Integers, std::enable_if_t<!Enable, int> = 0>
                    explicit bounds_type(result_type a = 0.0, result_type b = 1.0)
                        : a(a), b(b) {}

                    result_type min() const { return a; }
                    result_type max() const { return b; }
            };
        public:
    };

    namespace detail
    {
        template <typename Generator = std::mt19937>
        Generator rng;
    }

    template <typename Generator = std::mt19937>
    void seed_random(uint32_t seed = std::random_device()())
    {
        detail::rng<Generator>.seed(seed);
    }

    template <typename T,
              typename Generator = std::mt19937,
              typename Distribution = std::conditional_t<
                    std::is_floating_point<T>::value,
                    std::uniform_real_distribution<T>,
                    std::uniform_int_distribution<T>>
             >
    T random(T min, T max)
    {
        return Distribution(min, max)(detail::rng<Generator>);
    }
}

#endif
