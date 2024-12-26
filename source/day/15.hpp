#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "concepts.hpp"
#include "util.hpp"

#include <magic_enum/magic_enum.hpp>

#include <utility>

namespace aoc::day
{
    namespace al  = aoc::aliases;
    namespace sr  = aoc::common::sr;
    namespace sv  = aoc::common::sv;
    namespace cnp = aoc::concepts;

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
            Movement m_movement;
            al::u8   m_steps;
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
                    case Thing::Wall: fmt::print("#"); break;
                    case Thing::Box: fmt::print("O"); break;
                    case Thing::Empty: fmt::print("."); break;
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
                const auto [ox, oy] = std::pair{ 1uz, 1uz };

                auto score = 0uz;
                for (auto [x, y] : util::iter_2d(m_width, m_height)) {
                    if ((*this)[x, y] == Thing::Box) {
                        score += mx * (x + ox) + my * (y + oy);
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
                    case ThingWide::BoxLeft: fmt::print("["); break;
                    case ThingWide::BoxRight: fmt::print("]"); break;
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
                DEBUG_ASSERT(steps > 0, "moving 0 steps does not makes sense");

                auto [x, y] = coord;

                switch (movement) {
                case Movement::Right: {
                    auto actual_steps = move_horz(
                        steps,
                        sv::iota(x, m_width),
                        [&](auto of) { return (*this)[of, y]; },
                        [&](auto of) { (*this)[x + of, y] = ThingWide::Empty; },
                        [&, swap = false](auto of) mutable {
                            if (swap) {
                                (*this)[x + of, y] = ThingWide::BoxRight;
                                swap               = false;
                            } else {
                                (*this)[x + of, y] = ThingWide::BoxLeft;
                                swap               = true;
                            }
                        }
                    );
                    return { x + actual_steps, y };
                }

                case Movement::Left: {
                    auto actual_steps = move_horz(
                        steps,
                        sv::iota(0uz, x + 1),
                        [&](auto of) { return (*this)[x - of, y]; },
                        [&](auto of) { (*this)[x - of, y] = ThingWide::Empty; },
                        [&, swap = false](auto of) mutable {
                            if (swap) {
                                (*this)[x - of, y] = ThingWide::BoxLeft;
                                swap               = false;
                            } else {
                                (*this)[x - of, y] = ThingWide::BoxRight;
                                swap               = true;
                            }
                        }
                    );
                    return { x - actual_steps, y };
                }

                case Movement::Up: {
                    auto new_coord = coord;
                    for (auto _ : sv::iota(0uz, steps)) {
                        if (new_coord.m_y == 0uz) {
                            break;
                        }

                        constexpr auto offset = -1uz;
                        if (not move_vert<offset>(new_coord)) {
                            break;
                        }

                        --new_coord.m_y;
                    }
                    return new_coord;
                }

                case Movement::Down: {
                    auto new_coord = coord;
                    for (auto _ : sv::iota(0uz, steps)) {
                        if (new_coord.m_y == m_height - 1) {
                            break;
                        }

                        constexpr auto offset = 1uz;
                        if (not move_vert<offset>(new_coord)) {
                            break;
                        }

                        ++new_coord.m_y;
                    }
                    return new_coord;
                }

                default: [[unlikely]] std::unreachable();
                }
            }

            al::usize move_horz(
                // clang-format off
                al::usize                          steps,
                cnp::Range                    auto range,
                cnp::Fn<ThingWide, al::usize> auto getter,
                cnp::Fn<void, al::usize>      auto empty_setter,
                cnp::Fn<void, al::usize>      auto box_setter
                // clang-format on
            )
            {
                auto empty_count = 0uz;
                auto box_count   = 0uz;

                for (auto i : range | sv::drop(1)) {
                    switch (getter(i)) {
                    case ThingWide::Empty: ++empty_count; break;
                    case ThingWide::BoxLeft: ++box_count; break;
                    case ThingWide::BoxRight: ++box_count; break;
                    case ThingWide::Wall: goto end_move_right;
                    }

                    if (empty_count == steps) {
                        break;
                    }
                }

                if (empty_count == 0uz) {
                    return 0uz;
                }

            end_move_right:    // C can't break a loop from switch without goto...
                for (auto i : sv::iota(0uz, empty_count)) {
                    empty_setter(i + 1);
                }
                for (auto i : sv::iota(0uz, box_count)) {
                    box_setter(i + 1 + empty_count);
                }

                return empty_count;
            }

            template <al::usize Offset>
            bool move_vert(Coord coord)
            {
                auto check = [&, p = this](this auto&& s, al::usize l, al::usize r, al::usize y_new) -> bool {
                    // relies on wraparound to detect below zero
                    if (y_new >= p->m_height or l >= p->m_width or r >= p->m_width) {
                        return false;
                    }

                    auto left_can_move = [&] {
                        switch ((*p)[l, y_new]) {
                        case ThingWide::Empty: return true;
                        case ThingWide::BoxLeft: return s(l, l + 1, y_new + Offset);    // lined up with curr
                        case ThingWide::BoxRight: return s(l - 1, l, y_new + Offset);
                        case ThingWide::Wall: return false;
                        default: [[unlikely]] std::unreachable();
                        }
                    }();

                    auto right_can_move = [&] {
                        switch ((*p)[r, y_new]) {
                        case ThingWide::Empty: return true;
                        case ThingWide::BoxLeft: return s(r, r + 1, y_new + Offset);
                        case ThingWide::BoxRight: return true;    // already visited by the left check above
                        case ThingWide::Wall: return false;
                        default: [[unlikely]] std::unreachable();
                        }
                    }();

                    return left_can_move and right_can_move;
                };

                auto move = [&, p = this](this auto&& s, al::usize l, al::usize r, al::usize y_new) -> void {
                    // relies on wraparound to detect below zero
                    if (y_new >= p->m_height or l >= p->m_width or r >= p->m_width) {
                        return;
                    }

                    switch ((*p)[l, y_new]) {
                    case ThingWide::BoxLeft: s(l, l + 1, y_new + Offset); break;    // lined up with current
                    case ThingWide::BoxRight: s(l - 1, l, y_new + Offset); break;
                    default: /* do nothing */ break;
                    }

                    switch ((*p)[r, y_new]) {
                    case ThingWide::BoxLeft: s(r, r + 1, y_new + Offset); break;
                    case ThingWide::BoxRight: break;    // already visited by the left move above
                    default: /* do nothing */ break;
                    }

                    (*p)[l, y_new]          = ThingWide::BoxLeft;
                    (*p)[r, y_new]          = ThingWide::BoxRight;
                    (*p)[l, y_new - Offset] = ThingWide::Empty;
                    (*p)[r, y_new - Offset] = ThingWide::Empty;
                };

                auto check_and_move = [&](al::usize l, al::usize r, al::usize y_new) {
                    if (not check(l, r, y_new)) {
                        return false;
                    }
                    move(l, r, y_new);
                    return true;
                };

                auto [x, y] = coord;

                switch ((*this)[x, y + Offset]) {
                case ThingWide::Empty: return true;
                case ThingWide::BoxLeft: return check_and_move(x, x + 1, y + 2 * Offset);
                case ThingWide::BoxRight: return check_and_move(x - 1, x, y + 2 * Offset);
                case ThingWide::Wall: return false;
                default: [[unlikely]] std::unreachable();
                }
            }

            al::usize gps_score() const
            {
                const auto [mx, my] = std::pair{ 1uz, 100uz };
                const auto [ox, oy] = std::pair{ 2uz, 1uz };

                auto score = 0uz;
                for (auto [x, y] : util::iter_2d(m_width, m_height)) {
                    if ((*this)[x, y] == ThingWide::BoxLeft) {
                        score += mx * (x + ox) + my * (y + oy);
                    }
                }
                return score;
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

        Input parse(common::Lines lines, common::Context /* ctx */) const
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
            auto count     = 0_u8;
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

                    if (prev.has_value() and prev.value() == next and count < 255) {
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

        Output solve_part_one(Input input, common::Context /* ctx */) const
        {
            auto&& [robot_pos, warehouse, movements] = input;

            for (const auto& [movement, steps] : movements) {
                robot_pos = warehouse.move(robot_pos, movement, steps);
            }

            return warehouse.gps_score();
        }

        Output solve_part_two(Input input, common::Context /* ctx */) const
        {
            auto&& [robot_pos, warehouse, movements] = input;

            auto wide_warehouse = day15::widen(warehouse);
            robot_pos           = { robot_pos.m_x * 2, robot_pos.m_y };

            for (const auto& [movement, steps] : movements) {
                robot_pos = wide_warehouse.move(robot_pos, movement, steps);
            }

            return wide_warehouse.gps_score();
        }
    };

    static_assert(common::Day<Day15>);
}
