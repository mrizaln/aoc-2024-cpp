#include "day/all.hpp"

#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <fmt/color.h>

using aoc::common::Day, aoc::common::Part, aoc::common::RunResult, aoc::common::BenchResult;

inline static auto DATA_DIR = std::filesystem::path{ "data" };

template <Day D, std::invocable<const D&, const std::filesystem::path&, Part> Fn>
bool run_impl(const D& day, const std::filesystem::path infile, Fn runner)
{
    fmt::println(">>> [{}] {:<24.24}", D::id, D::name);
    if (not std::filesystem::exists(infile)) {
        fmt::println(
            "\t{}: {} - {}\n",    //
            fmt::styled("FAILED", fmt::fg(fmt::color::red)),
            "input file not found",
            infile
        );
        return false;
    }

    for (auto part : { Part::One, Part::Two }) {
        try {
            runner(day, infile, part);
        } catch (std::exception& e) {
            fmt::println(
                "\t{}: exception thrown - {}\n",    //
                fmt::styled("FAILED", fmt::fg(fmt::color::red)),
                e.what()
            );
        }
    }

    return true;
}

template <Day D>
bool run(const D& day)
{
    auto to_ms  = aoc::common::to_ms<double>;
    auto infile = DATA_DIR / "inputs" / D::id;
    infile.replace_extension(".txt");

    return run_impl(day, infile, [&](const D& day, const std::filesystem::path& infile, Part part) {
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
    auto to_ms  = aoc::common::to_ms<double>;
    auto infile = DATA_DIR / "inputs" / D::id;
    infile.replace_extension(".txt");

    return run_impl(day, infile, [&](const D& day, const std::filesystem::path& infile, Part part) {
        fmt::println("\t> part {}", std::to_underlying(part));

        BenchResult result = aoc::common::bench_solution(std::move(day), infile, part, repeat);

        fmt::println("\t  parse time: {}", to_ms(result.m_parse_time));
        fmt::println("\t  solve time: {}", to_ms(result.m_solve_time));
        fmt::println("\t  total time: {}\n", to_ms(result.m_parse_time + result.m_solve_time));
    });
}

template <Day D>
bool test(const D& day)
{
    auto to_ms  = aoc::common::to_ms<double>;
    auto infile = DATA_DIR / "examples" / D::id;
    infile.replace_extension(".txt");

    return run_impl(day, infile, [&](const D& day, const std::filesystem::path& infile, Part part) {
        fmt::println("\t> part {}", std::to_underlying(part));

        RunResult result = aoc::common::run_solution(std::move(day), infile, part);

        fmt::println("\t  parse time: {}", to_ms(result.m_parse_time));
        fmt::println("\t  solve time: {}", to_ms(result.m_solve_time));
        fmt::println("\t  total time: {}", to_ms(result.m_parse_time + result.m_solve_time));
        fmt::println("\t  result    : {}\n", result.m_result);
    });
}

int main(int argc, char** argv)
{
    auto app = CLI::App{ "AOC C++ solutions" };

    auto selected_day = std::string{};
    auto bench_repeat = 0uz;
    auto should_test  = false;

    auto solutions = aoc::common::generate_solutions_ids<aoc::day::Days>();
    solutions.insert(solutions.begin(), "all");

    app.add_option("day", selected_day, "which solution to run")
        ->transform(CLI::IsMember{ solutions })
        ->required(true);
    app.add_option("-b,--bench", bench_repeat, "benchmark the solution by running specified number of times")
        ->transform(CLI::Bound{ 3, 10000 });
    app.add_flag("-t,--test", should_test, "test the solution by using example data");

    if (argc <= 1) {
        fmt::print("{}", app.help());
        return 0;
    }

    CLI11_PARSE(app, argc, argv);

    if (should_test == true and bench_repeat != 0) {
        fmt::println("--test and --bench flags are mutually exclusive");
        return 1;
    }

    // clang-format off
    auto run_visitor = [&](auto&& d) {
        if      (should_test)         return test(d);
        else if (bench_repeat != 0uz) return bench(d, bench_repeat);
        else                          return run(d);
    };
    // clang-format on

    if (selected_day == "all") {
        auto successes     = aoc::helper::for_each_tuple(aoc::day::Days{}, run_visitor);
        auto success_count = 0;
        aoc::helper::for_each_tuple(successes, [&](bool success) { success_count += success; });

        return static_cast<int>(std::tuple_size_v<aoc::day::Days>) - success_count;
    } else {
        auto variant = aoc::common::create_solution<aoc::day::Days>(selected_day).value();
        auto success = std::visit(run_visitor, variant);

        return success ? EXIT_SUCCESS : EXIT_FAILURE;
    }
}
