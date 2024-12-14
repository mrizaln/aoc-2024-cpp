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

        { T::id } -> std::convertible_to<std::string_view>;
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
    struct ParseResult
    {
        std::string                   m_raw_input;
        std::vector<std::string_view> m_raw_input_lines;
        D::Input                      m_parsed;
        Timer::Duration               m_time;
    };

    template <Day D>
    struct RunResult
    {
        D::Output       m_result;
        Timer::Duration m_parse_time;
        Timer::Duration m_solve_time;
    };

    template <Day D>
    ParseResult<D> parse_file(const D& d, const fs::path& path) noexcept
    {
        assert(fs::exists(path));

        // allow raw input (and the lines span) be moved outside without invalidating std::string_view to it
        auto result = ParseResult<D>{};

        auto  file         = std::ifstream{ path };
        auto& file_content = result.m_raw_input;

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

        result.m_raw_input_lines = lines_view | sv::transform(to_substr) | sr::to<std::vector>();

        auto timer = Timer{};

        result.m_parsed = d.parse(result.m_raw_input_lines);
        result.m_time   = timer.elapsed();

        return result;
    }

    template <AreDays Days>
    std::vector<std::string_view> generate_solutions_ids()
    {
        auto ids = std::vector<std::string_view>{};
        helper::for_each_tuple<Days>([&]<common::Day T>() {
            static_assert(T::id != "all", "Solution id cannot be 'all'");
            ids.push_back(T::id);
        });
        return ids;
    }

    template <AreDays Days>
    std::optional<helper::ToVariant<Days>> create_solution(std::string_view id)
    {
        auto solution = std::optional<helper::ToVariant<Days>>{};
        helper::for_each_tuple<Days>([&]<common::Day T>() {
            if (T::id == id) {
                solution = T{};
            }
        });
        return solution;
    }

    template <Day D>
    RunResult<D> run_solution(D&& day, const fs::path& infile, Part part)
    {
        auto [_raw, _raw_lines, input, parse_timing] = parse_file(day, infile);

        auto timer  = Timer{};
        auto output = [&] {
            switch (part) {
            case Part::One: return day.solve_part_one(std::move(input)); break;
            case Part::Two: return day.solve_part_two(std::move(input)); break;
            default: [[unlikely]]; std::abort();
            }
        }();

        return {
            .m_result     = std::move(output),
            .m_parse_time = parse_timing,
            .m_solve_time = timer.elapsed(),
        };
    }

    template <Day... Ds>
    std::tuple<RunResult<Ds>...> run_solution_multi(
        const std::tuple<Ds...>&                 days,
        std::span<const fs::path, sizeof...(Ds)> infiles,
        Part                                     part
    )
    {
        auto result = std::tuple<RunResult<Ds>...>{};

        auto solver = [&]<std::size_t I>() {
            std::get<I>(result) = run_solution(std::get<I>(days), infiles[I], part);
        };

        helper::for_each_tuple<std::tuple<Ds...>>([&]<common::Day T>() {
            auto solution = T{};
            auto do_run   = [&]<std::size_t... I>(std::index_sequence<I...>) {
                (solver.template operator()<I>(), ...);
            };
        });

        return result;
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
