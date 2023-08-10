#ifndef UTILITIES_H
#define UTILITIES_H

#include <cstdint>
#include <concepts>
#include <type_traits>

template<typename T>
    requires std::is_enum_v<T>
constexpr std::underlying_type_t<T> to_underlying_type(T value)
{
    return static_cast<std::underlying_type_t<T>>(value);
}

template<unsigned Alignment>
struct is_aligned_by
{
    template<std::convertible_to<std::uintptr_t> T>
    constexpr is_aligned_by(T pointer)
        : value(((std::uintptr_t)pointer & (Alignment - 1)) == 0)
    {}

    constexpr operator bool()
    {
        return value;
    }

    bool value;
};

template<typename... Ts>
void discard(Ts...) {}

#endif  // UTILITIES_H
