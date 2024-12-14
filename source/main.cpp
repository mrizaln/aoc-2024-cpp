#include "day/all.hpp"

#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <fmt/color.h>

using aoc::common::Day, aoc::common::Part, aoc::common::RunResult, aoc::common::BenchResult;

inline static auto DATA_DIR = std::filesystem::path{ "data" };

template <Day D, std::invocable<const D&, const std::filesystem::path&, Part> Fn>
bool run_impl(const D& day, Fn runner)
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
        runner(day, infile, part);
    }

    return true;
}

template <Day D>
bool run(const D& day)
{
    auto to_ms = aoc::common::to_ms<double>;

    return run_impl(day, [&](const D& day, const std::filesystem::path& infile, Part part) {
        fmt::println("\t> part {}", std::to_underlying(part));

        RunResult result = aoc::common::run_solution(std::move(day), infile, part);

        fmt::println("\t  parse time: {}", to_ms(result.m_parse_time));
        fmt::println("\t  solve time: {}", to_ms(result.m_solve_time));
        fmt::println("\t  total time: {}", to_ms(result.m_parse_time + result.m_solve_time));
        fmt::println("\t  result    : {}\n", result.m_result);
    });
}

template <Day D>
bool bench(const D& day, std::size_t repeat)
{
    auto to_ms = aoc::common::to_ms<double>;

    return run_impl(day, [&](const D& day, const std::filesystem::path& infile, Part part) {
        fmt::println("\t> part {}", std::to_underlying(part));

        BenchResult result = aoc::common::bench_solution(std::move(day), infile, part, repeat);

        fmt::println("\t  parse time: {}", to_ms(result.m_parse_time));
        fmt::println("\t  solve time: {}", to_ms(result.m_solve_time));
        fmt::println("\t  total time: {}\n", to_ms(result.m_parse_time + result.m_solve_time));
    });
}

int main(int argc, char** argv)
{
    auto app = CLI::App{ "AOC C++ solutions" };

    auto selected_solution = std::string{};
    auto should_bench      = false;
    auto bench_repeat      = 10uz;

    auto solutions = aoc::common::generate_solutions_ids<aoc::day::Days>();
    solutions.insert(solutions.begin(), "all");

    app.add_option("solution", selected_solution, "which solution to run")
        ->transform(CLI::IsMember{ solutions })
        ->required(true);
    app.add_flag("-b,--bench", should_bench, "benchmark the solution")->default_val(false);
    app.add_option("-r,--repeat", bench_repeat, "benchmark repeat count")
        ->default_val(10)
        ->transform(CLI::Bound{ 3, 10000 });

    if (argc <= 1) {
        fmt::print("{}", app.help());
        return 0;
    }

    CLI11_PARSE(app, argc, argv);

    auto run_visitor = [&](auto&& d) { return should_bench ? bench(d, bench_repeat) : run(d); };

    if (selected_solution == "all") {
        auto successes     = aoc::helper::for_each_tuple(aoc::day::Days{}, run_visitor);
        auto success_count = 0;
        aoc::helper::for_each_tuple(successes, [&](bool success) { success_count += success; });

        return static_cast<int>(std::tuple_size_v<aoc::day::Days>) - success_count;
    } else {
        auto variant = aoc::common::create_solution<aoc::day::Days>(selected_solution).value();
        auto success = std::visit(run_visitor, variant);

        return success;
    }
}
