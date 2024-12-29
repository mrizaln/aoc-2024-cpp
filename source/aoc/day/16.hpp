#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

#include <queue>
#include <unordered_set>
#include <vector>

namespace aoc::day
{
    namespace al  = aoc::aliases;
    namespace sv  = aoc::common::sv;
    namespace cnp = aoc::concepts;

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

        struct Map : util::Array2D<Tile>
        {
            Map(al::usize width, al::usize height, Tile default_val)
                : util::Array2D<Tile>{ width, height, default_val }
            {
            }

            bool bounded(const Coord& coord) const { return coord.m_x < m_width and coord.m_y < m_height; }

            void print(std::optional<std::unordered_set<Coord>> best_paths) const
            {
                for (auto [c, v] : util::Array2D<Tile>::iter_enumerate()) {
                    if (best_paths.has_value() and best_paths->contains(c)) {
                        fmt::print("█");
                    } else {
                        switch (v) {
                        case Tile::Empty: fmt::print(" "); break;
                        case Tile::Wall: fmt::print("░"); break;
                        default: std::unreachable();
                        }
                    }

                    if (c.m_x == m_width - 1) {
                        fmt::print("\n");
                    }
                }
            }
        };

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

        struct BestScoreMap
        {
            using Dirs   = std::array<al::usize, 4>;
            using Scores = util::Array2D<Dirs>;

            static constexpr auto default_scores = Dirs{
                std::numeric_limits<al::usize>::min(),
                std::numeric_limits<al::usize>::min(),
                std::numeric_limits<al::usize>::min(),
                std::numeric_limits<al::usize>::min(),
            };

            BestScoreMap(al::usize width, al::usize height)
                : m_scores{ width, height, default_scores }
            {
            }

            template <typename Self>
            auto&& at(this Self&& self, const DirectedCoord& dc)
            {
                auto dir = static_cast<al::usize>(dc.m_dir);
                return std::forward<Self>(self).m_scores.at(dc.m_coord)[dir];
            }

            Scores m_scores;
        };

        struct Visited
        {
            using Dirs   = al::u8;
            using Visits = util::Array2D<Dirs>;

            Visited(al::usize width, al::usize height)
                : m_visits{ width, height, 0 }
            {
            }

            void visit(const DirectedCoord& dc)
            {
                auto  udir  = std::to_underlying(dc.m_dir);
                auto& dirs  = m_visits.at(dc.m_coord);
                dirs       |= 1 << udir;
            }

            bool is_visited(const DirectedCoord& dc) const
            {
                auto  udir = std::to_underlying(dc.m_dir);
                auto& dirs = m_visits.at(dc.m_coord);
                return (dirs & (1 << udir)) != 0;
            }

            Visits m_visits;
        };

        // compare the score only so that the priority queue is a min-heap
        struct Compare
        {
            using S = ScoredCoord;
            bool operator()(const S& p1, const S& p2) const { return p1.m_score > p2.m_score; }
        };

        using PriorityQueue = std::priority_queue<ScoredCoord, std::vector<ScoredCoord>, Compare>;

        constexpr auto moves = std::array<DirectedCoord, 4>{ {
            { .m_coord = { 0uz, -1uz }, .m_dir = Direction::North },
            { .m_coord = { 1uz, 0uz }, .m_dir = Direction::East },
            { .m_coord = { 0uz, 1uz }, .m_dir = Direction::South },
            { .m_coord = { -1uz, 0uz }, .m_dir = Direction::West },
        } };

        inline Coord move_from_dir(Direction dir)
        {
            auto udir = static_cast<al::usize>(dir);
            return moves[udir].m_coord;
        }

        template <cnp::Fn<void, PriorityQueue&, const ScoredCoord&> Logic>
        std::optional<ScoredCoord> dijkstra(const Map& map, ScoredCoord start, Coord end, Logic logic)
        {
            auto priority_queue = PriorityQueue{};
            auto visited        = Visited{ map.m_width, map.m_height };

            priority_queue.push(start);

            while (not priority_queue.empty()) {
                auto current = priority_queue.top();
                priority_queue.pop();

                if (visited.is_visited(current.m_dir_coord)) {
                    continue;
                } else {
                    visited.visit(current.m_dir_coord);
                }

                if (current.m_dir_coord.m_coord == end) {
                    return current;
                }

                logic(priority_queue, current);
            }

            return std::nullopt;
        }

