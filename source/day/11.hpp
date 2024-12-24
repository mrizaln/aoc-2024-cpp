#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

#include <unordered_map>

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    namespace day11
    {
        inline al::usize num_digits(al::u64 num)
        {
            auto digits = 0uz;
            while (num > 0) {
                num /= 10;
                ++digits;
            }
            return digits;
        }

        inline std::pair<al::u64, al::u64> split_digits(al::u64 num, al::usize midpoint)
        {
            auto divisor = 1_u64;
            for (auto _ : sv::iota(0uz, midpoint)) {
                divisor *= 10;
            }
            return { num / divisor, num % divisor };
        }
    }

    struct Day11
    {
        static constexpr auto id   = "11";
        static constexpr auto name = "plutonian-pebbles";

        static constexpr auto num_blinks_part_one = 25uz;
        static constexpr auto num_blinks_part_two = 75uz;

        using Pebble = al::u64;

        using Input  = std::vector<Pebble>;
        using Output = al::u64;

        Input parse(common::Lines lines) const
        {
            ASSERT(lines.size() >= 1);

            auto input    = Input{};
            auto splitter = util::StringSplitter{ lines[0], ' ' };

            while (auto res = splitter.next_parse<al::u64>()) {
                input.push_back(std::move(*res).as_success());
            }

            return input;
        }

        Output solve_impl(Input input, al::usize blinks) const
        {
            // x = blink, y = num
            using MemoEntry = util::Coordinate<al::u64>;
            auto memo       = std::unordered_map<MemoEntry, al::usize>{};

            auto blink = [&](this auto&& self, al::usize current_blink, al::u64 num) -> al::usize {
                if (current_blink >= blinks) {
                    return 1;
                }

                if (auto it = memo.find({ current_blink, num }); it != memo.end()) {
                    return it->second;
                }

                auto result = 0uz;
                if (num == 0) {
                    result = self(current_blink + 1, 1);
                } else if (auto digits = day11::num_digits(num); digits % 2 == 0) {
                    auto [left, right] = day11::split_digits(num, digits / 2);
                    result             = self(current_blink + 1, left) + self(current_blink + 1, right);
                } else {
                    result = self(current_blink + 1, num * 2024);
                }

                memo[{ current_blink, num }] = result;
                return result;
            };

            return sr::fold_left(input, 0uz, [&](al::usize a, al::u64 n) { return a + blink(0uz, n); });
        }

        Output solve_part_one(Input input) const { return solve_impl(std::move(input), num_blinks_part_one); }
        Output solve_part_two(Input input) const { return solve_impl(std::move(input), num_blinks_part_two); }
    };

    static_assert(common::Day<Day11>);
}
