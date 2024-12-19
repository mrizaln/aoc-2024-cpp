#pragma once

#include <array>
#include <charconv>
#include <string_view>
#include <optional>
#include <system_error>
#include <variant>

#include <fmt/core.h>

#include "common.hpp"

namespace aoc::util
{
    template <typename T>
    struct SplitParseResult
    {
        // clang-format off
        struct SplitError {                  };
        struct ParseError { std::errc m_err; };

        using Success = T;
        using Variant = std::variant<Success, SplitError, ParseError>;
        // clang-format on

        bool      is_success() const noexcept { return std::holds_alternative<Success>(m_value); }
        Success&& unwrap_success() && { return std::move(*std::get_if<Success>(&m_value)); }

        Success&& as_success() &&
        {
            if (not is_success()) {
                throw as_error();
            }
            return std::move(*this).unwrap_success();
        }

        std::runtime_error as_error() const noexcept
        {
            if (auto* err = std::get_if<SplitError>(&m_value)) {
                return std::runtime_error{ "Failed to split" };
            } else if (auto* err = std::get_if<ParseError>(&m_value)) {
                auto err_code = std::make_error_code(err->m_err);
                return std::runtime_error{ "Failed to parse: " + err_code.message() };
            }

            // should never reach here, if it does, it's programmer's fault
            std::abort();
        }

        Variant m_value;
    };

    template <std::size_t N>
    using SplitResult = std::optional<std::array<std::string_view, N>>;

    template <std::size_t N>
    struct SplitPartResult
    {
        std::array<std::string_view, N> m_split = {};
        std::size_t                     m_count = 0;
    };

    template <std::default_initializable T, std::size_t N>
    using Parsed = std::array<T, N>;

    template <std::default_initializable T, std::size_t N>
    struct PartParsed
    {
        std::array<T, N> m_parsed = {};
        std::size_t      m_count  = 0;
    };

    template <typename T>
        requires std::is_fundamental_v<T>
    std::pair<T, std::errc> from_chars(std::string_view str) noexcept
    {
        auto res       = T{};
        auto [ptr, ec] = std::from_chars(str.begin(), str.end(), res);
        return { res, ec };
    };

    class StringSplitter
    {
    public:
        template <typename T>
        struct NextParseResult
        {
            // clang-format off
            struct Error { std::errc m_err; };

            using Success = T;
            using Variant = std::variant<Success, Error>;
            // clang-format on

            bool      is_success() const noexcept { return std::holds_alternative<Success>(m_value); }
            Success&& unwrap_success() && { return std::move(*std::get_if<Success>(&m_value)); }

            Success&& as_success() &&
            {
                if (not is_success()) {
                    throw as_error();
                }
                return std::move(*this).unwrap_success();
            }

            std::runtime_error as_error() const noexcept
            {
                if (auto* err = std::get_if<Error>(&m_value)) {
                    auto err_code = std::make_error_code(err->m_err);
                    return std::runtime_error{ "Failed to parse: " + err_code.message() };
                }

                // should never reach here, if it does, it's programmer's fault
                std::abort();
            }

            Variant m_value;
        };

        StringSplitter(std::string_view str, char delim) noexcept
            : m_str{ str }
            , m_delim{ delim }
        {
        }

        std::optional<std::string_view> next() noexcept
        {
            if (m_idx >= m_str.size()) {
                return std::nullopt;
            }

            while (m_idx <= m_str.size() and m_str[m_idx] == m_delim) {
                ++m_idx;
            }

            auto pos = m_str.find(m_delim, m_idx);
            if (pos == std::string_view::npos) {
                auto res = m_str.substr(m_idx);
                m_idx    = m_str.size();    // mark end of line
                return res;
            }

            auto res = m_str.substr(m_idx, pos - m_idx);
            m_idx    = pos + 1;

            return res;
        }

        template <typename T>
        std::optional<NextParseResult<T>> next_parse() noexcept
        {
            auto res = next();
            if (not res) {
                return std::nullopt;
            }

            auto [value, ec] = from_chars<T>(*res);
            if (ec != std::errc{}) {
                return NextParseResult<T>{ typename NextParseResult<T>::Error{ ec } };
            }

            return NextParseResult<T>{ typename NextParseResult<T>::Success{ std::move(value) } };
        }

    private:
        std::string_view m_str;
        std::size_t      m_idx   = 0;
        char             m_delim = ' ';
    };

    template <std::size_t N>
    constexpr SplitPartResult<N> split_part_n(std::string_view str, char delim) noexcept
    {
        auto res = SplitPartResult<N>{};

        auto i = 0uz;
        auto j = 0uz;

        while (i < N and j < str.size()) {
            while (j != str.size() and str[j] == delim) {
                ++j;
            }

            auto pos = str.find(delim, j);

            if (pos == std::string_view::npos) {
                res.m_split[i++] = str.substr(j);
                break;
            }

            res.m_split[i++] = str.substr(j, pos - j);
            j                = pos + 1;
        }

        res.m_count = i;
        return res;
    }

