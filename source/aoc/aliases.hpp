#pragma once

#include <cstdint>
#include <span>
#include <string_view>
#include <variant>

namespace aoc::aliases
{
    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using usize = std::size_t;
    using isize = std::ptrdiff_t;

    using f32 = float;
    using f64 = double;

    using unit = std::monostate;

    using Lines = std::span<const std::string_view>;

    // I don't know where to place this tbh, but since `aoc::concepts::Day` require this to be defined
    // beforehand, may as well place it here alongside `Lines` which is also required to be defined beforehand
    struct Context
    {
        bool m_debug;
        bool m_benchmark;

        bool is_debug() const noexcept { return m_debug; }
        bool is_benchmark() const noexcept { return m_benchmark; }
    };

    // like std::identity but instead of returning the arguments, unchanging, this function consumes the
    // arguments and ignore them, returning a unit instead
    struct Consume
    {
        template <typename... T>
        unit operator()(T&&...) const noexcept
        {
            return unit{};
        }
    };

    template <typename T, typename Mem>
    struct Proj
    {
        Mem T::*      m_mem;
        constexpr Mem operator()(const T& t) const noexcept { return t.*m_mem; }
    };
}

namespace aoc::inline literals
{
    // clang-format off
    constexpr aliases::i8  operator""_i8 (unsigned long long value) { return static_cast<aliases::i8 >(value); }
    constexpr aliases::i16 operator""_i16(unsigned long long value) { return static_cast<aliases::i16>(value); }
    constexpr aliases::i32 operator""_i32(unsigned long long value) { return static_cast<aliases::i32>(value); }
    constexpr aliases::i64 operator""_i64(unsigned long long value) { return static_cast<aliases::i64>(value); }

    constexpr aliases::u8  operator""_u8 (unsigned long long value) { return static_cast<aliases::u8 >(value); }
    constexpr aliases::u16 operator""_u16(unsigned long long value) { return static_cast<aliases::u16>(value); }
    constexpr aliases::u32 operator""_u32(unsigned long long value) { return static_cast<aliases::u32>(value); }
    constexpr aliases::u64 operator""_u64(unsigned long long value) { return static_cast<aliases::u64>(value); }

    constexpr aliases::usize operator""_usize(unsigned long long value) { return static_cast<aliases::usize>(value); }
    constexpr aliases::isize operator""_isize(unsigned long long value) { return static_cast<aliases::isize>(value); }

    constexpr aliases::f32 operator""_f32(long double value) { return static_cast<aliases::f32>(value); }
    constexpr aliases::f64 operator""_f64(long double value) { return static_cast<aliases::f64>(value); }
    // clang-format on
}
