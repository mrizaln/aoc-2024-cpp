#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

#include <unordered_map>

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    struct Day05
    {
        static constexpr auto id           = "05";
        static constexpr auto name         = "print-queue";
        static constexpr auto max_line_len = 23;    // the input of 05.txt says so

        using Rules   = std::unordered_map<al::u32, std::vector<al::u32>>;
        using Pages   = std::vector<al::u32>;
        using Updates = std::vector<Pages>;

        struct Input
        {
            Rules   m_rules;
            Updates m_updates;
        };

        using Output = al::u32;

        al::usize correctly_ordered_last_index(const Rules& rules, std::span<const al::u32> pages) const
        {
            auto rule = std::span{ rules.at(pages[0]) };
            for (auto [i, num] : pages | sv::enumerate | sv::drop(1)) {
                if (sr::find(rule, num) == rule.end()) {
                    return static_cast<al::usize>(i);
                }
                rule = rules.at(num);
            }
            return pages.size();
        };

        bool is_correctly_ordered(const Rules& rules, std::span<const al::u32> pages) const
        {
            return correctly_ordered_last_index(rules, pages) == pages.size();
        }

        Input parse(common::Lines lines) const
        {
            auto parsed = Input{};

            auto i = 0uz;
            while (i <= lines.size()) {
                auto line = lines[i++];
                if (line.empty()) {
                    break;
                }

                auto res = util::split_parse_n<al::u32, 2>(line, '|');
                if (not res.is_success()) {
                    throw res.as_error();
                }

                auto [l, r] = std::move(res).as_success().m_val;
                parsed.m_rules[l].emplace_back(r);
            }

            while (i < lines.size()) {
                auto line       = lines[i++];
                auto line_pages = std::vector<al::u32>{};
                line_pages.reserve(max_line_len);

                auto splitter = util::SplitDyn{ line, ',' };
                while (true) {
                    auto res = splitter.next_parse<al::i32>();
                    if (not res) {
                        break;
                    }
                    if (not res->is_success()) {
                        throw res->as_error();
                    }
                    auto value = std::move(*res).as_success().m_val;
                    line_pages.emplace_back(value);
                }

                parsed.m_updates.push_back(std::move(line_pages));
            }

            return parsed;
        }

        Output solve_part_one(Input input) const
        {
            const auto& [rules, updates] = input;
            auto acc_middle_num          = al::u32{ 0 };

            for (std::span pages : updates) {
                if (is_correctly_ordered(rules, pages)) {
                    auto mid        = pages.size() / 2;
                    acc_middle_num += pages[mid];
                }
            }

            return acc_middle_num;
        }

        // TODO: use more efficient algorithm
        Output solve_part_two(Input input) const
        {
            const auto& [rules, updates] = input;

            auto ordered = std::vector<al::u32>{};
            ordered.reserve(max_line_len);

            auto rectify_order = [&](std::span<const al::u32> pages, al::usize unorder_pos) {
                ordered.clear();

                for (auto i : sv::iota(0uz, unorder_pos)) {
                    ordered.push_back(pages[i]);
                }

                // complexity: idk, kinda like n^2?
                while (unorder_pos < pages.size()) {
                    auto reorder_pos = 0uz;
                    auto to_insert   = pages[unorder_pos];
                    auto rule        = std::span{ rules.at(to_insert) };

                    while (true) {
                        auto rule_contains = [&](al::u32 v) { return sr::find(rule, v) != rule.end(); };

                        if (reorder_pos >= ordered.size() or rule_contains(ordered[reorder_pos])) {
                            ordered.insert(ordered.begin() + static_cast<al::isize>(reorder_pos), to_insert);
                            ++unorder_pos;
                            break;
                        } else {
                            ++reorder_pos;
                        }
                    }
                }

                return std::span{ ordered };
            };

            auto acc_middle_num = al::u32{ 0 };

            for (std::span pages : updates) {
                auto idx = correctly_ordered_last_index(rules, pages);
                if (idx != pages.size()) {
                    auto corrected  = rectify_order(pages, idx);
                    auto mid        = corrected.size() / 2;
                    acc_middle_num += corrected[mid];
                }
            }

            return acc_middle_num;
        }
    };

    static_assert(common::Day<Day05>);
}
