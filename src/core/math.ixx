module;

#include <type_traits>
#include <algorithm>
#include <cmath>

export module math;

import types;

export namespace math
{
    namespace constants
    {
        constexpr auto PI    = 3.1415926535897932f;  // PI (180 deg)

        constexpr auto SQRT2 = 1.4142135623730950f;  // square root of 2
        constexpr auto SQRT3 = 1.7320508075688773f;  // square root of 3
    }

    // use to compare floating point numbers for equality
    template<types::FloatingPoint T>
    [[nodiscard]]
    constexpr auto AlmostEqual(T a, T b) -> bool
    {
        // different epsilon for float and double
        constexpr bool isFloat = std::is_same_v<T, float_t>;
        const auto epsilon = isFloat ? 128 * FLT_EPSILON : 128 * DBL_EPSILON;
        const auto absTh  = isFloat ? FLT_MIN : DBL_MIN;

        if (a == b) return true;

        const auto diff = std::abs(a - b);
        const auto norm = std::min({std::abs(a + b), std::numeric_limits<T>::max()});
        return diff < std::max(absTh, epsilon * norm);
    }
}