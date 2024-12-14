#include "day/all.hpp"

#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <fmt/color.h>

using aoc::common::Day, aoc::common::RunResult, aoc::common::Part, aoc::common::Timer;

inline static auto DATA_DIR = std::filesystem::path{ "data" };

template <Day D>
bool run_one(D&& day)
{
    auto infile = DATA_DIR / "inputs" / D::id;
    infile.replace_extension(".txt");

    fmt::println(">>> [{}] {:<24.24}", D::id, D::name);
    if (not std::filesystem::exists(infile)) {
        fmt::println(
            "\t {}: {} - {}",    //
            fmt::styled("FAILED", fmt::fg(fmt::color::red)),
            "input file not found",
            infile
        );
        return false;
    }

    for (auto part : { Part::One, Part::Two }) {
        RunResult result = aoc::common::run_solution(std::move(day), infile, part);

        auto to_ms = [](Timer::Duration dur) {
            using Ms = std::chrono::duration<double, std::milli>;
            return std::chrono::duration_cast<Ms>(dur);
        };

        fmt::println("\t> part {}", std::to_underlying(part));
        fmt::println("\t  parse time: {}", to_ms(result.m_parse_time));
        fmt::println("\t  solve time: {}", to_ms(result.m_solve_time));
        fmt::println("\t  total time: {}", to_ms(result.m_parse_time + result.m_solve_time));
        fmt::println("\t  result    : {}\n", result.m_result);
    }

    return true;
}

int main(int argc, char** argv)
{
    auto app               = CLI::App{ "AOC C++ solutions" };
    auto selected_solution = std::string{};

    auto solutions = aoc::common::generate_solutions_ids<aoc::day::Days>();
    solutions.insert(solutions.begin(), "all");

    app.add_option("solution", selected_solution, "which solution to run")
        ->transform(CLI::IsMember{ solutions })
        ->required(true);

    if (argc <= 1) {
        fmt::print("{}", app.help());
        return 0;
    }

    CLI11_PARSE(app, argc, argv);

    if (selected_solution == "all") {
        using aoc::helper::for_each_tuple;

        auto success_count = 0;
        for_each_tuple<aoc::day::Days>([&]<Day D>() { success_count += run_one(D{}); });

        return success_count;
    }

    auto variant = aoc::common::create_solution<aoc::day::Days>(selected_solution).value();
    auto success = std::visit([](auto&& d) { return run_one(std::move(d)); }, variant);

    return success;
}
