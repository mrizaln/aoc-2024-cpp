#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include <charconv>

namespace aoc::day
{
    namespace al = aoc::aliases;

    namespace day3
    {
        struct MulParser
        {
            al::i64 parse()
            {
                auto acc = al::i64{ 0 };

                for (auto i = 0uz; i < m_str.size() - 3;) {
                    auto slice = m_str.substr(i, 3);

                    if (slice == "mul") {
                        // all mul go proceed parse operands
                        auto [result, next]  = parse_operands(i + 3);
                        acc                 += result;
                        i                    = next;
                    } else if (slice[1] == 'm' and slice[2] == 'u') {
                        // possibly mul offset by 1
                        i += 1;
                    } else if (slice[2] == 'm') {
                        // possibly mul offset by 2
                        i += 2;
                    } else {
                        // no mul found, skip to next
                        i += 3;
                    }
                }

                return acc;
            }

            // returns the mul operation result and the next index to start parsing
            std::pair<al::i64, al::usize> parse_operands(al::usize start)
            {
                if (start >= m_str.size() or m_str[start] != '(') {
                    // no left paren, skip to next
                    return { 0, start + 1 };
                }

                auto [num1, next1] = parse_num(start + 1);

                if (start >= m_str.size() or m_str[next1] != ',') {
                    // no comma, skip to next
                    return { 0, next1 + 1 };
                }

                auto [num2, next2] = parse_num(next1 + 1);

                if (start >= m_str.size() or m_str[next2] != ')') {
                    // no right paren, skip to next
                    return { 0, next2 + 1 };
                }

                return { num1 * num2, next2 + 1 };
            }

            std::pair<al::i64, al::usize> parse_num(al::usize start)
            {
                auto storage     = std::array<char, 3>{};
                auto storage_idx = 0uz;

                while (start < m_str.size() and storage_idx < 3) {
                    if (std::isdigit(m_str[start])) {
                        storage[storage_idx++]  = m_str[start];
                        start                  += 1;
                    } else {
                        break;
                    }
                }

                if (storage_idx == 0) {
                    return { 0, start };
                }

                auto num = al::i64{ 0 };
                std::from_chars(storage.data(), storage.data() + storage_idx, num);

                return { num, start };
            }

            std::string_view m_str;
        };
    }

    struct Day03
    {
        static constexpr auto id   = "03";
        static constexpr auto name = "mull-it-over";

        using Input  = common::Lines;
        using Output = al::i64;

        Input parse(common::Lines lines) const { return lines; }

        Output solve_part_one(Input input) const
        {
            auto acc = al::i64{ 0 };

            for (auto&& line : input) {
                auto parser  = day3::MulParser{ line };
                acc         += parser.parse();
            }

            return acc;
        }

        Output solve_part_two(Input /* input */) const
        {
            return {};    // TODO: implement
        }
    };

    static_assert(common::Day<Day03>);
}
