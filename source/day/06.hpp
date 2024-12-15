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

    namespace day6
    {
        struct Pair32Hash
        {
            al::usize operator()(const std::pair<al::u32, al::u32>& pair) const
            {
                auto&& [l, r] = pair;
                auto repr     = static_cast<al::usize>(l) << 32uz | static_cast<al::usize>(r);
                return std::hash<al::usize>{}(repr);
            }
        };
    }

    struct Day06
    {
        static constexpr auto id   = "06";
        static constexpr auto name = "guard-gallivant";

        static constexpr auto up           = '^';
        static constexpr auto obstructions = '#';

        using Map = common::Lines;

        using Input  = Map;
        using Output = al::usize;

        enum class Facing
        {
            Up    = 0,
            Right = 1,
            Down  = 2,
            Left  = 3,
        };

        using Position    = std::pair<al::u32, al::u32>;
        using VisitedPath = std::unordered_map<Position, Facing, day6::Pair32Hash>;

        Position find_guard(Input input) const
        {
            for (auto [y, line] : input | sv::enumerate) {
                for (auto [x, c] : line | sv::enumerate) {
                    if (c == up) {
                        return { x, y };
                    }
                }
            }

            // the guard must exist in the input
            return { -1, -1 };
        }

        std::optional<std::pair<Position, Facing>> guard_next_step(
            Map     map,
            al::u32 x,
            al::u32 y,
            Facing  facing
        ) const
        {
            auto in_bound        = [&](auto x, auto y) { return x < map[0].size() and y < map.size(); };
            auto has_obstruction = [&](auto x, auto y) { return map[y][x] == obstructions; };

            auto new_x = x;
            auto new_y = y;

            switch (facing) {
            case Facing::Up: {
                if (not in_bound(new_x, --new_y)) {
                    return std::nullopt;
                }
            } break;
            case Facing::Right: {
                if (not in_bound(++new_x, new_y)) {
                    return std::nullopt;
                }
            } break;
            case Facing::Down: {
                if (not in_bound(new_x, ++new_y)) {
                    return std::nullopt;
                }
            } break;
            case Facing::Left: {
                if (not in_bound(--new_x, new_y)) {
                    return std::nullopt;
                }
            } break;
            }

            if (not has_obstruction(new_x, new_y)) {
                return std::pair{ Position{ new_x, new_y }, facing };
            }

            auto underlying = std::to_underlying(facing);
            auto size       = std::to_underlying(Facing::Left) + 1;
            auto new_face   = static_cast<Facing>((underlying + 1) % size);

            return std::pair{ Position{ x, y }, new_face };
        }

        Input parse(common::Lines lines) const { return lines; }

        Output solve_part_one(Input input) const
        {
            auto visited_path = VisitedPath{};
            visited_path.reserve(input.size() * input[0].size());

            auto facing = Facing::Up;
            auto pos    = find_guard(input);
            auto [x, y] = pos;

            visited_path[pos] = facing;

            while (y < input.size() and x < input[0].size()) {
                auto next = guard_next_step(input, x, y, facing);
                if (not next) {
                    break;
                }

                auto [new_pos, new_facing] = *next;
                if (new_facing != facing) {
                    facing = new_facing;
                    continue;
                }

                visited_path[new_pos] = facing;

                auto [new_x, new_y] = new_pos;
                x                   = new_x;
                y                   = new_y;
            }

            return visited_path.size();
        }

        Output solve_part_two(Input /* input */) const
        {
            return {};    // TODO: implement
        }
    };

    static_assert(common::Day<Day06>);
}
