#include "day/all.hpp"

#include <CLI/CLI.hpp>
#include <fmt/core.h>

using Solutions        = aoc::day::Days;
using SolutionsVariant = aoc::helper::ToVariant<Solutions>;

static_assert(std::tuple_size_v<Solutions> != 0, "No solutions provided");
static_assert(aoc::common::AreDays<Solutions>, "Not all types in Solutions conform to the Day concept");

inline static auto data_dir = std::filesystem::path{ "data" };

bool run_one(SolutionsVariant&& solution)
{
    auto name   = std::visit([](auto&& t) { return t.name; }, solution);
    auto infile = data_dir / "inputs" / name;

    infile.replace_extension(".txt");

    if (not std::filesystem::exists(infile)) {
        fmt::println("{:<24.24} │ {}: {}", name, "input file not found", infile);
        return false;
    }

    using P = aoc::common::Part;
    for (auto part : { P::One, P::Two }) {
        aoc::common::run_solution(std::move(solution), infile, part, [&](auto&& output) {
            auto str = aoc::common::display(output);
            fmt::print("{:<24.24} │ part {} │ {}\n", name, std::to_underlying(part), str);
        });
    }

    return true;
}

int main(int argc, char** argv)
{
    auto app               = CLI::App{ "AOC C++ template/runner" };
    auto selected_solution = std::string{};

    auto solutions = aoc::common::generate_solutions_names<Solutions>();
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
        for (auto& solution : solutions | aoc::common::sv::drop(1)) {
            auto solution_variant = aoc::common::create_solution<Solutions>(solution).value();
            auto success          = run_one(std::move(solution_variant));

            if (not success) {
                return 1;
            }
        }

        return 0;
    }

    auto solution = aoc::common::create_solution<Solutions>(selected_solution).value();
    auto success  = run_one(std::move(solution));

    if (not success) {
        return 1;
    }

    return 0;
}
