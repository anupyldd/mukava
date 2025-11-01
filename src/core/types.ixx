module;

#include <type_traits>

export module types;

// contains utilities related to type info, templates, concepts, etc.
export namespace types
{
    // integral type, excludes char and bool
    template<typename T>
    concept Integer = std::is_integral_v<T>     &&
        !std::is_same_v<T,          bool    >   &&
        !std::is_same_v<T,          char    >   &&
        !std::is_same_v<T, signed   char    >   &&
        !std::is_same_v<T, unsigned char    >   &&
        !std::is_same_v<T,          char8_t >   &&
        !std::is_same_v<T,          char16_t>   &&
        !std::is_same_v<T,          char32_t>   &&
        !std::is_same_v<T,          wchar_t>;

    template<typename T>
    concept FloatingPoint = std::is_floating_point_v<T>;

    template<typename T>
    concept Number = Integer<T> || FloatingPoint<T>;


}