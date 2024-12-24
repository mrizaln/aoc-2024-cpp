#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    namespace day13
    {
        using Coord = util::Coordinate<al::i64>;

        struct Machine
        {
            Coord m_button_a;
            Coord m_button_b;
            Coord m_prize;
        };
    };

    struct Day13
    {
        static constexpr auto id   = "13";
        static constexpr auto name = "claw-contraption";

        using Coord   = day13::Coord;
        using Machine = day13::Machine;

        using Input  = std::vector<Machine>;
        using Output = al::i64;

        Input parse(common::Lines lines) const
        {
            auto parse_btn = [](std::string_view line) -> Coord {
                auto delims                   = util::SplitDelim{ " :,+" };
                auto [btn, n, x, x_v, y, y_v] = util::split_n<6>(line, delims).value();

                auto x_val = util::from_chars<al::i64>(x_v).first;
                auto y_val = util::from_chars<al::i64>(y_v).first;

                return { x_val, y_val };
            };

            auto parse_prize = [](std::string_view line) -> Coord {
                auto delims                  = util::SplitDelim{ " :,=" };
                auto [prize, x, x_v, y, y_v] = util::split_n<5>(line, delims).value();

                auto x_val = util::from_chars<al::i64>(x_v).first;
                auto y_val = util::from_chars<al::i64>(y_v).first;

                return { x_val, y_val };
            };

            return lines           //
                 | sv::chunk(4)    //
                 | sv::transform([&](auto group) -> Machine {
                       ASSERT(group.size() == 4 or group.size() == 3);
                       return { parse_btn(group[0]), parse_btn(group[1]), parse_prize(group[2]) };
                   })
                 | sr::to<std::vector>();
        }

        Output solve_impl(Input input, al::i64 prize_offset) const
        {
            auto solve = [&](const Machine& machine) -> Coord {
                auto [btn_a, btn_b, prize] = machine;

                auto [ax, ay] = btn_a;
                auto [bx, by] = btn_b;
                auto [px, py] = prize + prize_offset;

                auto det   = (ax * by) - (ay * bx);
                auto det_a = (px * by) - (py * bx);
                auto det_b = (ax * py) - (ay * px);

                if (det_a % det == 0 and det_b % det == 0) {
                    return { det_a / det, det_b / det };
                }

                return { 0, 0 };
            };

            constexpr auto cost = Coord{ 3, 1 };

            return sr::fold_left(input, 0_i64, [&](auto&& sum, auto&& machine) {
                auto [na, nb] = solve(machine);
                return sum + cost.m_x * na + cost.m_y * nb;
            });
        }

        Output solve_part_one(Input input) const { return solve_impl(input, 0); }
        Output solve_part_two(Input input) const { return solve_impl(input, 10'000'000'000'000); }
    };

    static_assert(common::Day<Day13>);
}
