#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

#include <unordered_map>
#include <unordered_set>

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    namespace day10
    {
        using Coord = util::Coordinate<al::u32>;

        struct TopographicMap
        {
            decltype(auto) operator[](this auto&& self, const Coord& coord)
            {
                auto&& [x, y] = coord;
                return std::forward<decltype(self)>(self).m_map[y][x];
            }

            auto surrounding_four(const Coord& coord)
            {
                using Diff    = util::Coordinate<std::make_signed_t<Coord::Type>>;
                using DiffArr = std::array<Diff, 4>;

                auto bound = [&](const Coord& c) { return c.m_x < m_map[0].size() && c.m_y < m_map.size(); };
                auto add   = [=](const Diff& d) { return coord + d; };

                static constexpr auto diffs = DiffArr{ { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } } };
                return diffs | sv::transform(add) | sv::filter(bound);
            }

            common::Lines m_map;
        };
    };

    struct Day10
    {
        static constexpr auto id        = "10";
        static constexpr auto name      = "hoof-it";
        static constexpr auto trailhead = '0';
        static constexpr auto peak      = '9';

        using Coord          = day10::Coord;
        using TopographicMap = day10::TopographicMap;
        using TrailHeads     = std::vector<Coord>;

        using Input  = std::pair<TopographicMap, TrailHeads>;
        using Output = al::usize;

        Input parse(common::Lines lines, common::Context /* ctx */) const
        {
            auto trail_heads = TrailHeads{};

            for (auto [y, line] : lines | sv::enumerate) {
                for (auto [x, c] : line | sv::enumerate) {
                    if (c == trailhead) {
                        trail_heads.emplace_back(x, y);
                    }
                }
            }
            return { TopographicMap{ lines }, trail_heads };
        }

        Output solve_part_one(Input input, common::Context /* ctx */) const
        {
            auto&& [map, heads] = input;

            auto find_peak = [&](this auto&& self, Coord start, auto&& fn_when_peak) -> void {
                if (map[start] == peak) {
                    fn_when_peak(start);
                }
                for (const Coord& next : map.surrounding_four(start)) {
                    if (map[next] - map[start] == 1) {
                        self(next, fn_when_peak);
                    }
                }
            };

            auto unique_peaks = [&](Coord start) {
                auto peaks = std::unordered_set<Coord>{};
                find_peak(start, [&](Coord peak) { peaks.insert(peak); });
                return peaks.size();
            };

            auto acc = [&](auto&& acc, auto&& head) { return acc + unique_peaks(head); };
            return sr::fold_left(heads, 0uz, acc);
        }

        Output solve_part_two(Input input, common::Context /* ctx */) const
        {
            auto&& [map, heads] = input;

            auto find_peak = [&](this auto&& self, Coord start, auto&& fn_when_peak) -> void {
                if (map[start] == peak) {
                    fn_when_peak(start);
                }
                for (const Coord& next : map.surrounding_four(start)) {
                    if (map[next] - map[start] == 1) {
                        self(next, fn_when_peak);
                    }
                }
            };

            auto distinct_trails = [&](Coord start) {
                auto peaks = std::unordered_map<Coord, al::usize>{};
                find_peak(start, [&](Coord peak) { ++peaks[peak]; });
                return sr::fold_left(peaks | sv::values, 0uz, std::plus<>{});
            };

            auto acc = [&](auto&& acc, auto&& head) { return acc + distinct_trails(head); };
            return sr::fold_left(heads, 0uz, acc);
        }
    };

    static_assert(common::Day<Day10>);
}
