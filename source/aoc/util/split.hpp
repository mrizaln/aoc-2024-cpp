#pragma once

#include <algorithm>
#include <charconv>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <system_error>
#include <variant>

namespace aoc::util
{
    struct SplitDelim
    {
        using Variant = std::variant<char, std::span<const char>>;

        constexpr SplitDelim(char ch)
            : m_variant{ ch }
        {
        }

        constexpr SplitDelim(std::span<const char> delims)
            : m_variant{ delims }
        {
        }

        constexpr bool is_delim(char ch) const
        {
            auto visitor = [&](auto&& value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::same_as<T, char>) {
                    return ch == value;
                } else {
                    return std::ranges::find(value, ch) != value.end();
                }
            };
            return std::visit(visitor, m_variant);
        }

        Variant m_variant;
    };

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

        StringSplitter(std::string_view str, SplitDelim delim) noexcept
            : m_str{ str }
            , m_delim{ delim }
        {
        }

        std::optional<std::string_view> next() noexcept
        {
            if (m_idx >= m_str.size()) {
                return std::nullopt;
            }

            while (m_idx <= m_str.size() and m_delim.is_delim(m_str[m_idx])) {
                ++m_idx;
            }

            auto it = std::ranges::find_if(m_str | std::views::drop(m_idx), [this](char ch) {
                return m_delim.is_delim(ch);
            });

            if (it == m_str.end()) {
                auto res = m_str.substr(m_idx);
                m_idx    = m_str.size();    // mark end of line
                return res;
            }

            auto pos = static_cast<std::size_t>(it - m_str.begin());
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
        SplitDelim       m_delim = SplitDelim{ ' ' };
    };

    template <std::size_t N>
    constexpr SplitPartResult<N> split_part_n(std::string_view str, SplitDelim delim) noexcept
    {
        auto res = SplitPartResult<N>{};

        auto i = 0uz;
        auto j = 0uz;

        while (i < N and j < str.size()) {
            while (j != str.size() and delim.is_delim(str[j])) {
                ++j;
            }

            auto it = std::ranges::find_if(str | std::views::drop(j), [&delim](char ch) {
                return delim.is_delim(ch);
            });

            if (it == str.end()) {
                res.m_split[i++] = str.substr(j);
                break;
            }

            auto pos         = static_cast<std::size_t>(it - str.begin());
            res.m_split[i++] = str.substr(j, pos - j);
            j                = pos + 1;
        }

        res.m_count = i;
        return res;
    }

    template <std::size_t N>
    constexpr SplitResult<N> split_n(std::string_view str, SplitDelim delim) noexcept
    {
        auto res = split_part_n<N>(str, delim);
        if (res.m_count != N) {
            return std::nullopt;
        }
        return SplitResult<N>{ res.m_split };
    }

    template <typename T, std::size_t N>
        requires std::is_fundamental_v<T>
    SplitParseResult<Parsed<T, N>> split_parse_n(std::string_view str, SplitDelim delim) noexcept
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
        SplitDelim       delim,
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
}
