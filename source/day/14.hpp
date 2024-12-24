#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;
    namespace fs = aoc::common::fs;

    namespace day14
    {
        using Coord = util::Coordinate<al::i64>;

        // always positive modulo, like in python
        inline al::i64 mod(al::i64 a, al::i64 b)
        {
            DEBUG_ASSERT(b > 0);
            auto ret = a % b;
            return ret >= 0 ? ret : ret + b;
        }

        // always positive modulo, like in python. modulo each component
        inline Coord mod(Coord coord, Coord b)
        {
            return { mod(coord.m_x, b.m_x), mod(coord.m_y, b.m_y) };
        }

        struct Robot
        {
            Coord m_pos;
            Coord m_vel;

            Robot& move(al::i64 step, Coord bound)
            {
                m_pos = mod(m_pos + m_vel * step, bound);
                return *this;
            }
        };

        struct Map
        {
            template <typename Self>
            auto&& operator[](this Self&& self, Coord coord)
            {
                DEBUG_ASSERT(coord.within(
                    { 0, 0 }, { static_cast<al::i64>(self.m_width), static_cast<al::i64>(self.m_height) }
                ));

                auto [x, y] = coord;
                return std::forward<Self>(self)
                    .m_data[static_cast<al::usize>(y) * self.m_width + static_cast<al::usize>(x)];
            }

            template <typename Self>
            auto&& operator[](this Self&& self, util::Coordinate<al::usize> coord)
            {
                DEBUG_ASSERT(coord.within({ 0, 0 }, { self.m_width, self.m_height }));

                auto [x, y] = coord;
                return std::forward<Self>(self)
                    .m_data[static_cast<al::usize>(y) * self.m_width + static_cast<al::usize>(x)];
            }

            void inc_no_wrap(Coord coord)
            {
                auto& v = (*this)[coord];
                if (v < 255) {
                    ++v;
                }
            }

            // the more a cell has other cells around it, the more likely it's in a cluster (the tree)
            // only cells with a value > 0 are considered
            al::usize score_cluster()
            {
                auto score = 0uz;

                for (auto coord : util::iter_2d(m_width, m_height)) {
                    if ((*this)[coord] == 0) {
                        continue;
                    }

                    auto surrounding = 0;
                    for (auto neighbor : util::neumann_neighbors(coord)) {
                        surrounding += (*this)[neighbor] != 0;
                    }

                    score += 1u << surrounding;
                }

                return score;
            }

            void fill(al::u8 value) { sr::fill(m_data, value); }

            void to_ppm(fs::path filename) const
            {
                auto max   = sr::max(m_data);
                auto scale = [&](al::u8 v) { return static_cast<al::u8>(v * 255 / max); };

                auto header  = fmt::format("P3\n{} {}\n255\n", m_width, m_height);
                auto content = std::string{};
                content.reserve(m_width * m_height * 3);

                content.append(header);
                for (auto coord : util::iter_2d(m_width, m_height)) {
                    auto v = scale((*this)[coord]);
                    content.append(fmt::format("{} {} {}\n", v, v, v));
                }

                // DEBUG_ASSERT(not fs::exists(filename), fmt::format("file already exists: {}", filename));
                if (fs::exists(filename)) {
                    fs::remove(filename);
                }
                auto file = std::ofstream{ filename };

                DEBUG_ASSERT(file.is_open(), fmt::format("could not open file: {}", filename));
                file << content;
            }

            al::usize           m_width;
            al::usize           m_height;
            std::vector<al::u8> m_data;
        };
    }

    struct Day14
    {
        static constexpr auto id   = "14";
        static constexpr auto name = "restroom-redoubt";

        using Coord = day14::Coord;
        using Robot = day14::Robot;
        using Map   = day14::Map;

        static constexpr auto timestep = 100uz;
        static constexpr auto map_size = Coord{ 101, 103 };

        using Input  = std::vector<Robot>;
        using Output = al::usize;

        Input parse(common::Lines lines) const
        {
            auto parse = [](std::string_view str) -> Robot {
                auto [p, px, py, v, vx, vy] = util::split_n<6>(str, util::SplitDelim{ " =," }).value();
                auto to_i64                 = [](auto sv) { return util::from_chars<al::i64>(sv).first; };

                return {
                    .m_pos = { to_i64(px), to_i64(py) },
                    .m_vel = { to_i64(vx), to_i64(vy) },
                };
            };

            return lines | sv::transform(parse) | sr::to<std::vector>();
        }

        Output solve_part_one(Input input) const
        {
            //    --> x+
            //  |
            //  v   I   |  II
            //  y+  ---------
            //      III |  IV

            auto quadrant_I   = 0uz;
            auto quadrant_II  = 0uz;
            auto quadrant_III = 0uz;
            auto quadrant_IV  = 0uz;

            const auto [w, h] = map_size;

            for (auto&& robot : input) {
                auto next_pos       = robot.m_pos + robot.m_vel * timestep;
                auto [x, y]         = day14::mod(next_pos, map_size);
                auto [x_mid, y_mid] = std::pair{ w / 2, h / 2 };

                if (x < x_mid and y < y_mid) {
                    ++quadrant_I;
                } else if (x > x_mid and y < y_mid) {
                    ++quadrant_II;
                } else if (x < x_mid and y > y_mid) {
                    ++quadrant_III;
                } else if (x > x_mid and y > y_mid) {
                    ++quadrant_IV;
                }
            }

            return quadrant_I * quadrant_II * quadrant_III * quadrant_IV;
        }

        // assuming the tree is filled, not just an outline
        Output solve_part_two(Input input) const
        {
            const auto [w, h] = map_size;

            auto robots      = input;
            auto scratch_map = Map{
                .m_width  = static_cast<al::usize>(w),
                .m_height = static_cast<al::usize>(h),
                .m_data   = std::vector<al::u8>(w * h, 0x00),
            };

            auto highest_score = std::numeric_limits<al::usize>::min();
            auto highest_index = 0_i64;

            for (auto i : sv::iota(0_i64, w * h)) {
                for (auto&& robot : robots) {
                    auto pos = robot.move(1, map_size).m_pos;
                    scratch_map.inc_no_wrap(pos);
                }
                auto score = scratch_map.score_cluster();
                if (score > highest_score) {
                    highest_score = score;
                    highest_index = i;
                }

                scratch_map.fill(0x00);
            }

            // recreate the map with the highest score
            for (auto robot : input) {
                robot.move(highest_index + 1, map_size);
                scratch_map.inc_no_wrap(robot.m_pos);
            }

            if constexpr (info::is_debug) {
                scratch_map.to_ppm(fmt::format("day14_part2_{:05}.ppm", highest_index + 1));
            }

            return static_cast<al::usize>(highest_index + 1);
        }
    };

    static_assert(common::Day<Day14>);
}
