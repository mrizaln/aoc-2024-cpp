#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

#include <deque>
#include <unordered_set>
#include <vector>

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sv = aoc::common::sv;

    namespace day12
    {
        using Coord = util::Coordinate<al::usize>;

        struct Map
        {
            al::usize     m_width;
            al::usize     m_height;
            common::Lines m_lines;
        };

        struct Region
        {
            char      m_name;
            al::usize m_area;
            al::usize m_fence;
        };

        struct Region2
        {
            char                      m_name;
            al::usize                 m_corners;
            std::unordered_set<Coord> m_area;
        };

        struct Visited
        {
            Visited(al::usize width, al::usize height)
                : m_width(width)
                , m_height(height)
                , m_visited(width * height, 0x00)
            {
            }

            bool is_visited(const Coord& coord) const
            {
                auto&& [x, y] = coord;
                return m_visited[y * m_width + x];
            }

            void visit(const Coord& coord)
            {
                auto&& [x, y]              = coord;
                m_visited[y * m_width + x] = true;
            }

            al::usize         m_width;
            al::usize         m_height;
            std::vector<bool> m_visited;
        };
    }

    struct Day12
    {
        static constexpr auto id   = "12";
        static constexpr auto name = "garden-groups";

        using Coord   = day12::Coord;
        using Visited = day12::Visited;
        using Map     = day12::Map;
        using Region  = day12::Region;
        using Region2 = day12::Region2;

        using Input  = Map;
        using Output = al::usize;

        Input parse(common::Lines lines, common::Context /* ctx */) const
        {
            ASSERT(lines.size() > 0, "can't process an empty input :(");
            auto width = lines[0].size();

            for (auto [y, line] : lines | sv::enumerate) {
                ASSERT(line.size() == width, "every line must have the same width");
            }

            return { width, lines.size(), lines };
        }

        Output solve_part_one(Input input, common::Context /* ctx */) const
        {
            auto&& [width, height, map] = input;

            const auto min = Coord{ 0, 0 };
            const auto max = Coord{ width, height };

            auto visited = Visited{ input.m_width, input.m_height };
            auto queue   = std::deque<Coord>{};

            auto find_region = [&](const Coord& coord, char name) -> Region {
                auto region = Region{ name, 0uz, 0uz };
                auto outers = std::unordered_set<Coord>{};

                queue.push_back(coord);

                while (not queue.empty()) {
                    auto current = queue.front();
                    queue.pop_front();

                    if (not current.within(min, max) or map[current.m_y][current.m_x] != name) {
                        ++region.m_fence;
                        continue;
                    }
                    if (visited.is_visited(current)) {
                        continue;
                    }

                    visited.visit(current);
                    ++region.m_area;

                    for (auto&& [i, neighbor] : util::neumann_neighbors(current) | sv::enumerate) {
                        queue.push_back(neighbor);
                    }
                }

                return region;
            };

            auto price = 0uz;
            for (auto&& coord : util::iter_2d(width, height)) {
                if (not visited.is_visited(coord)) {
                    auto reg  = find_region(coord, map[coord.m_y][coord.m_x]);
                    price    += reg.m_area * reg.m_fence;
                }
            }
            return price;
        }

        Output solve_part_two(Input input, common::Context /* ctx */) const
        {
            auto&& [width, height, map] = input;

            const auto min = Coord{ 0, 0 };
            const auto max = Coord{ width, height };

            auto visited = Visited{ input.m_width, input.m_height };
            auto queue   = std::deque<Coord>{};

            auto find_region = [&](const Coord& coord, char name) -> Region2 {
                auto region = Region2{ name, 0uz, {} };
                auto outers = std::unordered_set<Coord>{};

                queue.push_back(coord);

                while (not queue.empty()) {
                    auto current = queue.front();
                    queue.pop_front();

                    if (not current.within(min, max)                //
                        or map[current.m_y][current.m_x] != name    //
                        or visited.is_visited(current)) {
                        continue;
                    }

                    visited.visit(current);
                    region.m_area.insert(current);

                    // for convex corners
                    auto adj = 0_u32;
                    for (auto&& [i, neighbor] : util::neumann_neighbors(current) | sv::enumerate) {
                        if (not neighbor.within(min, max)) {
                            adj |= 1 << i;
                        } else if (map[neighbor.m_y][neighbor.m_x] != name) {
                            adj |= 1 << i;
                            outers.insert(neighbor);
                        }
                        queue.push_back(neighbor);
                    }

                    //   0bWSEN
                    switch (adj) {
                    case 0b0011:
                    case 0b0110:
                    case 0b1100:
                    case 0b1001: region.m_corners += 1; break;
                    case 0b0111:
                    case 0b1110:
                    case 0b1101:
                    case 0b1011: region.m_corners += 2; break;
                    case 0b1111: region.m_corners += 4; break;
                    }
                }

                // for concave corners
                for (const auto& out : outers) {
                    auto adj = 0_u32;
                    for (const auto& [i, n] : util::neumann_neighbors(out) | sv::enumerate) {
                        if (n.within(min, max) and map[n.m_y][n.m_x] == name) {
                            adj |= 1 << i;
                        }
                    }

                    using D = util::NeighborDir;

                    // concave corners require additional check on whether the corner is a corner of the
                    // region of interest, that's why the area itself as coordinate is required instead of
                    // simple name check
                    auto in_region = [&](D d) {
                        auto c = util::neighbor_by_dir(out, d);
                        return c.within(min, max) and region.m_area.contains(c);
                    };

                    auto& r = in_region;
                    auto& c = region.m_corners;

                    // the diagonals might be not part of the region, so the check here is required

                    //   0bWSEN
                    switch (adj) {
                    case 0b0011: c += r(D::NE); break;
                    case 0b0110: c += r(D::SE); break;
                    case 0b1100: c += r(D::SW); break;
                    case 0b1001: c += r(D::NW); break;
                    case 0b0111: c += r(D::NE) + r(D::SE); break;
                    case 0b1110: c += r(D::SE) + r(D::SW); break;
                    case 0b1101: c += r(D::SW) + r(D::NW); break;
                    case 0b1011: c += r(D::NW) + r(D::NE); break;
                    case 0b1111: c += r(D::NE) + r(D::SE) + r(D::SW) + r(D::NW); break;
                    }
                }

                return region;
            };

            auto price = 0uz;
            for (auto&& coord : util::iter_2d(width, height)) {
                if (not visited.is_visited(coord)) {
                    auto reg  = find_region(coord, map[coord.m_y][coord.m_x]);
                    price    += reg.m_area.size() * reg.m_corners;    // num corners == num edges
                }
            }

            return price;
        }
    };

    static_assert(common::Day<Day12>);
}
