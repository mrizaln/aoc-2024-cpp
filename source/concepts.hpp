#pragma once

#include "aliases.hpp"

#include <fmt/base.h>

#include <concepts>
#include <ranges>

namespace aoc::concepts
{
    template <typename F, typename Ret, typename... Args>
    concept Fn = requires {
        requires std::invocable<F, Args...>;
        requires std::same_as<std::invoke_result_t<F, Args...>, Ret>;
    };

    template <typename T>
    concept Range = std::ranges::range<T>;

    template <typename T>
    concept Streamable = requires (std::ostream& os, const T& t) {
        { os << t } -> std::same_as<std::ostream&>;
    };

    template <typename T>
    concept Displayable = fmt::formattable<T> or Streamable<T>;

    template <typename T>
    concept Day = requires {
        requires std::semiregular<T>;

        requires std::is_trivially_move_constructible_v<T>;
        requires std::is_trivially_move_assignable_v<T>;
        requires std::is_trivially_copyable_v<T>;

        typename T::Input;
        typename T::Output;

        requires Displayable<typename T::Output>;

        { T::id } -> std::convertible_to<std::string_view>;
        { T::name } -> std::convertible_to<std::string_view>;

        requires requires (const T ct, T::Input input, aliases::Lines lines, aliases::Context ctx) {
            { ct.parse(lines, ctx) } -> std::same_as<typename T::Input>;
            { ct.solve_part_one(input, ctx) } -> std::same_as<typename T::Output>;
            { ct.solve_part_two(input, ctx) } -> std::same_as<typename T::Output>;
        };
    };

    namespace detail
    {
        template <typename>
        struct DayTraits : std::false_type
        {
        };

        template <Day... T>
        struct DayTraits<std::tuple<T...>> : std::true_type
        {
        };

        template <Day... T>
        struct DayTraits<std::variant<T...>> : std::true_type
        {
        };
    }

    template <typename T>
    concept AreDays = detail::DayTraits<T>::value;
}
