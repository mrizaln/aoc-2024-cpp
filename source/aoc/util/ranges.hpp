#pragma once

#include <ranges>

namespace aoc::util
{
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
}
