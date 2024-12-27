#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

#include <queue>
#include <unordered_set>
#include <vector>

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sv = aoc::common::sv;

    namespace day16
    {
        enum class Tile : al::u8
        {
            Empty,
            Wall,
        };

        enum class Direction : al::i8
        {
            North = 0,
            East  = 1,
            South = 2,
            West  = 3
        };

        using Coord = util::Coordinate<al::usize>;
        using Map   = util::Array2D<Tile>;

        struct DirectedCoord
        {
            Coord     m_coord;
            Direction m_dir;

            auto operator<=>(const DirectedCoord&) const = default;
        };

        struct ScoredCoord
        {
            DirectedCoord m_dir_coord;
            al::usize     m_score;

            auto operator<=>(const ScoredCoord&) const = default;
        };

        constexpr auto moves = std::array<DirectedCoord, 4>{ {
            { .m_coord = { 0uz, -1uz }, .m_dir = Direction::North },
            { .m_coord = { 1uz, 0uz }, .m_dir = Direction::East },
            { .m_coord = { 0uz, 1uz }, .m_dir = Direction::South },
            { .m_coord = { -1uz, 0uz }, .m_dir = Direction::West },
        } };

        inline al::isize dir_diff(Direction a, Direction b)
        {
            auto ua = std::to_underlying(a);
            auto ub = std::to_underlying(b);

            return std::abs(ua - ub);
        }
    }

    struct Day16
    {
        static constexpr auto id   = "16";
        static constexpr auto name = "reindeer-maze";

        static constexpr auto score_step = 1;
        static constexpr auto score_turn = 1000;

        using Coord         = day16::Coord;
        using Tile          = day16::Tile;
        using Map           = day16::Map;
        using ScoredCoord   = day16::ScoredCoord;
        using Direction     = day16::Direction;
        using DirectedCoord = day16::DirectedCoord;

        struct Input
        {
            DirectedCoord m_start;
            Coord         m_end;
            Map           m_map;
        };

        using Output = al::usize;

        Input parse(common::Lines lines, common::Context /* ctx */) const
        {
            ASSERT(lines.size() > 0, "file should not be empty!");

            auto width = lines.front().size();
            auto start = std::optional<DirectedCoord>{};
            auto end   = std::optional<Coord>{};

            auto map = Map{ width, lines.size(), Tile::Empty };

            for (auto [y, line] : lines | sv::enumerate) {
                ASSERT(line.size() == width, "all lines must have the same width");
                for (auto [x, ch] : line | sv::enumerate) {
                    auto coord = util::unsigned_coord(x, y);
                    switch (ch) {
                    case '#': map.at(coord) = Tile::Wall; break;
                    case '.': map.at(coord) = Tile::Empty; break;
                    case 'S': start = DirectedCoord{ coord, Direction::East }; break;
                    case 'E': end = coord; break;
                    default: ASSERT(false, fmt::format("input contains invalid character: {:?}", ch));
                    }
                }
            }

            ASSERT(start.has_value(), "start position not found");
            ASSERT(end.has_value(), "end position not found");

            return { *start, *end, std::move(map) };
        }

        Output solve_part_one(Input input, common::Context /* ctx */) const
        {
            // compare the score only so that the priority queue is a min-heap
            using Compare = decltype([](const ScoredCoord& a, const ScoredCoord& b) {
                return a.m_score > b.m_score;
            });

            using PriQueue = std::priority_queue<ScoredCoord, std::vector<ScoredCoord>, Compare>;
            using Visited  = util::Array2D<bool>;

            auto&& [start, end, map] = input;

            auto priority_queue = PriQueue{};
            auto visited        = Visited{ map.m_width, map.m_height, false };

            priority_queue.emplace(start, 0);

            auto in_map = [&](const Coord& coord) {
                return coord.m_x < map.m_width and coord.m_y < map.m_height;
            };

            while (not priority_queue.empty()) {
                auto [current, score] = priority_queue.top();
                priority_queue.pop();

                if (visited.at(current.m_coord)) {
                    continue;
                } else {
                    visited.at(current.m_coord) = true;
                }

                if (current.m_coord == end) {
                    return score;
                }

                for (auto&& [move, dir] : day16::moves) {
                    auto next = DirectedCoord{
                        .m_coord = current.m_coord + move,
                        .m_dir   = dir,
                    };

                    if (not in_map(next.m_coord) or map.at(next.m_coord) == Tile::Wall) {
                        continue;
                    }

                    // the logic
                    auto dir_diff = day16::dir_diff(current.m_dir, next.m_dir);

                    switch (dir_diff) {
                    case 0:    // same direction, step only
                        priority_queue.emplace(next, score + score_step);
                        break;
                    case 2:    // opposite direction, two turns and one step
                        priority_queue.emplace(next, score + 2 * score_turn + score_step);
                        break;
                    case 1:
                    case 3:    // one turn and one step
                        priority_queue.emplace(next, score + score_turn + score_step);
                        break;
                    default:    // invalid value
                        std::unreachable();
                    }
                }
            }

            return -1uz;    // to indicate that the end is not reachable
        }

        Output solve_part_two(Input /* input */, common::Context /* ctx */) const
        {
            return {};    // TODO: implement
        }
    };

    static_assert(common::Day<Day16>);
}