    template <std::size_t N>
    constexpr SplitResult<N> split_n(std::string_view str, char delim) noexcept
    {
        auto res = split_part_n<N>(str, delim);
        if (res.m_count != N) {
            return std::nullopt;
        }
        return SplitResult<N>{ res.m_split };
    }

    template <typename T, std::size_t N>
        requires std::is_fundamental_v<T>
    SplitParseResult<Parsed<T, N>> split_parse_n(std::string_view str, char delim) noexcept
    {
        using Res = SplitParseResult<Parsed<T, N>>;

        auto values = std::array<T, N>{};
        auto split  = split_n<N>(str, delim);
        if (not split) {
            return Res{ typename Res::SplitError{} };
        }

        for (auto i = 0uz; i < N; ++i) {
            auto [value, ec] = from_chars<T>(split->at(i));
            if (ec != std::errc{}) {
                return Res{ typename Res::ParseError{ ec } };
            }
            values[i] = value;
        }

        return Res{ typename Res::Success{ values } };
    }

    template <typename T, std::size_t N>
        requires std::is_fundamental_v<T>
    SplitParseResult<PartParsed<T, N>> split_part_parse_n(
        std::string_view str,
        char             delim,
        T                default_value
    ) noexcept
    {
        using Res = SplitParseResult<PartParsed<T, N>>;

        auto values = std::array<T, N>{};
        values.fill(default_value);

        auto [split, count] = split_part_n<N>(str, delim);

        for (auto i = 0uz; i < count; ++i) {
            auto [value, ec] = from_chars<T>(split.at(i));
            if (ec != std::errc{}) {
                return Res{ typename Res::ParseError{ ec } };
            }
            values[i] = value;
        }

        return Res{ typename Res::Success{ .m_parsed = std::move(values), .m_count = count } };
    }

    template <std::ranges::range R>
    auto subrange(R&& range, std::size_t start, std::size_t end)
    {
        return range | common::sv::drop(start) | common::sv::take(end - start);
    }

    template <std::ranges::range R>
    auto subrange_rev(R&& range, std::size_t start, std::size_t end)
    {
        return subrange(range, start, end) | common::sv::reverse;
    }

    template <typename T>
    struct Coordinate
    {
        using Type = T;

        auto operator<=>(const Coordinate&) const = default;

        Coordinate operator+(const Coordinate& rhs) const { return { m_x + rhs.m_x, m_y + rhs.m_y }; }
        Coordinate operator-(const Coordinate& rhs) const { return { m_x - rhs.m_x, m_y - rhs.m_y }; }
        Coordinate operator-() const { return { -m_x, -m_y }; }

        // special case for unsigned integral addition with diff of its signed counterpart
        Coordinate operator+(const Coordinate<std::make_signed_t<T>>& rhs) const
            requires std::unsigned_integral<T>
        {
            return { m_x + static_cast<T>(rhs.m_x), m_y + static_cast<T>(rhs.m_y) };
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
}

template <std::formattable<char> T>
struct fmt::formatter<aoc::util::Coordinate<T>, char> : fmt::formatter<T>
{
    template <typename FormatContext>
    auto format(const aoc::util::Coordinate<T>& coord, FormatContext& ctx) const
    {
        fmt::format_to(ctx.out(), "(");
        fmt::formatter<T>::format(coord.m_x, ctx);
        fmt::format_to(ctx.out(), ", ");
        fmt::formatter<T>::format(coord.m_y, ctx);
        fmt::format_to(ctx.out(), ")");
        return ctx.out();
    }
};

template <typename T>
struct std::hash<aoc::util::Coordinate<T>>
{
    std::size_t operator()(const aoc::util::Coordinate<T>& c) const
    {
        using Coord = aoc::util::Coordinate<T>;

        if constexpr (sizeof(Coord) == 2) {
            auto casted = std::bit_cast<std::uint16_t>(c);
            return std::hash<std::uint16_t>{}(casted);
        } else if constexpr (sizeof(Coord) == 4) {
            auto casted = std::bit_cast<std::uint32_t>(c);
            return std::hash<std::uint32_t>{}(casted);
        } else if constexpr (sizeof(Coord) == 8) {
            auto casted = std::bit_cast<std::uint64_t>(c);
            return std::hash<std::uint64_t>{}(casted);
        } else {
            auto&& [x, y] = c;
            auto hash     = std::hash<T>{};
            return hash(x) ^ (hash(y) << 1);
        }
    }
};
