#pragma once

#include <array>
#include <charconv>
#include <ranges>
#include <string_view>
#include <optional>
#include <system_error>
#include <variant>

#include <fmt/core.h>
#include <rapidhash.h>

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
        return range | std::views::drop(start) | std::views::take(end - start);
    }

    template <std::ranges::range R>
    auto subrange_rev(R&& range, std::size_t start, std::size_t end)
    {
        return subrange(range, start, end) | std::views::reverse;
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
        template <typename U>
            requires std::same_as<std::make_signed_t<std::decay_t<T>>, U>
        Coordinate operator+(const Coordinate<U>& rhs) const
        {
            return { m_x + static_cast<T>(rhs.m_x), m_y + static_cast<T>(rhs.m_y) };
        }

        // check if `this` is within range [min, max)
        bool within(const Coordinate& min, const Coordinate& max) const
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
    }

    struct Iter2D
    {
        struct Iterator
        {
            Iterator& operator++()
            {
                if (++m_x == m_width) {
                    m_x = 0;
                    ++m_y;
                }
                return *this;
            }

            Coordinate<std::size_t> operator*() const { return { m_x, m_y }; }
            auto                    operator<=>(const Iterator&) const = default;

            std::size_t m_x;
            std::size_t m_y;
            std::size_t m_width;
        };

        Iterator begin() const { return { 0, 0, m_width }; }
        Iterator end() const { return { 0, m_height, m_width }; }

        std::size_t m_width;
        std::size_t m_height;
    };

    inline Iter2D iter_2d(std::size_t width, std::size_t height)
    {
        return { width, height };
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

// general hash for any type that has unique object representations
template <typename T>
    requires std::has_unique_object_representations_v<T>
struct std::hash<T>
{
    std::size_t operator()(const T& obj) const noexcept { return rapidhash(&obj, sizeof(obj)); }
};
