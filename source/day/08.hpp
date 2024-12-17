#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    namespace day8
    {
        template <typename T = al::usize>
        struct Coordinate
        {
            T m_x;
            T m_y;

            auto operator<=>(const Coordinate&) const = default;

            friend auto format_as(const Coordinate& c) { return fmt::format("({:>2}, {:>})", c.m_x, c.m_y); }
        };

        using Antenna           = char;
        using CoordinateVector  = std::vector<Coordinate<>>;
        using AntennaCollection = std::unordered_map<Antenna, CoordinateVector>;

        class AntennaPairIter
        {
        public:
            AntennaPairIter(std::span<const Coordinate<>> antennas_location)
                : m_antennas_location{ antennas_location }
                , m_left_index{ 0 }
                , m_right_index{ 1 }
            {
            }

            std::optional<std::pair<Coordinate<>, Coordinate<>>> next()
            {
                if (m_left_index >= m_antennas_location.size() - 1) {
                    return std::nullopt;
                }

                auto left  = m_antennas_location[m_left_index];
                auto right = m_antennas_location[m_right_index];

                ++m_right_index;
                if (m_right_index >= m_antennas_location.size()) {
                    ++m_left_index;
                    m_right_index = m_left_index + 1;
                }

                return std::optional{ std::pair{ left, right } };
            }

            al::usize permutation_count()
            {
                // number of pairs that can be formed from n elements = n * (n - 1) / 2
                return m_antennas_location.size() * (m_antennas_location.size() - 1) / 2;
            }

        private:
            std::span<const Coordinate<>> m_antennas_location;
            al::usize                     m_left_index;
            al::usize                     m_right_index;
        };

        struct AntennaMap
        {
            AntennaCollection m_antennas;
            al::usize         m_width;
            al::usize         m_height;
        };

        template <typename T>
        Coordinate<std::make_signed_t<T>> distance(const Coordinate<T>& lhs, const Coordinate<T>& rhs)
        {
            using Signed = std::make_signed_t<T>;

            auto&& [lx, ly] = lhs;
            auto&& [rx, ry] = rhs;

            auto x_diff = static_cast<Signed>(rx) - static_cast<Signed>(lx);
            auto y_diff = static_cast<Signed>(ry) - static_cast<Signed>(ly);

            return { x_diff, y_diff };
        }
    }

    struct Day08
    {
        static constexpr auto id         = "08";
        static constexpr auto name       = "resonant-collineariry";
        static constexpr auto no_antenna = '.';

        using Antenna          = day8::Antenna;
        using Coordinate       = day8::Coordinate<>;
        using Diff             = day8::Coordinate<al::isize>;
        using AntennaMap       = day8::AntennaMap;
        using AntennaPairIter  = day8::AntennaPairIter;
        using CoordinateVector = day8::CoordinateVector;

        using Input  = day8::AntennaMap;
        using Output = al::usize;

        Input parse(common::Lines lines) const
        {
            ASSERT(lines.size() > 0);

            auto antenna_map     = AntennaMap{};
            antenna_map.m_width  = lines[0].size();
            antenna_map.m_height = lines.size();

            for (auto [y, line] : lines | sv::enumerate) {
                for (auto [x, antenna] : line | sv::enumerate) {
                    if (antenna != no_antenna) {
                        antenna_map.m_antennas[antenna].emplace_back(x, y);
                    }
                }
            }

            return antenna_map;
        }

        Output solve_part_one(Input input) const
        {
            const auto& [antennas, width, height] = input;
            auto antinodes                        = CoordinateVector{};

            for (const auto& [antenna, locations] : antennas) {
                auto iter = AntennaPairIter{ locations };
                while (auto pair = iter.next()) {
                    auto [left, right] = *pair;
                    if (left > right) {
                        std::swap(left, right);
                    }

                    auto [dx, dy] = distance(left, right);
                    auto u        = [](al::isize x) { return static_cast<al::usize>(x); };

                    antinodes.emplace_back(left.m_x - u(dx), left.m_y - u(dy));
                    antinodes.emplace_back(right.m_x + u(dx), right.m_y + u(dy));
                }
            }

            auto in_bound = [&](auto&& x, auto&& y) { return x < width and y < height; };
            std::erase_if(antinodes, [&](auto&& c) { return not in_bound(c.m_x, c.m_y); });

            std::sort(antinodes.begin(), antinodes.end());
            auto end          = std::unique(antinodes.begin(), antinodes.end());
            auto unique_count = static_cast<al::usize>(std::distance(antinodes.begin(), end));

            return unique_count;
        }

        Output solve_part_two(Input input) const
        {
            const auto& [antennas, width, height] = input;

            auto in_bound  = [&](const Coordinate& c) { return c.m_x < width and c.m_y < height; };
            auto antinodes = CoordinateVector{};

            for (const auto& [antenna, locations] : antennas) {
                auto iter = AntennaPairIter{ locations };
                while (auto pair = iter.next()) {
                    auto [left, right] = *pair;
                    if (left > right) {
                        std::swap(left, right);
                    }

                    // antinodes on the antennas also counted
                    antinodes.push_back(left);
                    antinodes.push_back(right);

                    auto [dx, dy] = distance(left, right);
                    auto u        = [](al::isize x) { return static_cast<al::usize>(x); };

                    left = Coordinate{ left.m_x - u(dx), left.m_y - u(dy) };
                    while (in_bound(left)) {
                        antinodes.push_back(left);
                        left.m_x -= u(dx);
                        left.m_y -= u(dy);
                    }

                    right = Coordinate{ right.m_x + u(dx), right.m_y + u(dy) };
                    while (in_bound(right)) {
                        antinodes.push_back(right);
                        right.m_x += u(dx);
                        right.m_y += u(dy);
                    }
                }
            }

            std::sort(antinodes.begin(), antinodes.end());
            auto end          = std::unique(antinodes.begin(), antinodes.end());
            auto unique_count = static_cast<al::usize>(std::distance(antinodes.begin(), end));

            return unique_count;
        }
    };

    static_assert(common::Day<Day08>);
}
