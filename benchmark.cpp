#include <algorithm>
#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "src/grid.h"
#include "src/sudoku_generator.h"
#include "src/sudoku_solver.h"
#include "src/sudoku_validator.h"

static constexpr int PUZZLES_PER_DIFFICULTY = 1000;

struct BenchResult {
    std::string solver_name;
    double avg_time_ms    = 0.0;
    double max_time_ms    = 0.0;
    double avg_backtracks = 0.0;
    int    max_backtracks = 0;
    int    pure_logic     = 0;   
    int    total          = 0;
};

enum class SolverType { BASELINE, PROPAGATION, AC3 };

BenchResult run_benchmark(const std::vector<sudoku::Grid>& puzzles,
                          SolverType solver,
                          const std::string& name)
{
    BenchResult res;
    res.solver_name = name;
    res.total       = static_cast<int>(puzzles.size());

    double total_time = 0.0;
    long   total_bt   = 0;

    for (const sudoku::Grid& puzzle : puzzles) {
        sudoku::Grid g = puzzle;   // copy — solve mutates
        sudoku::SolveStats stats;

        try {
            switch (solver) {
                case SolverType::BASELINE:    stats = sudoku::solve_baseline(g);        break;
                case SolverType::PROPAGATION: stats = sudoku::solve_with_stats(g);      break;
                case SolverType::AC3:         stats = sudoku::solve_ac3_with_stats(g);  break;
            }
        } catch (...) {
            res.total--;
            continue;
        }

        total_time         += stats.time_ms;
        total_bt           += stats.backtracks;
        res.max_time_ms     = std::max(res.max_time_ms,   stats.time_ms);
        res.max_backtracks  = std::max(res.max_backtracks, stats.backtracks);
        if (stats.backtracks == 0) res.pure_logic++;
    }

    if (res.total > 0) {
        res.avg_time_ms    = total_time / res.total;
        res.avg_backtracks = static_cast<double>(total_bt) / res.total;
    }
    return res;
}

void sep(char c = '-', int w = 88) { std::cout << std::string(w, c) << "\n"; }

void print_table(const std::string& diff_name,
                 const BenchResult& baseline,
                 const BenchResult& prop,
                 const BenchResult& ac3)
{
    const int W0=26, W1=10, W2=10, W3=11, W4=10, W5=12;

    sep('=', 88);
    std::cout << "  " << diff_name << "  (" << baseline.total << " puzzles)\n";
    sep('-', 88);

    // Header
    std::cout
        << std::left  << std::setw(W0) << "  Solver"
        << std::right << std::setw(W1) << "Avg ms"
        << std::right << std::setw(W2) << "Max ms"
        << std::right << std::setw(W3) << "Avg BT"
        << std::right << std::setw(W4) << "Max BT"
        << std::right << std::setw(W5) << "Pure logic"
        << "\n";
    sep('-', 88);

    auto row = [&](const BenchResult& r) {
        double pct = r.total > 0 ? 100.0*r.pure_logic/r.total : 0.0;
        std::cout
            << std::left  << std::setw(W0) << ("  " + r.solver_name)
            << std::right << std::fixed
            << std::setprecision(3) << std::setw(W1) << r.avg_time_ms
            << std::setw(W2) << r.max_time_ms
            << std::setprecision(1) << std::setw(W3) << r.avg_backtracks
            << std::setprecision(0) << std::setw(W4) << r.max_backtracks
            << std::setw(W5) << (std::to_string((int)std::round(pct)) + "%")
            << "\n";
    };

    row(baseline);
    row(prop);
    row(ac3);
    sep('-', 88);

    // Speedup lines
    auto speedup = [](double base, double improved) -> double {
        return improved > 0.0 ? base / improved : 0.0;
    };
    auto bt_drop = [](double base, double improved) -> double {
        return base > 0.0 ? (1.0 - improved/base)*100.0 : 100.0;
    };

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  Propagation vs Baseline:  "
              << speedup(baseline.avg_time_ms, prop.avg_time_ms)   << "x faster"
              << ",  " << bt_drop(baseline.avg_backtracks, prop.avg_backtracks)
              << "% fewer backtracks\n";
    std::cout << "  AC-3 vs Baseline:         "
              << speedup(baseline.avg_time_ms, ac3.avg_time_ms)    << "x faster"
              << ",  " << bt_drop(baseline.avg_backtracks, ac3.avg_backtracks)
              << "% fewer backtracks\n";
    std::cout << "  AC-3 vs Propagation:      "
              << speedup(prop.avg_time_ms, ac3.avg_time_ms)        << "x faster"
              << ",  " << bt_drop(prop.avg_backtracks, ac3.avg_backtracks)
              << "% fewer backtracks\n";
    std::cout << "\n";
}

int main()
{
    sep('=', 88);
    std::cout << "  Sudoku Engine — Solver Benchmark\n";
    std::cout << "  " << PUZZLES_PER_DIFFICULTY << " puzzles per difficulty, 3 solvers\n";
    std::cout << "  Solver 1: Baseline     — pure MRV backtracking (original)\n";
    std::cout << "  Solver 2: Propagation  — naked+hidden singles + MRV\n";
    std::cout << "  Solver 3: AC-3         — arc consistency + MRV\n";
    sep('=', 88);
    std::cout << "\n";

    struct Entry { sudoku::Difficulty diff; std::string name; };
    const std::array<Entry,3> diffs = {{
        { sudoku::Difficulty::EASY,   "EASY   (35 cells removed, ~46 clues)" },
        { sudoku::Difficulty::MEDIUM, "MEDIUM (45 cells removed, ~36 clues)" },
        { sudoku::Difficulty::HARD,   "HARD   (55 cells removed, ~26 clues)" },
    }};

    for (const auto& entry : diffs) {
        std::cout << "  Generating " << PUZZLES_PER_DIFFICULTY
                  << " " << entry.name << " puzzles...\n";
        std::cout.flush();

        std::vector<sudoku::Grid> puzzles;
        puzzles.reserve(PUZZLES_PER_DIFFICULTY);
        for (int i=0; i<PUZZLES_PER_DIFFICULTY; i++)
            puzzles.push_back(sudoku::generate_puzzle(entry.diff));

        std::cout << "  Running baseline...\n";     std::cout.flush();
        auto r1 = run_benchmark(puzzles, SolverType::BASELINE,    "Baseline (BT only)");

        std::cout << "  Running propagation...\n";  std::cout.flush();
        auto r2 = run_benchmark(puzzles, SolverType::PROPAGATION, "Propagation (N+H+BT)");

        std::cout << "  Running AC-3...\n";         std::cout.flush();
        auto r3 = run_benchmark(puzzles, SolverType::AC3,         "AC-3 (AC3+BT)");

        print_table(entry.name, r1, r2, r3);
    }

    sep('=', 88);
    std::cout << "  KEY\n";
    std::cout << "  Avg BT     = average backtrack nodes per puzzle\n";
    std::cout << "  Max BT     = worst-case puzzle (most backtracks)\n";
    std::cout << "  Pure logic = % of puzzles solved with ZERO backtracking\n\n";
    sep('=', 88);
    return 0;
}