#pragma once

#include "aliases.hpp"
#include "common.hpp"

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    namespace day4
    {
        inline al::usize xmas(common::Lines in, al::usize y, al::usize x)
        {
            // search xmas in 8 cardinal direction: N, E, S, W, NE, SE, SW, NW
            auto count = 0uz;

            // W
            if (x >= 3) {
                count += in[y][x - 1] == 'M'    //
                     and in[y][x - 2] == 'A'    //
                     and in[y][x - 3] == 'S';
            }
            // E
            if (x + 3 < in[y].size()) {
                count += in[y][x + 1] == 'M'    //
                     and in[y][x + 2] == 'A'    //
                     and in[y][x + 3] == 'S';
            }
            // N
            if (y >= 3) {
                count += in[y - 1][x] == 'M'    //
                     and in[y - 2][x] == 'A'    //
                     and in[y - 3][x] == 'S';
            }
            // S
            if (y + 3 < in.size()) {
                count += in[y + 1][x] == 'M'    //
                     and in[y + 2][x] == 'A'    //
                     and in[y + 3][x] == 'S';
            }

            // NE
            if (y >= 3 and x + 3 < in[y].size()) {
                count += in[y - 1][x + 1] == 'M'    //
                     and in[y - 2][x + 2] == 'A'    //
                     and in[y - 3][x + 3] == 'S';
            }
            // SE
            if (y + 3 < in.size() and x + 3 < in[y].size()) {
                count += in[y + 1][x + 1] == 'M'    //
                     and in[y + 2][x + 2] == 'A'    //
                     and in[y + 3][x + 3] == 'S';
            }
            // NW
            if (y >= 3 and x >= 3) {
                count += in[y - 1][x - 1] == 'M'    //
                     and in[y - 2][x - 2] == 'A'    //
                     and in[y - 3][x - 3] == 'S';
            }
            // SW
            if (y + 3 < in.size() and x >= 3) {
                count += in[y + 1][x - 1] == 'M'    //
                     and in[y + 2][x - 2] == 'A'    //
                     and in[y + 3][x - 3] == 'S';
            }

            return count;
        }

        inline bool x_mas(common::Lines in, al::usize y, al::usize x)
        {
            assert(x >= 1 and y >= 1 and x < in[y].size() - 1 and y < in.size() - 1);

            // x-mas down
            auto pattern_1 = [&] {
                return in[y - 1][x - 1] == 'M' and in[y - 1][x + 1] == 'M'    //
                   and in[y + 1][x - 1] == 'S' and in[y + 1][x + 1] == 'S';
            };
            // x-mas up
            auto pattern_2 = [&] {
                return in[y - 1][x - 1] == 'S' and in[y - 1][x + 1] == 'S'    //
                   and in[y + 1][x - 1] == 'M' and in[y + 1][x + 1] == 'M';
            };
            // x-mas right
            auto pattern_3 = [&] {
                return in[y - 1][x - 1] == 'M' and in[y - 1][x + 1] == 'S'    //
                   and in[y + 1][x - 1] == 'M' and in[y + 1][x + 1] == 'S';
            };
            // x-mas left
            auto pattern_4 = [&] {
                return in[y - 1][x - 1] == 'S' and in[y - 1][x + 1] == 'M'    //
                   and in[y + 1][x - 1] == 'S' and in[y + 1][x + 1] == 'M';
            };

            return pattern_1() or pattern_2() or pattern_3() or pattern_4();
        }
    }

    struct Day04
    {
        static constexpr auto id   = "04";
        static constexpr auto name = "ceres-search";

        using Input  = common::Lines;
        using Output = al::usize;

        Input parse(common::Lines lines) const { return lines; }

        Output solve_part_one(Input input) const
        {
            auto count = 0uz;
            for (auto&& [y, line] : input | sv::enumerate) {
                for (auto&& [x, chr] : line | sv::enumerate) {
                    if (chr == 'X') {
                        count += day4::xmas(input, static_cast<al::usize>(y), static_cast<al::usize>(x));
                    }
                }
            }
            return count;
        }

        Output solve_part_two(Input input) const
        {
            auto count = 0uz;
            for (auto&& [y, line] : input | sv::enumerate | sv::take(input.size() - 1) | sv::drop(1)) {
                for (auto&& [x, chr] : line | sv::enumerate | sv::take(line.size() - 1) | sv::drop(1)) {
                    if (chr == 'A') {
                        count += day4::x_mas(input, static_cast<al::usize>(y), static_cast<al::usize>(x));
                    }
                }
            }
            return count;
        }
    };

    static_assert(common::Day<Day04>);
}
