#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    struct Day02
    {
        static constexpr auto id       = "02";
        static constexpr auto name     = "red-nosed-reports";
        static constexpr auto max_size = 8uz;
        static constexpr auto invalid  = std::numeric_limits<al::i32>::max();

        using Arr = std::array<al::i32, max_size>;

        using Input  = std::vector<Arr>;
        using Output = al::usize;

        enum class Level
        {
            Start,
            Increasing,
            Decreasing,
        };

        Input parse(common::Lines lines, common::Context /* ctx */) const
        {
            auto to_arr = [](std::string_view line) -> Arr {
                auto res = util::split_part_parse_n<al::i32, max_size>(line, ' ', invalid).as_success();
                return std::move(res).m_parsed;
            };

            return lines | sv::transform(to_arr) | sr::to<std::vector>();
        }

        Output solve_part_one(Input input, common::Context /* ctx */) const
        {
            auto is_safe = [&](const Arr& arr) {
                auto diff = std::array<al::i32, max_size - 1>{};

                auto invalid_idx = max_size - 1;
                for (auto i : sv::iota(0uz, max_size - 1)) {
                    if (arr[i + 1] == invalid) {
                        invalid_idx = i;
                        break;
                    }
                    diff[i] = arr[i + 1] - arr[i];
                }

                auto level = Level::Start;
                for (auto i : sv::iota(0uz, invalid_idx)) {
                    auto d = diff[i];

                    if (d >= 1 and d <= 3) {
                        if (level == Level::Decreasing) {
                            return false;
                        }
                        level = Level::Increasing;
                    } else if (d >= -3 and d <= -1) {
                        if (level == Level::Increasing) {
                            return false;
                        }
                        level = Level::Decreasing;
                    } else {
                        return false;
                    }
                }

                return true;
            };

            auto count = sr::count_if(input, is_safe);
            return static_cast<Output>(count);
        }

        Output solve_part_two(Input input, common::Context /* ctx */) const
        {
            auto is_safe = [&](const Arr& arr) {
                auto diff = std::array<al::i32, max_size - 1>{};

                auto invalid_idx = max_size - 1;
                for (auto i : sv::iota(0uz, max_size - 1)) {
                    if (arr[i + 1] == invalid) {
                        invalid_idx = i;
                        break;
                    }
                    diff[i] = arr[i + 1] - arr[i];
                }

                auto level     = Level::Start;
                auto tolerance = 1_i32;

                for (auto i : sv::iota(0uz, invalid_idx)) {
                    auto d = diff[i];

                    if (d >= 1 and d <= 3) {
                        if (level == Level::Decreasing and tolerance-- == 0) {
                            return false;
                        }
                        level = Level::Increasing;
                    } else if (d >= -3 and d <= -1) {
                        if (level == Level::Increasing and tolerance-- == 0) {
                            return false;
                        }
                        level = Level::Decreasing;
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
