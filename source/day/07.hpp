#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sv = aoc::common::sv;

    struct Day07
    {
        static constexpr auto id   = "07";
        static constexpr auto name = "bridge-repair";

        static constexpr auto max_operands  = 12;
        static constexpr auto invalid_value = std::numeric_limits<al::u64>::max();

        struct Operands
        {
            std::array<al::u64, max_operands> m_values;
            al::u64                           m_count;

            auto get() { return m_values | sv::take(m_count); }
        };

        struct Equation
        {
            al::u64  m_expect;
            Operands m_operands;
        };

        struct PermutatedOperation
        {
            template <std::ranges::sized_range R>
            al::u64 operate(R&& r)
            {
                auto op = [&](al::u64 v, al::isize i) {
                    switch ((m_op_perm >> i) & 1) {
                    case 0: return v + r[i + 1]; break;
                    case 1: return v * r[i + 1]; break;
                    default: [[unlikely]] std::abort(); break;
                    }
                };

                auto result = r[0];
                for (auto i : sv::iota(0uz, std::ranges::size(r) - 1)) {
                    result = op(result, static_cast<al::isize>(i));
                }

                ++m_op_perm;

                return result;
            }

            void reset() { m_op_perm = 0; }

            al::u16 m_op_perm = {};
        };

        struct PermutatedOperation3
        {
            enum class Op : al::u8
            {
                Add,
                Mul,
                Concat,
            };

            template <std::ranges::sized_range R>
            al::u64 operate(R&& r)
            {
                // there are only 3 digits per operand on the right, I add more checks til 100000 just in-case
                auto concat = [](al::u64 l, al::u64 r) {
                    static auto pow10  = std::array{ 1uz, 10uz, 100uz, 1000uz, 10000uz, 100000uz };
                    auto        digits = [](al::u64 v) {
                        // clang-format off
                        if (v < 10)     return 1uz;
                        if (v < 100)    return 2uz;
                        if (v < 1000)   return 3uz;
                        if (v < 10000)  return 4uz;
                        if (v < 100000) return 5uz;
                        else            std::abort();
                        // clang-format on
                    };

                    return l * pow10[digits(r)] + r;
                };

                auto op = [&](al::u64 v, al::isize i) {
                    switch (m_op_perm[static_cast<al::usize>(i)]) {
                    case Op::Add: return v + r[i + 1]; break;
                    case Op::Mul: return v * r[i + 1]; break;
                    case Op::Concat: return concat(v, r[i + 1]); break;
                    default: [[unlikely]] std::abort(); break;
                    }
                };

                auto result = r[0];
                for (auto i : sv::iota(0uz, std::ranges::size(r) - 1)) {
                    result = op(result, static_cast<al::isize>(i));
                }

                next_perm();

                return result;
            }

            void next_perm()
            {
                auto cycle = [](Op& op) {
                    auto res = std::div(static_cast<int>(op) + 1, 3);
                    op       = static_cast<Op>(res.rem);
                    return res.quot != 0;
                };

                for (auto& op : m_op_perm) {
                    if (not cycle(op)) {
                        break;
                    }
                }
            }

            void reset() { m_op_perm.fill(Op::Add); }

            std::array<Op, max_operands> m_op_perm = {};
        };

        using Input  = std::vector<Equation>;
        using Output = al::u64;

        Input parse(common::Lines lines) const
        {
            auto input = Input{};

            for (auto line : lines) {
                auto res = util::split_n<2>(line, ':');
                if (not res) {
                    throw std::runtime_error{ "Failed to parse input" };
                }
                auto [expect_str, operands_str] = *res;

                auto [expect, ec] = util::from_chars<al::u64>(expect_str);
                if (ec != std::errc{}) {
                    auto err_code = std::make_error_code(ec);
                    throw std::runtime_error{ err_code.message() };
                }

                auto ops = util::split_part_parse_n<al::u64, max_operands>(operands_str, ' ', invalid_value);
                auto [parsed, count] = std::move(ops).as_success();

                auto operands = Operands{ .m_values = std::move(parsed), .m_count = count };
                input.emplace_back(expect, operands);
            }

            return input;
        }

        Output solve_part_one(Input input) const
        {
            auto result  = 0uz;
            auto perm_op = PermutatedOperation{};

            for (auto [expect, ops] : input) {
                for (auto _ : sv::iota(0uz, 1uz << (ops.m_count - 1))) {
                    if (perm_op.operate(ops.get()) == expect) {
                        result += expect;
                        break;
                    }
                }
                perm_op.reset();
            }

            return result;
        }

        Output solve_part_two(Input input) const
        {
            auto result  = 0uz;
            auto perm_op = PermutatedOperation3{};

            // I don't believe in std::pow for exact integers :>
            // since the max operands is 12, this is ok
            static auto pow3 = std::array{
                1uz, 3uz, 9uz, 27uz, 81uz, 243uz, 729uz, 2187uz, 6561uz, 19683uz, 59049uz, 177147uz,
            };
            static_assert(pow3.size() == max_operands, "pow3 size must match max_operands");

            for (auto [expect, ops] : input) {
                for (auto _ : sv::iota(0uz, pow3[ops.m_count - 1])) {
                    if (perm_op.operate(ops.get()) == expect) {
                        result += expect;
                        break;
                    }
                }
                perm_op.reset();
            }

            return result;
        }
    };

    static_assert(common::Day<Day07>);
}
