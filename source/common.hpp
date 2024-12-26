#pragma once

#include "aliases.hpp"
#include "concepts.hpp"
#include "meta.hpp"

#include <fmt/base.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <libassert/assert.hpp>

namespace aoc::common
{
    namespace fs = std::filesystem;
    namespace sr = std::ranges;
    namespace sv = std::views;

    using aliases::Context;
    using aliases::Lines;

    using concepts::AreDays;
    using concepts::Day;
    using concepts::Displayable;
    using concepts::Streamable;

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
        void     reset() noexcept { m_start = Clock::now(); }

        TimePoint m_start;
    };

    struct RawInput
    {
        std::string                   m_string;
        std::vector<std::string_view> m_lines;
    };

    template <Day D>
    struct RunResult
    {
        D::Output       m_result;
        Timer::Duration m_parse_time;
        Timer::Duration m_solve_time;
    };

    struct BenchResult
    {
        Timer::Duration m_parse_time;
        Timer::Duration m_solve_time;
    };

    inline RawInput parse_file(const fs::path& path) noexcept
    {
        ASSERT(fs::exists(path), fmt::format("path '{}' must exist when calling this function", path));

        // allow raw input (and the lines span) be moved outside without invalidating std::string_view to it
        auto raw_input = RawInput{};

        auto  file         = std::ifstream{ path };
        auto& file_content = raw_input.m_string;

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
            auto str_begin       = file_content.begin() + static_cast<std::ptrdiff_t>(begin);
            return std::string_view{ str_begin, str_begin + static_cast<std::ptrdiff_t>(size) };
        };

        raw_input.m_lines = lines_view | sv::transform(to_substr) | sr::to<std::vector>();
        return raw_input;
    }

    template <AreDays Days>
    std::vector<std::string_view> generate_solutions_ids()
    {
        auto ids = std::vector<std::string_view>{};
        meta::for_each_tuple<Days>([&]<common::Day T>() {
            static_assert(T::id != "all", "Solution id cannot be 'all'");
            ids.push_back(T::id);
        });
        return ids;
    }

    template <AreDays Days>
    std::optional<meta::ToVariant<Days>> create_solution(std::string_view id)
    {
        auto solution = std::optional<meta::ToVariant<Days>>{};
        meta::for_each_tuple<Days>([&]<common::Day T>() {
            if (T::id == id) {
                solution = T{};
            }
        });
        return solution;
    }

    template <Day D>
    RunResult<D> run_solution(const D& day, const fs::path& infile, Part part)
    {
        auto timer                 = Timer{};
        auto [_raw_str, raw_lines] = parse_file(infile);

        auto context = Context{
#if defined(NDEBUG)
            .m_debug = false,
#else
            .m_debug = true,
#endif
            .m_benchmark = false,
        };

        timer.reset();
        auto input      = day.parse(raw_lines, context);
        auto parse_time = timer.elapsed();

        auto solve = [&] {
            switch (part) {
            case Part::One: return day.solve_part_one(std::move(input), context); break;
            case Part::Two: return day.solve_part_two(std::move(input), context); break;
            default: [[unlikely]]; std::unreachable();
            }
        };

        timer.reset();
        auto output     = solve();
        auto solve_time = timer.elapsed();

        return {
            .m_result     = std::move(output),
            .m_parse_time = parse_time,
            .m_solve_time = solve_time,
        };
    }

    template <Day D>
    BenchResult bench_solution(const D& day, const fs::path& infile, Part part, std::size_t repeat)
    {
        if (repeat < 3) {
            throw std::logic_error{ "repeating less than 3 is not very useful for benchmarking..." };
        }

        auto timer                 = Timer{};
        auto [_raw_str, raw_lines] = parse_file(infile);

        auto context = Context{
#if defined(NDEBUG)
            .m_debug = false,
#else
            .m_debug = true,
#endif
            .m_benchmark = true,
        };

        auto bench_parse = [&] {
            timer.reset();
            auto _ = day.parse(raw_lines, context);
            return timer.elapsed();
        };

        auto bench_solve = [&](const D::Input& input) {
            timer.reset();
            switch (part) {
            case Part::One: day.solve_part_one(input, context); break;    // copy input
            case Part::Two: day.solve_part_two(input, context); break;    // copy input
            default: [[unlikely]]; std::unreachable();
            }
            return timer.elapsed();
        };

        constexpr auto warmup = 3uz;

        auto parse_time = Timer::Duration{};
        for (auto _ : sv::iota(0uz, warmup)) {
            std::ignore = bench_parse();
        }
        for (auto _ : sv::iota(0uz, repeat)) {
            parse_time += bench_parse();
        }

        auto input      = day.parse(raw_lines, context);
        auto solve_time = Timer::Duration{};
        for (auto _ : sv::iota(0uz, warmup)) {
            std::ignore = bench_solve(input);
        }
        for (auto _ : sv::iota(0uz, repeat)) {
            solve_time += bench_solve(input);
        }

        return { .m_parse_time = parse_time / repeat, .m_solve_time = solve_time / repeat };
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

    template <std::floating_point ToRep>
    auto to_ms(Timer::Duration dur)
    {
        using Ms = std::chrono::duration<ToRep, std::milli>;
        return std::chrono::duration_cast<Ms>(dur);
    };
}
