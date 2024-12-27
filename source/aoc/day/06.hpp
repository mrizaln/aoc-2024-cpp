#pragma once

#include "aliases.hpp"
#include "common.hpp"

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    namespace day6
    {
        enum class Facing : al::u8
        {
            Up    = 0,
            Right = 1,
            Down  = 2,
            Left  = 3,

            Invalid = 0xff,    // used to fill empty facing direction when initializing a scratch map
        };

        struct Position
        {
            al::u32 m_x = 0;
            al::u32 m_y = 0;

            auto operator<=>(const Position&) const = default;
        };

        struct ScratchMap
        {
            ScratchMap(al::usize width, al::usize height, Facing facing)
                : m_width{ width }
                , m_height{ height }
                , m_facing(width * height, facing)
            {
            }

            void fill(Facing facing) { sr::fill(m_facing, facing); }

            auto&& operator[](this auto&& self, al::usize x, al::usize y)
            {
                return std::forward<decltype(self)>(self).m_facing[y * self.m_width + x];
            }

            auto&& operator[](this auto&& self, const Position& pos)
            {
                return std::forward<decltype(self)>(self)[pos.m_x, pos.m_y];
            }

            void print(Position mark)
            {
                auto facing_char = [](auto facing) {
                    switch (facing) {
                    case Facing::Up: return '^';
                    case Facing::Right: return '>';
                    case Facing::Down: return 'v';
                    case Facing::Left: return '<';
                    default: return '.';
                    }
                };

                for (auto y : sv::iota(0uz, m_height)) {
                    for (auto x : sv::iota(0uz, m_width)) {
                        auto facing = m_facing[y * m_width + x];
                        if (Position{ static_cast<al::u32>(x), static_cast<al::u32>(y) } == mark) {
                            if (facing == Facing::Invalid) {
                                fmt::print("▒");
                            } else {
                                fmt::print("█");
                            }
                        } else {
                            fmt::print("{}", facing_char(facing));
                        }
                    }
                    fmt::print("\n");
                }
                fmt::print("\n");
            }

            al::usize           m_width  = 0;
            al::usize           m_height = 0;
            std::vector<Facing> m_facing;
        };
    }

    struct Day06
    {
        static constexpr auto id   = "06";
        static constexpr auto name = "guard-gallivant";

        static constexpr auto up          = '^';
        static constexpr auto obstruction = '#';

        using Map = common::Lines;

        using Input  = Map;
        using Output = al::usize;

        using Facing     = day6::Facing;
        using ScratchMap = day6::ScratchMap;
        using Position   = day6::Position;

        Position find_guard(Input input) const
        {
            for (auto [y, line] : input | sv::enumerate) {
                for (auto [x, c] : line | sv::enumerate) {
                    if (c == up) {
                        return { static_cast<al::u32>(x), static_cast<al::u32>(y) };
                    }
                }
            }

            ASSERT(false, "guard must exist on the map, if it's not, then the input is ill-formed");
            std::unreachable();
        }

        Position next_position(Position pos, Facing facing) const
        {
            switch (facing) {
            case day6::Facing::Up: return { pos.m_x, pos.m_y - 1 };
            case day6::Facing::Right: return { pos.m_x + 1, pos.m_y };
            case day6::Facing::Down: return { pos.m_x, pos.m_y + 1 };
            case day6::Facing::Left: return { pos.m_x - 1, pos.m_y };
            case day6::Facing::Invalid: return pos;
            default: return pos;
            }
        };

        template <std::invocable<Position> Fn>
        std::optional<std::pair<Position, Facing>> guard_next_step(
            Map      map,
            Position pos,
            Facing   facing,
            Fn       obstruction_check
        ) const
        {
            auto in_bound   = [&](Position p) { return p.m_x < map[0].size() and p.m_y < map.size(); };
            auto cycle_face = [](Facing f) {
                auto underlying = std::to_underlying(f);
                auto size       = std::to_underlying(Facing::Left) + 1;
                return static_cast<Facing>((underlying + 1) % size);
            };

            auto new_pos = next_position(pos, facing);
            if (not in_bound(new_pos)) {
                return std::nullopt;
            }
            if (not obstruction_check(new_pos)) {
                return std::pair{ new_pos, facing };
            }

            auto new_face = cycle_face(facing);
            new_pos       = next_position(pos, new_face);

            // there might be second obstruction in the new position
            // NOTE: edge case:
            //              v        #      #
            //     >>#     #v       #<<     ^#
            //      #       #               ^
            if (not in_bound(new_pos)) {
                return std::nullopt;
            }
            if (not obstruction_check(new_pos)) {
                return std::pair{ new_pos, new_face };
            }

            new_face = cycle_face(new_face);
            new_pos  = next_position(pos, new_face);

            return std::pair{ new_pos, new_face };
        }

        bool guard_is_looping(
            Map         map,
            ScratchMap& scratchmap,
            Position    new_obstruction_pos,
            Position    start_pos,
            Facing      start_facing
        ) const
        {
            auto has_obstruction = [&](Position p) {
                return map[p.m_y][p.m_x] == obstruction or new_obstruction_pos == p;
            };

            auto pos    = start_pos;
            auto facing = start_facing;

            while (pos.m_y < map.size() and pos.m_x < map[0].size()) {
                auto next = guard_next_step(map, pos, facing, has_obstruction);
                if (not next) {
                    scratchmap[pos] = facing;
                    break;
                }

                auto [new_pos, new_facing] = *next;

                // the new_position already visited and facing the same direction
                // NOTE: the second check needed to avoid edge case:
                //  >v   or    v<
                //  ^<         >^
                if (scratchmap[new_pos] == new_facing or scratchmap[pos] == new_facing) {
                    scratchmap[pos] = new_facing;
                    return true;
                }

                scratchmap[pos] = new_facing;
                pos             = new_pos;
                facing          = new_facing;
            }

            return false;
        }

        Input parse(common::Lines lines, common::Context /* ctx */) const { return lines; }

        Output solve_part_one(Input input, common::Context /* ctx */) const
        {
            auto scratchmap = ScratchMap{ input[0].size(), input.size(), Facing::Invalid };

            auto facing = Facing::Up;
            auto pos    = find_guard(input);

            while (pos.m_y < input.size() and pos.m_x < input[0].size()) {
                auto next = guard_next_step(input, pos, facing, [&](Position p) {
                    return input[p.m_y][p.m_x] == obstruction;
                });

                if (not next) {
                    scratchmap[pos] = facing;
                    break;
                }

                auto [new_pos, new_facing] = *next;
                scratchmap[pos]            = new_facing;
                pos                        = new_pos;
                facing                     = new_facing;
            }

            auto count = sr::count_if(scratchmap.m_facing, [](auto f) { return f != Facing::Invalid; });
            return static_cast<Output>(count);
        }

        Output solve_part_two(Input input, common::Context /* ctx */) const
        {
            auto scratchmap      = ScratchMap{ input[0].size(), input.size(), Facing::Invalid };
            auto scratchmap_copy = scratchmap;

            const auto initial_pos     = find_guard(input);
            const auto has_obstruction = [&](Position p) { return input[p.m_y][p.m_x] == obstruction; };

            auto facing = Facing::Up;
            auto pos    = initial_pos;

            auto looping_count = 0uz;

            while (pos.m_y < input.size() and pos.m_x < input[0].size()) {
                auto next = guard_next_step(input, pos, facing, has_obstruction);
                if (not next) {
                    scratchmap[pos] = facing;
                    break;
                }

                auto [new_pos, new_facing] = *next;
                scratchmap[pos]            = new_facing;

                // - if the new position is already visited, then we can't place a new obstruction there
                // since if there is an obstruction there, we can't arrive at current position
                // - we can't place obstruction at the initial position
                if (scratchmap[new_pos] == Facing::Invalid and new_pos != initial_pos) {
                    scratchmap_copy  = scratchmap;
                    auto looping     = guard_is_looping(input, scratchmap_copy, new_pos, pos, facing);
                    looping_count   += looping;
                }

                pos    = new_pos;
                facing = new_facing;
            }

            return looping_count;
        }
    };

    static_assert(common::Day<Day06>);
}
