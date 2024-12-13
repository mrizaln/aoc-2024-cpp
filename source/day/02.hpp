#pragma once

#include "aliases.hpp"
#include "common.hpp"

#include <scn/scan.h>

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    struct Day02
    {
        static constexpr auto name     = "02-red-nosed-reports";
        static constexpr auto max_size = 8uz;
        static constexpr auto invalid  = std::numeric_limits<al::i32>::max();

        using Arr = std::array<al::i32, max_size>;

        using Input  = std::vector<Arr>;
        using Output = al::usize;

        enum class Level1
        {
            Start,
            Increasing,
            Decreasing,
        };

        enum class Level2
        {
            DontCare,      // d == invalid
            Still,         // d == 0
            Increasing,    // 1 <= d <= 3
            Decreasing,    // -3 <= d <= -1
            Danger,        // anything else
        };

        Input parse(common::Lines lines) const
        {
            auto to_arr = [](std::string_view line) -> Arr {
                auto arr = Arr{};
                arr.fill(invalid);

                auto i = 0uz;

                auto input = scn::ranges::subrange{ line };
                while (auto res = scn::scan<al::i32>(input, "{}")) {
                    arr[i++] = res->value();
                    input    = res->range();
                }

                return arr;
            };

            return lines | sv::transform(to_arr) | sr::to<std::vector>();
        }

        Output solve_part_one(Input input) const
        {
            auto is_safe = [&](const Arr& arr) {
                auto diff = std::array<al::i32, max_size - 1>{};
                diff.fill(invalid);

                auto invalid_idx = max_size - 1;
                for (auto i : sv::iota(0uz, max_size - 1)) {
                    if (arr[i + 1] == invalid) {
                        invalid_idx = i;
                        break;
                    }
                    diff[i] = arr[i + 1] - arr[i];
                }

                auto level = Level1::Start;
                for (auto i : sv::iota(0uz, invalid_idx)) {
                    auto d = diff[i];

                    if (d >= 1 and d <= 3) {
                        if (level == Level1::Decreasing) {
                            return false;
                        }
                        level = Level1::Increasing;
                    } else if (d >= -3 and d <= -1) {
                        if (level == Level1::Increasing) {
                            return false;
                        }
                        level = Level1::Decreasing;
                    } else {
                        return false;
                    }
                }

                return true;
            };

            auto count = sr::count_if(input, is_safe);
            return static_cast<Output>(count);
        }

        Output solve_part_two(Input input) const
        {
            auto is_safe = [&](const Arr& arr) {
                auto diff = std::array<al::i32, max_size - 1>{};
                diff.fill(invalid);

                auto invalid_idx = max_size - 1;
                for (auto i : sv::iota(0uz, max_size - 1)) {
                    if (arr[i + 1] == invalid) {
                        invalid_idx = i;
                        break;
                    }
                    diff[i] = arr[i + 1] - arr[i];
                }

                auto level     = Level1::Start;
                auto tolerance = 1;

                for (auto i : sv::iota(0uz, invalid_idx)) {
                    auto d = diff[i];

                    if (d >= 1 and d <= 3) {
                        if (level == Level1::Decreasing and tolerance-- == 0) {
                            return false;
                        }
                        level = Level1::Increasing;
                    } else if (d >= -3 and d <= -1) {
                        if (level == Level1::Increasing and tolerance-- == 0) {
                            return false;
                        }
                        level = Level1::Decreasing;
                    } else {
                        if (tolerance-- == 0) {
                            return false;
                        }
                    }
                }

                return true;
            };

            auto count = sr::count_if(input, is_safe);
            return static_cast<Output>(count);
        }
    };

    static_assert(common::Day<Day02>);
}
