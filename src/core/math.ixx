module;

#include <type_traits>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <numeric>

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

    // returns true if ranges 'a' and 'b' overlap
    template<types::Number T>
    [[nodiscard]]
    constexpr auto Overlap(T aMin, T aMax, T bMin, T bMax) -> bool
    {
        assert(aMin <= aMax && "Invalid range: aMin > aMax");
        assert(bMin <= bMax && "Invalid range: bMin > bMax");
        return aMin <= bMax && aMax >= bMin;
    }

    // returns scaling factor from linear value
    template<types::FloatingPoint T>
    [[nodiscard]]
    constexpr auto ScalingFactor(T linValue) -> types::FloatOrDouble<T>
    {
        return (linValue >= 0) ? ( 1 + linValue) : (1 / (1 - linValue));
    }

    // returns linear value from scaling factor
    template<types::FloatingPoint T>
    [[nodiscard]]
    constexpr auto LinearValue(T scaleFactor) -> types::FloatOrDouble<T>
    {
        return (scaleFactor >= 1) ? (scaleFactor - 1) : ( 1 - (1 / scaleFactor));
    }

    // find average of two numbers
    template<types::Number T>
    [[nodiscard]]
    constexpr auto Average(T a, T b) -> types::FloatOrDouble<T>
    {
        return static_cast<types::FloatOrDouble<T>>(a + b) / static_cast<types::FloatOrDouble<T>>(2);
    }

    // find average of three numbers
    template<types::Number T>
    [[nodiscard]]
    constexpr auto Average(T a, T b, T c) -> types::FloatOrDouble<T>
    {
        return static_cast<types::FloatOrDouble<T>>(a + b + c) / static_cast<types::FloatOrDouble<T>>(3);
    }

    // find average value from a list
    template<types::Number T>
    [[nodiscard]]
    constexpr auto Average(std::initializer_list<T> vals) -> types::FloatOrDouble<T>
    {
        assert(!(vals.size() == 0) && "Empty list leads to division by zero");
        return static_cast<types::FloatOrDouble<T>>(
            std::accumulate(vals.begin(), vals.end(), 0.0) / vals.size());
    }
}

export namespace math
{
    template<types::Number>
    struct Vector2;

    template<types::Number>
    struct Vector3;

    template<types::Number>
    struct Vector4;

    template<types::Number T>
    struct Vector2
    {
        T x, y;

        // constructs a vector with x == y == 0
        Vector2() : x(static_cast<T>(0), static_cast<T>(0)) {}
        // constructs a vector from 2 values
        Vector2(T x, T y) : x(x), y(y) {}
        // constructs a vector with x == y == val
        Vector2(T val) : x(val), y(val) {}
        // copy-constructs a vector
        Vector2(const Vector2<T>& src) : x(src.x), y(src.y) {}

        // sets x and y to new values
        auto Set(T newX, T newY) -> Vector2<T>& { x = newX; y = newY; return *this; }
        // sets both x and y to val
        auto Set(T val) -> Vector2<T>& { x = y = val; return *this; }
        // sets both x and y to 0
        auto Zero() -> Vector2<T>& { x = y = 0; return *this; }
        // normalizes the vector in place
        auto Normalize() -> Vector2<T>&
        {
            using Flt = types::FloatOrDouble<T>;
            // cannot normalize vector (0, 0)
            if (AlmostEqual(x, static_cast<Flt>(0.0)) &&
                AlmostEqual(y, static_cast<Flt>(0.0))) return *this;
            Flt mag = Magnitude();
            x /= mag;
            y /= mag;
            return *this;
        }

        // returns the squared magnitude of the vector
        [[nodiscard]]
        auto MagnitudeSq() -> types::FloatOrDouble<T> { return x * x + y * y; }
        // normalizes the vector
        [[nodiscard]]
        auto Magnitude() -> types::FloatOrDouble<T> { return std::sqrt(MagnitudeSq()); }
        // returns Vector2(y, x)
        [[nodiscard]]
        auto YX() -> Vector2<T>& { return Vector2<T>(y, x); }
        // returns true if either x or y are not 0
        [[nodiscard]]
        auto Any() -> bool { return x || y; }
        // returns the smaller component
        [[nodiscard]]
        auto Min() -> T { return x < y ? x : y; }
        // returns the larger component
        [[nodiscard]]
        auto Max() -> T { return x > y ? x : y; }
        // returns the average of components
        [[nodiscard]]
        auto Average() -> types::FloatOrDouble<T> { return math::Average(x, y); }
        // returns true if x and y are different
        [[nodiscard]]
        auto Different() -> bool { return x != y; }
    };
}