#pragma once

#include "aliases.hpp"
#include "common.hpp"
#include "util.hpp"

namespace aoc::day
{
    namespace al = aoc::aliases;
    namespace sr = aoc::common::sr;
    namespace sv = aoc::common::sv;

    namespace day9
    {
        template <al::usize Sentinel>
        inline al::usize checksum(std::span<const al::usize> memory)
        {
            auto sum = 0uz;
            for (auto [i, v] : memory | sv::enumerate) {
                if (v == Sentinel) {
                    continue;
                }
                sum += v * static_cast<al::usize>(i);
            }
            return sum;
        }
    }

    struct Day09
    {
        static constexpr auto id    = "09";
        static constexpr auto name  = "disk-fragmenter";
        static constexpr auto empty = std::numeric_limits<al::usize>::max();

        using Input  = std::string_view;
        using Output = al::usize;

        Input parse(common::Lines lines, common::Context /* ctx */) const
        {
            ASSERT(lines.size() >= 1);
            return lines[0];
        }

        Output solve_part_one(Input input, common::Context /* ctx */) const
        {
            auto memory          = std::vector<al::usize>{};
            auto file_id_counter = 0uz;

            for (auto [i, c] : input | sv::enumerate) {
                auto block = static_cast<al::usize>(c - '0');
                if (i % 2 == 0) {
                    memory.insert(memory.end(), block, file_id_counter++);
                } else {
                    memory.insert(memory.end(), block, empty);
                }
            }

            auto left  = 0uz;
            auto right = memory.size() - 1;

            while (left < right) {
                while (memory[left] != empty and left < right) {
                    left++;
                }
                while (memory[right] == empty and left < right) {
                    right--;
                }
                memory[left++] = std::exchange(memory[right--], empty);
            }

            return day9::checksum<empty>(memory);
        }

        Output solve_part_two(Input input, common::Context /* ctx */) const
        {
            auto memory          = std::vector<al::usize>{};
            auto file_id_counter = 0uz;

            for (auto [i, c] : input | sv::enumerate) {
                auto block = static_cast<al::usize>(c - '0');
                if (i % 2 == 0) {
                    memory.insert(memory.end(), block, file_id_counter++);
                } else {
                    memory.insert(memory.end(), block, empty);
                }
            }

            auto file_size = [&](al::usize start, al::usize file_id) {
                auto sub_mem = util::subrange_rev(memory, 0, start);
                auto end     = sr::find_if(sub_mem, [&](auto v) { return v != file_id; });
                return static_cast<al::usize>(end - sub_mem.begin());
            };

            auto next_file = [&](al::usize start, al::usize file_id) {
                auto sub_mem = util::subrange_rev(memory, 0, start);
                auto end     = sr::find(sub_mem, file_id);
                return start - static_cast<al::usize>(end - sub_mem.begin());
            };

            auto find_space = [&](al::usize size, al::usize right) -> std::optional<al::usize> {
                auto it_end = memory.begin() + static_cast<al::isize>(right);
                auto it     = sr::find(memory.begin(), it_end, empty);
                while (it != it_end) {
                    auto end = sr::find_if(it, memory.end(), [&](auto v) { return v != empty; });
                    auto len = static_cast<al::usize>(end - it);
                    if (len >= size) {
                        return static_cast<al::usize>(it - memory.begin());
                    }
                    it = sr::find(end, it_end, empty);
                }
                return std::nullopt;
            };

            auto right   = memory.size();
            auto file_id = file_id_counter;

            while (file_id-- > 0) {
                right     = next_file(right, file_id);
                auto size = file_size(right, file_id);

                if (auto maybe_space = find_space(size, right - 1); maybe_space.has_value()) {
                    auto space = *maybe_space;
                    for (auto i : sv::iota(0uz, size)) {
                        memory.at(space + i) = std::exchange(memory.at(right - i - 1), empty);
                    }
                }
                right -= size;
            }

            return day9::checksum<empty>(memory);
        }
    };

    static_assert(common::Day<Day09>);
}
