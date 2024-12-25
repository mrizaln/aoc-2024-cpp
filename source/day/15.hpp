#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

#include <magic_enum/magic_enum.hpp>

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    namespace day15
    {
        using Coord = util::Coordinate<al::usize>;

        enum class Thing : al::u8
        {
            Empty,
            Box,
            Wall,
        };

        enum class Movement : al::u8
        {
            Up,
            Right,
            Down,
            Left,
        };

        struct MovementStep
        {
            Movement  m_movement;
            al::usize m_steps;
        };

        struct Warehouse
        {
            Warehouse(al::usize width, al::usize height)
                : m_width{ width }
                , m_height{ height }
                , m_data(width * height, Thing::Empty)
            {
            }

            template <typename Self>
            auto&& operator[](this Self&& self, al::usize x, al::usize y)
            {
                DEBUG_ASSERT(x < self.m_width and y < self.m_height);
                return std::forward<Self>(self).m_data[y * self.m_width + x];
            }

            template <typename Self>
            auto&& operator[](this Self&& self, Coord coord)
            {
                DEBUG_ASSERT(coord.within({ 0, 0 }, { self.m_width, self.m_height }));

                auto&& [x, y] = coord;
                return std::forward<Self>(self).m_data[y * self.m_width + x];
            }

            void print(Coord robot_pos)
            {
                for (auto coord : util::iter_2d(m_width, m_height)) {
                    const auto& thing = (*this)[coord];

                    if (robot_pos == coord) {
                        DEBUG_ASSERT(thing == Thing::Empty, "robot is on top of a box or wall");
                        fmt::print("@");
                        continue;
                    }

                    switch (thing) {
                    case Thing::Wall: fmt::print("█"); break;
                    case Thing::Box: fmt::print("░"); break;
                    case Thing::Empty: fmt::print(" "); break;
                    default: [[unlikely]] fmt::print("?"); break;
                    }

                    if (coord.m_x == m_width - 1) {
                        fmt::print("\n");
                    }
                }
            }

            Coord move(Coord coord, Movement movement, al::usize steps)
            {
                DEBUG_ASSERT(steps > 0, "moving 0 steps does not makes sense");

                auto push_boxes = [&steps](auto r, auto get, auto empty, auto box) {
                    auto box_count   = 0uz;
                    auto empty_count = 0uz;

                    for (auto i : r | sv::drop(1)) {
                        switch (get(i)) {
                        case Thing::Empty: ++empty_count; break;
                        case Thing::Box: ++box_count; break;
                        case Thing::Wall: goto end_push_boxes;
                        }

                        if (empty_count == steps) {
                            break;
                        }
                    }

                    if (empty_count == 0uz) {
                        return 0uz;
                    }

                end_push_boxes:    // f*ck C and the inability to break a loop from a switch case
                    for (auto i : sv::iota(0uz, empty_count)) {
                        empty(i + 1);
                    }
                    for (auto i : sv::iota(0uz, box_count)) {
                        box(i + 1 + empty_count);
                    }

                    return empty_count;
                };

                auto [x, y] = coord;

                switch (movement) {
                case Movement::Up: {
                    auto actual_steps = push_boxes(
                        sv::iota(0uz, y + 1),
                        [&](auto offset) { return (*this)[x, y - offset]; },
                        [&](auto offset) { (*this)[x, y - offset] = Thing::Empty; },
                        [&](auto offset) { (*this)[x, y - offset] = Thing::Box; }
                    );
                    return { x, y - actual_steps };
                }
                case Movement::Right: {
                    auto actual_steps = push_boxes(
                        sv::iota(x, m_width),
                        [&](auto offset) { return (*this)[offset, y]; },
                        [&](auto offset) { (*this)[x + offset, y] = Thing::Empty; },
                        [&](auto offset) { (*this)[x + offset, y] = Thing::Box; }
                    );
                    return { x + actual_steps, y };
                }
                case Movement::Down: {
                    auto actual_steps = push_boxes(
                        sv::iota(y, m_height),
                        [&](auto offset) { return (*this)[x, offset]; },
                        [&](auto offset) { (*this)[x, y + offset] = Thing::Empty; },
                        [&](auto offset) { (*this)[x, y + offset] = Thing::Box; }
                    );
                    return { x, y + actual_steps };
                }
                case Movement::Left: {
                    auto actual_steps = push_boxes(
                        sv::iota(0uz, x + 1),
                        [&](auto offset) { return (*this)[x - offset, y]; },
                        [&](auto offset) { (*this)[x - offset, y] = Thing::Empty; },
                        [&](auto offset) { (*this)[x - offset, y] = Thing::Box; }
                    );
                    return { x - actual_steps, y };
                }
                default: [[unlikely]] std::unreachable();
                };
            }

            al::usize gps_score() const
            {
                const auto [mx, my] = std::pair{ 1uz, 100uz };

                auto score = 0uz;
                for (auto [x, y] : util::iter_2d(m_width, m_height)) {
                    if ((*this)[x, y] == Thing::Box) {
                        score += mx * (x + 1) + my * (y + 1);
                    }
                }
                return score;
            }

            al::usize          m_width;
            al::usize          m_height;
            std::vector<Thing> m_data;
        };

        enum class ThingWide : al::u8
        {
            Empty,
            BoxLeft,
            BoxRight,
            Wall,
        };

        struct WarehouseWide
        {
            WarehouseWide(al::usize width, al::usize height)
                : m_width{ width }
                , m_height{ height }
                , m_data(width * height, ThingWide::Empty)
            {
            }

            template <typename Self>
            auto&& operator[](this Self&& self, al::usize x, al::usize y)
            {
                DEBUG_ASSERT(x < self.m_width and y < self.m_height);
                return std::forward<Self>(self).m_data[y * self.m_width + x];
            }

            template <typename Self>
            auto&& operator[](this Self&& self, Coord coord)
            {
                DEBUG_ASSERT(coord.within({ 0, 0 }, { self.m_width, self.m_height }));

                auto&& [x, y] = coord;
                return std::forward<Self>(self).m_data[y * self.m_width + x];
            }

            void print(Coord robot_pos)
            {
                for (auto coord : util::iter_2d(m_width, m_height)) {
                    const auto& thing = (*this)[coord];

                    if (robot_pos == coord) {
                        DEBUG_ASSERT(thing == ThingWide::Empty, "robot is on top of a box or wall");
                        fmt::print("@");
                        continue;
                    }

                    switch (thing) {
                    case ThingWide::Wall: fmt::print("█"); break;
                    case ThingWide::BoxLeft: fmt::print("░"); break;
                    case ThingWide::BoxRight: fmt::print("░"); break;
                    case ThingWide::Empty: fmt::print(" "); break;
                    default: [[unlikely]] fmt::print("?"); break;
                    }

                    if (coord.m_x == m_width - 1) {
                        fmt::print("\n");
                    }
                }
            }

            Coord move(Coord coord, Movement movement, al::usize steps)
            {
                return { 0, 0 };    // TODO: implement
            }

            al::usize gps_score() const
            {
                return {};    // TODO: implement
            }

            al::usize              m_width;
            al::usize              m_height;
            std::vector<ThingWide> m_data;
        };

        inline WarehouseWide widen(const Warehouse& warehouse)
        {
            auto wide = WarehouseWide{ warehouse.m_width * 2, warehouse.m_height };

            for (auto [x, y] : util::iter_2d(warehouse.m_width, warehouse.m_height)) {
                switch (warehouse[x, y]) {
                case Thing::Wall:
                    wide[x * 2, y]     = ThingWide::Wall;
                    wide[x * 2 + 1, y] = ThingWide::Wall;
                    break;
                case Thing::Box:
                    wide[x * 2, y]     = ThingWide::BoxLeft;
                    wide[x * 2 + 1, y] = ThingWide::BoxRight;
                    break;
                case Thing::Empty:
                    wide[x * 2, y]     = ThingWide::Empty;
                    wide[x * 2 + 1, y] = ThingWide::Empty;
                    break;
                default:
                    [[unlikely]] DEBUG_ASSERT(
                        false,
                        fmt::format(
                            "invalid thing: {} at ({}, {})", std::to_underlying(warehouse[x, y]), x, y
                        )
                    );
                }
            }

            return wide;
        }
    };

    struct Day15
    {
        static constexpr auto id   = "15";
        static constexpr auto name = "warehouse-woes";

        using Coord        = day15::Coord;
        using Thing        = day15::Thing;
        using Movement     = day15::Movement;
        using MovementStep = day15::MovementStep;
        using Warehouse    = day15::Warehouse;

        struct Input
        {
            Coord                     m_robot_pos;
            Warehouse                 m_warehouse;
            std::vector<MovementStep> m_movements;
        };

        using Output = al::usize;

        Input parse(common::Lines lines) const
        {
            ASSERT(not lines.empty(), "can't use empty input");

            auto width = lines.front().size() - 2;    // ignore the left/right walls

            auto map_end = sr::find_if(lines | sv::drop(1), [&](auto line) {
                ASSERT(line.size() == width + 2, "invalid line length");
                return sr::all_of(line, [](auto c) { return c == '#'; });
            });

            auto height = static_cast<al::usize>(map_end - lines.begin() - 1);

            auto robot_pos = std::optional<Coord>{};
            auto warehouse = Warehouse{ width, height };

            // ignore the top/bottom walls
            for (auto [y, line] : util::subrange(lines, 1, height + 1) | sv::enumerate) {
                // ignore the left/right walls
                for (auto [x, ch] : util::subrange(line, 1, width + 1) | sv::enumerate) {
                    auto coord = Coord{ static_cast<al::usize>(x), static_cast<al::usize>(y) };
                    switch (ch) {
                    case '#': warehouse[coord] = Thing::Wall; break;
                    case 'O': warehouse[coord] = Thing::Box; break;
                    case '@': robot_pos = coord; break;
                    default: /* do nothing */ break;
                    }
                }
            }

            auto movements = std::vector<MovementStep>{};
            auto count     = 0uz;
            auto prev      = std::optional<Movement>{};

            for (auto line : lines | sv::drop(height + 3)) {
                ASSERT(not line.empty(), "movement instructions can't be empty");
                for (auto ch : line) {
                    auto next = [&] {
                        switch (ch) {
                        case '^': return Movement::Up;
                        case '>': return Movement::Right;
                        case 'v': return Movement::Down;
                        case '<': return Movement::Left;
                        default: ASSERT(false, "invalid movement instruction"); std::unreachable();
                        }
                    }();

                    if (prev.has_value() and prev.value() == next) {
                        ++count;
                    } else {
                        if (prev.has_value()) {
                            movements.emplace_back(*prev, count);
                        }
                        prev  = next;
                        count = 1;
                    }
                }
            }

            if (prev.has_value()) {
                movements.emplace_back(*prev, count);
            }

            ASSERT(robot_pos.has_value(), "robot not found");

            return {
                .m_robot_pos = *robot_pos,
                .m_warehouse = warehouse,
                .m_movements = movements,
            };
        }

        Output solve_part_one(Input input) const
        {
            auto&& [robot_pos, warehouse, movements] = input;

            for (auto [movement, steps] : movements) {
                robot_pos = warehouse.move(robot_pos, movement, steps);
            }

            if constexpr (::aoc::info::is_debug) {
                fmt::println("'{}' final state", name);
                warehouse.print(robot_pos);
            }

            return warehouse.gps_score();
        }

        Output solve_part_two(Input input) const
        {
            auto&& [robot_pos, warehouse, movements] = input;

            auto wide_warehouse = day15::widen(warehouse);
            robot_pos           = { robot_pos.m_x * 2, robot_pos.m_y };

            wide_warehouse.print(robot_pos);

            return {};    // TODO: implement
        }
    };

    static_assert(common::Day<Day15>);
}
