#pragma once

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
}
