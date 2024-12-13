#pragma once

#include "helper.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fmt/std.h>

#include <cassert>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

namespace aoc::common
{
    namespace fs = std::filesystem;
    namespace sr = std::ranges;
    namespace sv = std::views;

    using Lines = std::span<std::string_view>;

    template <typename T>
    concept Streamable = requires (std::ostream& os, const T& t) {
        { os << t } -> std::same_as<std::ostream&>;
    };

    template <typename T>
    concept Displayable = fmt::formattable<T> or Streamable<T>;

    template <typename T>
    concept Day = requires {
        requires std::semiregular<T>;

        requires std::is_trivially_move_constructible_v<T>;
        requires std::is_trivially_move_assignable_v<T>;
        requires std::is_trivially_copyable_v<T>;

        typename T::Input;
        typename T::Output;

        requires Displayable<typename T::Output>;

        { T::name } -> std::convertible_to<std::string_view>;

        requires requires (const T ct, T::Input input, Lines lines) {
            { ct.parse(lines) } -> std::same_as<typename T::Input>;
            { ct.solve_part_one(input) } -> std::same_as<typename T::Output>;
            { ct.solve_part_two(input) } -> std::same_as<typename T::Output>;
        };
    };

    template <typename>
    struct DayTraits : std::false_type
    {
    };

    template <Day... T>
    struct DayTraits<std::tuple<T...>> : std::true_type
    {
    };

    template <Day... T>
    struct DayTraits<std::variant<T...>> : std::true_type
    {
    };

    template <typename T>
    concept AreDays = DayTraits<T>::value;

    enum class WriteMode
    {
        Overwrite,
        Append,
    };

    enum class Part
    {
        One = 0b01,
        Two = 0b10,
    };

    struct Timer
    {
        using Clock     = std::chrono::high_resolution_clock;
        using TimePoint = Clock::time_point;
        using Duration  = Clock::duration;

        Timer() noexcept
            : m_start{ Clock::now() }
        {
        }

        Duration elapsed() const noexcept { return Clock::now() - m_start; }

        TimePoint m_start;
    };

    template <Day D>
    D::Input parse_file(const D& d, const fs::path& path) noexcept
    {
        assert(fs::exists(path));

        auto file         = std::ifstream{ path };
        auto file_content = std::string{};

        auto lines_view = std::vector<std::pair<std::size_t, std::size_t>>{};

        auto index = 0uz;
        auto line  = std::string{};

        while (std::getline(file, line)) {
            file_content += line;    // ignore new line it's not necessary anyway
            lines_view.emplace_back(index, line.size());
            index = file_content.size();
        }

        auto to_substr = [&](auto&& pair) {
            auto&& [begin, size] = pair;
            auto str_begin       = file_content.begin() + begin;
            return std::string_view{ str_begin, str_begin + size };
        };
        auto lines = lines_view | sv::transform(to_substr) | sr::to<std::vector>();

        return d.parse(lines);
    }

    template <Day D>
    bool write_output(const typename D::Output& output, const fs::path& path, WriteMode mode) noexcept
    {
        auto flag = std::ios::out;

        switch (mode) {
        case WriteMode::Overwrite: flag |= std::ios::trunc; break;
        case WriteMode::Append: flag |= std::ios::app; break;
        }

        auto file = std::ofstream{ path, flag };
        if (file << output) {
            return true;
        }

        return false;
    }

    template <AreDays Days>
    std::vector<std::string_view> generate_solutions_names()
    {
        auto names = std::vector<std::string_view>{};
        helper::for_each_tuple<Days>([&]<common::Day T>() {
            static_assert(T::name != "all", "Solution name cannot be 'all'");
            names.push_back(T::name);
        });
        return names;
    }

    template <AreDays Days>
    std::optional<helper::ToVariant<Days>> create_solution(std::string_view name)
    {
        auto solution = std::optional<helper::ToVariant<Days>>{};
        helper::for_each_tuple<Days>([&]<common::Day T>() {
            if (T::name == name) {
                solution = T{};
            }
        });
        return solution;
    }

    template <AreDays DaysVar, typename OutFn>
    void run_solution(DaysVar&& solution, const fs::path& infile, Part part, OutFn&& out_fn)
    {
        auto solver = [&]<common::Day T>(T&& t) {
            auto input  = parse_file(t, infile);
            auto output = [&] {
                switch (part) {
                case Part::One: return t.solve_part_one(input); break;
                case Part::Two: return t.solve_part_two(input); break;
                default: [[unlikely]]; std::abort();
                }
            }();
            out_fn(output);
        };
        std::visit(solver, std::forward<DaysVar>(solution));
    }

    template <Displayable T>
    std::string display(T&& t)
    {
        if constexpr (fmt::formattable<T>) {
            return fmt::format("{}", t);
        } else {
            auto ss = std::ostringstream{};
            ss << t;
            return ss.str();
        }
    }
}