        inline al::isize dir_diff(Direction a, Direction b)
        {
            auto ua = std::to_underlying(a);
            auto ub = std::to_underlying(b);

            return std::abs(ua - ub);
        }
    }

    /// reference implementation:
    /// https://github.com/vss2sn/advent_of_code/blob/f94d7f5ca09e351e5f7698a2d26776307fdca229/2024/cpp/day_16b.cpp
    struct Day16
    {
        static constexpr auto id   = "16";
        static constexpr auto name = "reindeer-maze";

        static constexpr auto score_step      = 1;
        static constexpr auto score_turn      = 1000;
        static constexpr auto unreachable_end = day16::Coord{ -1uz, -1uz };

        using Coord         = day16::Coord;
        using Tile          = day16::Tile;
        using Map           = day16::Map;
        using ScoredCoord   = day16::ScoredCoord;
        using Direction     = day16::Direction;
        using DirectedCoord = day16::DirectedCoord;
        using PriorityQueue = day16::PriorityQueue;
        using Visited       = day16::Visited;
        using BestScoreMap  = day16::BestScoreMap;

        struct Input
        {
            DirectedCoord m_start;
            Coord         m_end;
            Map           m_map;
        };

        using Output = std::optional<al::usize>;

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
            auto&& [start, end, map] = input;

            auto logic = [&](PriorityQueue& pq, const ScoredCoord& current) {
                auto [dir_coord, score] = current;

                for (auto&& [move, dir] : day16::moves) {
                    auto next = DirectedCoord{
                        .m_coord = dir_coord.m_coord + move,
                        .m_dir   = dir,
                    };

                    if (not map.bounded(next.m_coord) or map.at(next.m_coord) == Tile::Wall) {
                        continue;
                    }

                    // the actual logic
                    switch (dir_diff(dir_coord.m_dir, next.m_dir)) {
                    case 0: pq.emplace(next, score + score_step); break;
                    case 2: pq.emplace(next, score + 2 * score_turn + score_step); break;
                    case 1:
                    case 3: pq.emplace(next, score + score_turn + score_step); break;
                    default: std::unreachable();
                    }
                }
            };

            return day16::dijkstra(map, { start, 0 }, end, logic)
                .transform(al::Proj{ &ScoredCoord::m_score });
        }

        Output solve_part_two(Input input, common::Context ctx) const
        {
            auto&& [start, end, map] = input;

            auto reach_end = [&]() -> std::optional<ScoredCoord> {
                auto logic = [&](PriorityQueue& pq, const ScoredCoord& current) {
                    auto [dir_coord, score] = current;

                    for (auto&& [move, dir] : day16::moves) {
                        auto next = DirectedCoord{
                            .m_coord = dir_coord.m_coord + move,
                            .m_dir   = dir,
                        };

                        if (not map.bounded(next.m_coord) or map.at(next.m_coord) == Tile::Wall) {
                            continue;
                        }

                        // the actual logic
                        switch (dir_diff(dir_coord.m_dir, next.m_dir)) {
                        case 0: pq.emplace(next, score + score_step); break;
                        case 2: pq.emplace(next, score + 2 * score_turn + score_step); break;
                        case 1:
                        case 3: pq.emplace(next, score + score_turn + score_step); break;
                        default: std::unreachable();
                        }
                    }
                };

                return day16::dijkstra(map, { start, 0 }, end, logic);
            };

            auto find_best_score_for_all = [&](DirectedCoord new_start) -> BestScoreMap {
                auto best_score_map = BestScoreMap{ map.m_width, map.m_height };

                auto logic = [&](PriorityQueue& pq, const ScoredCoord& current) {
                    best_score_map.at(current.m_dir_coord) = current.m_score;

                    auto [dir_coord, score] = current;

                    auto move = move_from_dir(current.m_dir_coord.m_dir);
                    auto next = DirectedCoord{
                        .m_coord = dir_coord.m_coord - move,
                        .m_dir   = dir_coord.m_dir,
                    };

                    if (not map.bounded(next.m_coord) or map.at(next.m_coord) == Tile::Wall) {
                        return;
                    }

                    auto push_on_dir = [&](Direction dir) {
                        next.m_dir = dir;

                        switch (dir_diff(dir_coord.m_dir, next.m_dir)) {
                        case 0: pq.emplace(next, score + score_step); break;
                        case 2: pq.emplace(next, score + 2 * score_turn + score_step); break;
                        case 1:
                        case 3: pq.emplace(next, score + score_turn + score_step); break;
                        default: std::unreachable();
                        }
                    };

                    push_on_dir(Direction::North);
                    push_on_dir(Direction::East);
                    push_on_dir(Direction::South);
                    push_on_dir(Direction::West);
                };

                // unreachable_end since I want to traverse all paths
                auto end = day16::dijkstra(map, { new_start, 0 }, unreachable_end, logic);
                ASSERT(not end.has_value(), "end should not be reached");

                return best_score_map;
            };

            auto traverse_all_best_path = [&](const BestScoreMap& best_scores) -> std::unordered_set<Coord> {
                auto visited = std::unordered_set<Coord>{};

                auto logic = [&](PriorityQueue& pq, const ScoredCoord& current) {
                    auto [dir_coord, score] = current;

                    visited.insert(dir_coord.m_coord);

                    for (auto&& [move, dir] : day16::moves) {
                        auto next = DirectedCoord{
                            .m_coord = dir_coord.m_coord + move,
                            .m_dir   = dir,
                        };

                        if (not map.bounded(next.m_coord) or map.at(next.m_coord) == Tile::Wall) {
                            continue;
                        }

                        auto push_if = [&](DirectedCoord next, al::usize score) {
                            if (best_scores.at(next) == score) {
                                pq.emplace(next, score);
                            }
                        };

                        // the actual logic
                        switch (dir_diff(dir_coord.m_dir, next.m_dir)) {
                        case 0: push_if(next, score - score_step); break;
                        case 2: push_if(next, score - 2 * score_turn - score_step); break;
                        case 1:
                        case 3: push_if(next, score - score_turn - score_step); break;
                        default: std::unreachable();
                        }
                    }
                };

                auto best = best_scores.at(start);
                auto end  = day16::dijkstra(map, { start, best }, unreachable_end, logic);
                ASSERT(not end.has_value(), "end should not be reached");

                return visited;
            };

            auto end_scored_coord = reach_end();
            if (not end_scored_coord.has_value()) {
                return std::nullopt;
            }

            auto best_score_map = find_best_score_for_all(end_scored_coord->m_dir_coord);
            auto best_paths     = traverse_all_best_path(best_score_map);

            if (ctx.is_debug() and not ctx.is_benchmark()) {
                map.print(best_paths);
            }

            return best_paths.size();
        }
    };

    static_assert(common::Day<Day16>);
}
