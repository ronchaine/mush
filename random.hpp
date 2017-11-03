#ifndef MUSH_RANDOM
#define MUSH_RANDOM

#include <random>

namespace mush
{
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
