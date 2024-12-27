#pragma once

#include <fmt/base.h>

#include <array>
#include <cstdint>
#include <utility>

namespace aoc::util
{
    template <typename T>
    struct Coordinate
    {
        using Type = T;

        constexpr auto operator<=>(const Coordinate&) const = default;

        constexpr Coordinate operator*(const T& r) const { return { m_x * r, m_y * r }; }
        constexpr Coordinate operator/(const T& r) const { return { m_x / r, m_y / r }; }

        constexpr Coordinate operator+(const Coordinate& r) const { return { m_x + r.m_x, m_y + r.m_y }; }
        constexpr Coordinate operator-(const Coordinate& r) const { return { m_x - r.m_x, m_y - r.m_y }; }
        constexpr Coordinate operator-() const { return { -m_x, -m_y }; }

        constexpr Coordinate operator+(const T& r) const { return { m_x + r, m_y + r }; }

        // special case for unsigned integral addition with diff of its signed counterpart
        template <typename U>
            requires std::same_as<std::make_signed_t<std::decay_t<T>>, U>
        constexpr Coordinate operator+(const Coordinate<U>& r) const
        {
            return { m_x + static_cast<T>(r.m_x), m_y + static_cast<T>(r.m_y) };
        }

        template <typename U>
            requires std::same_as<std::make_signed_t<std::decay_t<T>>, U>
        constexpr Coordinate operator+(U& r) const
        {
            return { m_x + static_cast<T>(r), m_y + static_cast<T>(r) };
        }

        // check if `this` is within range [min, max)
        constexpr bool within(const Coordinate& min, const Coordinate& max) const
        {
            return m_x >= min.m_x and m_x < max.m_x and m_y >= min.m_y and m_y < max.m_y;
        }

        T m_x;
        T m_y;
    };

    template <typename T>
    Coordinate<std::make_signed_t<T>> distance(const Coordinate<T>& lhs, const Coordinate<T>& rhs)
    {
        using Signed = std::make_signed_t<T>;

        auto&& [lx, ly] = lhs;
        auto&& [rx, ry] = rhs;

        auto x_diff = static_cast<Signed>(rx) - static_cast<Signed>(lx);
        auto y_diff = static_cast<Signed>(ry) - static_cast<Signed>(ly);

        return { x_diff, y_diff };
    }

    /* Von Neumann neighborhood, clockwise (on left-handed system).
     *............
     *.....>......
     *....^cv.....
     *.....<......
     *............
     * start from `>`.
     */
    template <typename T>
    std::array<Coordinate<T>, 4> neumann_neighbors(const Coordinate<T>& coord)
    {
        auto&& [x, y] = coord;
        return {
            // clang-format off
            Coordinate{ x    , y - 1 },
            Coordinate{ x + 1, y     },
            Coordinate{ x    , y + 1 },
            Coordinate{ x - 1, y     },
            // clang-format on
        };
    }

    /* Moore neighborhood, clockwise (on left-handed system).
     *............
     *....>>v.....
     *....^cv.....
     *....^<<.....
     *............
     * start from `>`.
     */
    template <typename T>
    std::array<Coordinate<T>, 8> moore_neighbors(const Coordinate<T>& coord)
    {
        auto&& [x, y] = coord;
        return {
            // clang-format off
            Coordinate{ x - 1, y - 1 },
            Coordinate{ x    , y - 1 },
            Coordinate{ x + 1, y - 1 },
            Coordinate{ x + 1, y     },
            Coordinate{ x + 1, y + 1 },
            Coordinate{ x    , y + 1 },
            Coordinate{ x - 1, y + 1 },
            Coordinate{ x - 1, y     },
            // clang-format on
        };
    }

    enum class NeighborDir : std::uint8_t
    {
        N  = 0b0001,
        E  = 0b0010,
        S  = 0b0100,
        W  = 0b1000,
        NE = N | E,
        SE = S | E,
        SW = S | W,
        NW = N | W,
    };

    // get the neighbor coordinate by direction (left-handed system)
    template <typename T>
    util::Coordinate<T> neighbor_by_dir(const util::Coordinate<T>& coord, NeighborDir dir)
    {
        auto&& [x, y] = coord;

        switch (dir) {
            // clang-format off
        case NeighborDir::N:  return { x    , y - 1 };
        case NeighborDir::E:  return { x + 1, y     };
        case NeighborDir::S:  return { x    , y + 1 };
        case NeighborDir::W:  return { x - 1, y     };
        case NeighborDir::NE: return { x + 1, y - 1 };
        case NeighborDir::SE: return { x + 1, y + 1 };
        case NeighborDir::SW: return { x - 1, y + 1 };
        case NeighborDir::NW: return { x - 1, y - 1 };
            // clang-format on
        }

        std::unreachable();
    }

    template <typename T, typename Ctx>
    struct FormatterHelper
    {
        using Fmt = fmt::formatter<T, char>;

        [[gnu::always_inline]]
        const FormatterHelper& operator<<(const T& t) const
        {
            m_fmt.format(t, m_ctx);
            return *this;
        }

        [[gnu::always_inline]]
        const FormatterHelper& operator<<(const char* str) const
        {
            fmt::format_to(m_ctx.out(), "{}", str);
            return *this;
        }

        const Fmt& m_fmt;
        Ctx&       m_ctx;
    };
}

template <fmt::formattable<char> T>
struct fmt::formatter<aoc::util::Coordinate<T>, char> : fmt::formatter<T>
{
    template <typename FormatContext>
    auto format(const aoc::util::Coordinate<T>& coord, FormatContext& ctx) const
    {
        auto helper   = aoc::util::FormatterHelper<T, FormatContext>{ *this, ctx };
        auto&& [x, y] = coord;
        helper << "(" << x << "," << y << ")";
        return ctx.out();
    }
};
