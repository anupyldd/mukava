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

    // compares floating point numbers for equality
    template<types::FloatingPoint T>
    [[nodiscard]]
    constexpr auto AlmostEqual(T a, T b) noexcept -> bool
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

    // compares any numbers including float and double
    template<typename T>
    [[nodiscard]]
    constexpr auto NumericEqual(T a,T b) -> bool
    {
        if constexpr (types::FloatingPoint<T>)
            return AlmostEqual<T>(a, b);
        return (a == b);
    }

    // returns true if ranges 'a' and 'b' overlap
    template<types::Number T>
    [[nodiscard]]
    constexpr auto Overlap(T aMin, T aMax, T bMin, T bMax) noexcept -> bool
    {
        assert(aMin <= aMax && "Invalid range: aMin > aMax");
        assert(bMin <= bMax && "Invalid range: bMin > bMax");
        return aMin <= bMax && aMax >= bMin;
    }

    // returns scaling factor from linear value
    template<types::FloatingPoint T>
    [[nodiscard]]
    constexpr auto ScalingFactor(T linValue) noexcept -> types::FloatOrDouble<T>
    {
        return (linValue >= 0) ? (1 + linValue) : (1 / (1 - linValue));
    }

    // returns linear value from scaling factor
    template<types::FloatingPoint T>
    [[nodiscard]]
    constexpr auto LinearValue(T scaleFactor) noexcept -> types::FloatOrDouble<T>
    {
        return (scaleFactor >= 1) ? (scaleFactor - 1) : ( 1 - (1 / scaleFactor));
    }

    // find average of two numbers
    template<types::Number T>
    [[nodiscard]]
    constexpr auto Average(T a, T b) noexcept -> types::FloatOrDouble<T>
    {
        return static_cast<types::FloatOrDouble<T>>(a + b) / static_cast<types::FloatOrDouble<T>>(2);
    }

    // find average of three numbers
    template<types::Number T>
    [[nodiscard]]
    constexpr auto Average(T a, T b, T c) noexcept -> types::FloatOrDouble<T>
    {
        return static_cast<types::FloatOrDouble<T>>(a + b + c) / static_cast<types::FloatOrDouble<T>>(3);
    }

    // find average value from a list
    template<types::Number T>
    [[nodiscard]]
    constexpr auto Average(std::initializer_list<T> vals) noexcept -> types::FloatOrDouble<T>
    {
        assert(!(vals.size() == 0) && "Empty list leads to division by zero");
        return static_cast<types::FloatOrDouble<T>>(
            std::accumulate(vals.begin(), vals.end(), 0.0) / vals.size());
    }

    // returns true if any of the values is not 0
    template<types::Number T>
    constexpr auto Any(std::initializer_list<T> vals) noexcept -> bool
    {
        return std::any_of(vals.begin(), vals.end(),
            [](T val) { return val != static_cast<T>(0); });
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
        Vector2() : x(static_cast<T>(0)), y(static_cast<T>(0)) {}
        // constructs a vector from 2 values
        explicit Vector2(T x, T y) : x(x), y(y) {}
        // constructs a vector with x == y == val
        explicit Vector2(T val) : x(val), y(val) {}
        // copy-constructs a vector
        Vector2(const Vector2& src) : x(src.x), y(src.y) {}

        // sets x and y to new values
        auto Set(T newX, T newY) noexcept -> Vector2& { x = newX; y = newY; return *this; }
        // sets both x and y to val
        auto Set(T val) noexcept -> Vector2& { x = y = val; return *this; }
        // sets both x and y to 0
        auto Zero() noexcept -> Vector2& { x = y = 0; return *this; }
        // normalizes the vector in place
        // not accessible for int-based vectors
        template<typename U = T, typename = std::enable_if_t<types::FloatingPoint<U>>>
        auto Normalize() noexcept -> Vector2&
        {
            using Flt = types::FloatOrDouble<T>;

            // cannot normalize vector (0, 0)
            if (AlmostEqual(x, static_cast<Flt>(0.0)) &&
                AlmostEqual(y, static_cast<Flt>(0.0))) [[unlikely]] return *this;

            const Flt mag = Magnitude();
            x /= mag;
            y /= mag;
            return *this;
        }

        // returns the squared magnitude of the vector
        [[nodiscard]]
        auto MagnitudeSq() const noexcept -> types::FloatOrDouble<T> { return x * x + y * y; }
        // normalizes the vector
        [[nodiscard]]
        auto Magnitude() const noexcept -> types::FloatOrDouble<T> { return std::sqrt(MagnitudeSq()); }
        // returns Vector2(y, x)
        [[nodiscard]]
        auto YX() const noexcept -> Vector2 { return Vector2(y, x); }
        // returns true if either x or y are not 0
        [[nodiscard]]
        auto Any() const noexcept -> bool { return math::Any({ x, y }); }
        // returns the smaller component
        [[nodiscard]]
        auto Min() const noexcept -> T { return x < y ? x : y; }
        // returns the larger component
        [[nodiscard]]
        auto Max() const noexcept -> T { return x > y ? x : y; }
        // returns the average of components
        [[nodiscard]]
        auto Average() const noexcept -> types::FloatOrDouble<T> { return math::Average(x, y); }
        // returns true if x and y are different
        [[nodiscard]]
        auto Different() const noexcept -> bool { return x != y; }
        // returns dot product with another vector
        [[nodiscard]]
        auto Dot(const Vector2& other) const noexcept -> types::FloatOrDouble<T> { return x * other.x + y * other.y; }

        auto operator += (T rhs) -> Vector2& { x += rhs; y += rhs; return *this; }
        auto operator -= (T rhs) -> Vector2& { x -= rhs; y -= rhs; return *this; }
        auto operator *= (T rhs) -> Vector2& { x *= rhs; y *= rhs; return *this; }
        auto operator /= (T rhs) -> Vector2& { x /= rhs; y /= rhs; return *this; }

        auto operator += (const Vector2& rhs) -> Vector2& { x += rhs.x; y += rhs.y; return *this; }
        auto operator -= (const Vector2& rhs) -> Vector2& { x -= rhs.x; y -= rhs.y; return *this; }
        auto operator *= (const Vector2& rhs) -> Vector2& { x *= rhs.x; y *= rhs.y; return *this; }
        auto operator /= (const Vector2& rhs) -> Vector2& { x /= rhs.x; y /= rhs.y; return *this; }

        auto operator == (const Vector2& rhs) -> bool { return NumericEqual(x, rhs.x) && NumericEqual(y, rhs.y); }
        auto operator != (const Vector2& rhs) -> bool { return !((*this) == rhs); }
        auto operator >  (const Vector2& rhs) -> bool { return MagnitudeSq() >  rhs.MagnitudeSq(); }
        auto operator >= (const Vector2& rhs) -> bool { return MagnitudeSq() >= rhs.MagnitudeSq(); }
        auto operator <  (const Vector2& rhs) -> bool { return MagnitudeSq() <  rhs.MagnitudeSq(); }
        auto operator <= (const Vector2& rhs) -> bool { return MagnitudeSq() <= rhs.MagnitudeSq(); }

        friend auto operator + (const Vector2& lhs, T rhs) noexcept -> Vector2
        {
            return Vector2{lhs.x + rhs, lhs.y + rhs};
        }
        friend auto operator - (const Vector2& lhs, T rhs) noexcept -> Vector2
        {
            return Vector2{lhs.x - rhs, lhs.y - rhs};
        }
        friend auto operator * (const Vector2& lhs, T rhs) noexcept -> Vector2
        {
            return Vector2{lhs.x * rhs, lhs.y * rhs};
        }
        friend auto operator / (const Vector2& lhs, T rhs) noexcept -> Vector2
        {
            assert(rhs != static_cast<T>(0) && "Cannot divide by 0");
            return Vector2{lhs.x / rhs, lhs.y / rhs};
        }

        friend auto operator + (const Vector2& lhs, const Vector2& rhs) noexcept -> Vector2
        {
            return Vector2{lhs.x + rhs.x, lhs.y + rhs.y};
        }
        friend auto operator - (const Vector2& lhs, const Vector2& rhs) noexcept -> Vector2
        {
            return Vector2{lhs.x - rhs.x, lhs.y - rhs.y};
        }
        friend auto operator * (const Vector2& lhs, const Vector2& rhs) noexcept -> Vector2
        {
            return Vector2{lhs.x * rhs.x, lhs.y * rhs.y};
        }
        friend auto operator / (const Vector2& lhs, const Vector2& rhs) noexcept -> Vector2
        {
            assert(rhs.x != static_cast<T>(0) && "Cannot divide by 0");
            assert(rhs.y != static_cast<T>(0) && "Cannot divide by 0");
            return Vector2{lhs.x / rhs.x, lhs.y / rhs.y};
        }
    };

    // 2d vector of ints
    using Vector2i = Vector2<int>;
    // 2d vector of floats
    using Vector2f = Vector2<float>;
    // 2d vector of doubles
    using Vector2d = Vector2<double>;

    template<types::Number T>
    struct Vector3
    {
        T x, y, z;

        // constructs a vector with x == y == z == 0
        Vector3() : x(static_cast<T>(0)), y(static_cast<T>(0)), z(static_cast<T>(0)) {}
        // constructs a vector from 3 values
        explicit Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
        // constructs a vector with x == y == z == val
        explicit Vector3(T val) : x(val), y(val), z(val) {}
        // copy-constructs a vector
        Vector3(const Vector3& src) : x(src.x), y(src.y), z(src.z) {}

        // sets x, y and z to new values
        auto Set(T newX, T newY, T newZ) noexcept -> Vector3& { x = newX; y = newY; z = newZ; return *this; }
        // sets x, y and z to val
        auto Set(T val) noexcept -> Vector3& { x = y = z = val; return *this; }
        // sets x, y and z to 0
        auto Zero() noexcept -> Vector3& { x = y = z = 0; return *this; }
        // normalizes the vector in place
        // not accessible for int-based vectors
        template<typename U = T, typename = std::enable_if_t<types::FloatingPoint<U>>>
        auto Normalize() noexcept -> Vector3&
        {
            using Flt = types::FloatOrDouble<T>;

            // cannot normalize vector (0, 0)
            if (AlmostEqual(x, static_cast<Flt>(0.0)) &&
                AlmostEqual(y, static_cast<Flt>(0.0))) [[unlikely]] return *this;

            const Flt mag = Magnitude();
            x /= mag;
            y /= mag;
            z /= mag;
            return *this;
        }

        // returns the squared magnitude of the vector
        [[nodiscard]]
        auto MagnitudeSq() const noexcept -> types::FloatOrDouble<T> { return x * x + y * y + z * z; }
        // normalizes the vector
        [[nodiscard]]
        auto Magnitude() const noexcept -> types::FloatOrDouble<T> { return std::sqrt(MagnitudeSq()); }
        // returns true if any of components is not 0
        [[nodiscard]]
        auto Any() const noexcept -> bool { return math::Any({ x, y, z }); }
        // returns the smaller component
        [[nodiscard]]
        auto Min() const noexcept -> T { return std::min({x, y, z}); }
        // returns the larger component
        [[nodiscard]]
        auto Max() const noexcept -> T { return std::max({x, y, z}); }
        // returns the average of components
        [[nodiscard]]
        auto Average() const noexcept -> types::FloatOrDouble<T> { return math::Average(x, y, z); }
        // returns true if x and y are different
        [[nodiscard]]
        auto Different() const noexcept -> bool { return (x != y) || (x != z) || (y != z); }
        // returns dot product with another vector
        [[nodiscard]]
        auto Dot(const Vector3& other) const noexcept -> types::FloatOrDouble<T> { return x * other.x + y * other.y + z * other.z; }

        auto operator += (T rhs) -> Vector3& { x += rhs; y += rhs; z += rhs; return *this; }
        auto operator -= (T rhs) -> Vector3& { x -= rhs; y -= rhs; z -= rhs; return *this; }
        auto operator *= (T rhs) -> Vector3& { x *= rhs; y *= rhs; z *= rhs; return *this; }
        auto operator /= (T rhs) -> Vector3& { x /= rhs; y /= rhs; z /= rhs; return *this; }

        auto operator += (const Vector3& rhs) -> Vector3& { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        auto operator -= (const Vector3& rhs) -> Vector3& { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        auto operator *= (const Vector3& rhs) -> Vector3& { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
        auto operator /= (const Vector3& rhs) -> Vector3& { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

        auto operator == (const Vector3& rhs) -> bool { return NumericEqual(x, rhs.x) && NumericEqual(y, rhs.y) && NumericEqual(z, rhs); }
        auto operator != (const Vector3& rhs) -> bool { return !((*this) == rhs); }
        auto operator >  (const Vector3& rhs) -> bool { return MagnitudeSq() >  rhs.MagnitudeSq(); }
        auto operator >= (const Vector3& rhs) -> bool { return MagnitudeSq() >= rhs.MagnitudeSq(); }
        auto operator <  (const Vector3& rhs) -> bool { return MagnitudeSq() <  rhs.MagnitudeSq(); }
        auto operator <= (const Vector3& rhs) -> bool { return MagnitudeSq() <= rhs.MagnitudeSq(); }

        friend auto operator + (const Vector3& lhs, T rhs) noexcept -> Vector3
        {
            return Vector3{lhs.x + rhs, lhs.y + rhs, lhs.z + rhs};
        }
        friend auto operator - (const Vector3& lhs, T rhs) noexcept -> Vector3
        {
            return Vector3{lhs.x - rhs, lhs.y - rhs, lhs.z - rhs};
        }
        friend auto operator * (const Vector3& lhs, T rhs) noexcept -> Vector3
        {
            return Vector3{lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
        }
        friend auto operator / (const Vector3& lhs, T rhs) noexcept -> Vector3
        {
            assert(rhs != static_cast<T>(0) && "Cannot divide by 0");
            return Vector3{lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
        }

        friend auto operator + (const Vector3& lhs, const Vector3& rhs) noexcept -> Vector3
        {
            return Vector3{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
        }
        friend auto operator - (const Vector3& lhs, const Vector3& rhs) noexcept -> Vector3
        {
            return Vector3{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
        }
        friend auto operator * (const Vector3& lhs, const Vector3& rhs) noexcept -> Vector3
        {
            return Vector3{lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
        }
        friend auto operator / (const Vector3& lhs, const Vector3& rhs) noexcept -> Vector3
        {
            assert(rhs.x != static_cast<T>(0) && "Cannot divide by 0");
            assert(rhs.y != static_cast<T>(0) && "Cannot divide by 0");
            assert(rhs.z != static_cast<T>(0) && "Cannot divide by 0");
            return Vector3{lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z};
        }
    };

    // 3d vector of ints
    using Vector3i = Vector3<int>;
    // 3d vector of floats
    using Vector3f = Vector3<float>;
    // 3d vector of doubles
    using Vector3d = Vector3<double>;
}