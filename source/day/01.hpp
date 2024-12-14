#pragma once

#include "aliases.hpp"
#include "common.hpp"

#include <scn/scan.h>

#include <unordered_map>

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    struct Day01
    {
        static constexpr auto id   = "01";
        static constexpr auto name = "historian-hysteria";

        using Pair = std::pair<al::i32, al::i32>;

        using Input  = std::vector<Pair>;
        using Output = al::i32;

        Input parse(common::Lines lines) const
        {
            auto to_pair = [](std::string_view line) -> Pair {
                auto res = scn::scan<al::i32, al::i32>(line, "{} {}");
                if (not res) {
                    throw std::runtime_error("Failed to parse input");
                }
                return res->values();
            };

            return lines | sv::transform(to_pair) | sr::to<std::vector>();
        }

        // TODO: try using binary search tree
        Output solve_part_one(Input input) const
        {
            auto left  = std::vector<al::i32>{};
            auto right = std::vector<al::i32>{};

            for (auto [l, r] : input) {
                left.push_back(l);
                right.push_back(r);
            }

            sr::sort(left);
            sr::sort(right);

            auto diff = [](auto&& pair) { return std::abs(std::get<0>(pair) - std::get<1>(pair)); };
            return sr::fold_left(sv::zip(left, right) | sv::transform(diff), 0, std::plus{});
        }

        Output solve_part_two(Input input) const
        {
            auto left      = std::vector<al::i32>{};
            auto right     = std::vector<al::i32>{};
            auto right_map = std::unordered_map<al::i32, al::i32>();

            for (auto [l, r] : input) {
                left.push_back(l);
                right.push_back(r);

                right_map.emplace(l, 0);
            }

            for (auto v : right) {
                ++right_map[v];
            }

            auto calc_similarity = [&](al::i32 v) { return right_map.at(v) * v; };
            return sr::fold_left(left | sv::transform(calc_similarity), 0, std::plus{});
        }
    };

    static_assert(common::Day<Day01>);
}
