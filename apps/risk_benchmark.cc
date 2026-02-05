#include <iostream>
#include <iomanip>
#include <string>
#include <thread>

#include "lib/aggregator.h"
#include "lib/benchmark.h"
#include "lib/greeks.h"
#include "lib/monte_carlo.h"
#include "lib/position.h"
#include "lib/system.h"

void print_usage() {
    std::cout << "Usage: risk_benchmark [options]\n"
              << "\nBenchmark Options:\n"
              << "  --positions N       Number of positions (default: 10000)\n"
              << "  --simulations N     Number of MC simulations (default: 100000)\n"
              << "  --threads N         Number of threads (default: auto-detect)\n"
              << "\nSystem Tuning Options:\n"
              << "  --cpus LIST         Pin to specific CPUs (e.g., 0,1,2 or 0-3 or 0,2-4)\n"
              << "  --numa-node N       Bind to NUMA node N\n"
              << "  --lock-memory       Lock memory pages (prevents swapping)\n"
              << "  --realtime          Use realtime scheduling priority\n"
              << "  --prefault          Pre-fault memory pages\n"
              << "  --preallocate N     Pre-allocate N MB of memory\n"
              << "  --isolate           Apply all isolation options (lock, prefault)\n"
              << "\nInformation:\n"
              << "  --sysinfo           Print system information and exit\n"
              << "  --help              Show this help message\n"
              << "\nExamples:\n"
              << "  # Basic run\n"
              << "  risk_benchmark --positions 5000 --simulations 50000\n"
              << "\n"
              << "  # Pin to CPUs 0-3 on NUMA node 0\n"
              << "  risk_benchmark --numa-node 0 --cpus 0-3\n"
              << "\n"
              << "  # Full isolation for low-latency testing\n"
              << "  sudo risk_benchmark --isolate --realtime --cpus 2-5\n"
              << "\n"
              << "  # With Solarflare Onload (run externally)\n"
              << "  onload risk_benchmark --isolate\n";
}

void print_header(int num_positions, int num_simulations, int num_threads) {
    std::cout << "\n";
    std::cout << "=== Trading System CPU Benchmark ===\n";
    std::cout << "Positions: " << num_positions
              << " | Simulations: " << num_simulations
              << " | Threads: " << num_threads << "\n";
    std::cout << std::string(50, '-') << "\n\n";
}

void print_section(const std::string& name) {
    std::cout << name << ":\n";
}

int main(int argc, char* argv[]) {
    int num_positions = 10000;
    int num_simulations = 100000;
    int num_threads = std::thread::hardware_concurrency();
    trading::SystemConfig sys_config;
    bool show_sysinfo = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // Benchmark options
        if (arg == "--positions" && i + 1 < argc) {
            num_positions = std::stoi(argv[++i]);
        } else if (arg == "--simulations" && i + 1 < argc) {
            num_simulations = std::stoi(argv[++i]);
        } else if (arg == "--threads" && i + 1 < argc) {
            num_threads = std::stoi(argv[++i]);
        }
        // System tuning options
        else if (arg == "--cpus" && i + 1 < argc) {
            sys_config.cpu_affinity = trading::parse_cpu_list(argv[++i]);
        } else if (arg == "--numa-node" && i + 1 < argc) {
            sys_config.numa_node = std::stoi(argv[++i]);
        } else if (arg == "--lock-memory") {
            sys_config.lock_memory = true;
        } else if (arg == "--realtime") {
            sys_config.realtime_priority = true;
        } else if (arg == "--prefault") {
            sys_config.prefault_memory = true;
        } else if (arg == "--preallocate" && i + 1 < argc) {
            sys_config.preallocate_mb = std::stoul(argv[++i]);
        } else if (arg == "--isolate") {
            sys_config.lock_memory = true;
            sys_config.prefault_memory = true;
            sys_config.preallocate_mb = 256;
        }
        // Info options
        else if (arg == "--sysinfo") {
            show_sysinfo = true;
        } else if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            std::cerr << "Use --help for usage information.\n";
            return 1;
        }
    }

    // Show system info
    auto sys_info = trading::get_system_info();

    if (show_sysinfo) {
        trading::print_system_info(sys_info);
        return 0;
    }

    // Apply system configuration
    bool has_sys_config = !sys_config.cpu_affinity.empty() ||
                          sys_config.numa_node >= 0 ||
                          sys_config.lock_memory ||
                          sys_config.realtime_priority ||
                          sys_config.prefault_memory ||
                          sys_config.preallocate_mb > 0;

    if (has_sys_config) {
        std::cout << "Applying system configuration...\n";
        trading::apply_system_config(sys_config);
        trading::print_applied_config(sys_config);
        std::cout << "\n";
    }

    print_header(num_positions, num_simulations, num_threads);

    std::cout << "Generating " << num_positions << " random positions...\n";
    trading::Timer gen_timer;
    auto positions = trading::generate_random_positions(num_positions, 42);
    std::cout << "Generated in " << std::fixed << std::setprecision(1)
              << gen_timer.elapsed_ms() << " ms\n\n";

    // Monte Carlo VaR
    print_section("Monte Carlo VaR");
    trading::VaRResult var_result;

    auto mc_single = trading::run_benchmark("MC Single", [&]() {
        var_result = trading::run_monte_carlo_single(
            positions, num_simulations, 1.0/252.0, 42);
        return var_result.var_99;
    });

    auto mc_multi = trading::run_benchmark("MC Multi", [&]() {
        var_result = trading::run_monte_carlo_multi(
            positions, num_simulations, 1.0/252.0, num_threads, 42);
        return var_result.var_99;
    });

    trading::print_comparison(mc_single, mc_multi);
    std::cout << "\n";

    // Greeks Calculation
    print_section("Greeks Calculation");
    double total_delta = 0.0;

    auto greeks_single = trading::run_benchmark("Greeks Single", [&]() {
        auto greeks = trading::calculate_all_greeks_single(positions);
        total_delta = trading::total_portfolio_delta(greeks, positions);
        return total_delta;
    });

    auto greeks_multi = trading::run_benchmark("Greeks Multi", [&]() {
        auto greeks = trading::calculate_all_greeks_multi(positions, num_threads);
        total_delta = trading::total_portfolio_delta(greeks, positions);
        return total_delta;
    });

    trading::print_comparison(greeks_single, greeks_multi);
    std::cout << "\n";

    // Position Aggregation
    print_section("Position Aggregation");
    trading::AggregationResult agg_result;

    auto agg_single = trading::run_benchmark("Agg Single", [&]() {
        agg_result = trading::aggregate_positions_single(positions);
        return agg_result.net_exposure;
    });

    auto agg_multi = trading::run_benchmark("Agg Multi", [&]() {
        agg_result = trading::aggregate_positions_multi(positions, num_threads);
        return agg_result.net_exposure;
    });

    trading::print_comparison(agg_single, agg_multi);
    std::cout << "\n";

    // Summary
    std::cout << std::string(50, '-') << "\n";
    std::cout << "Results Summary:\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  VaR (99%):        $" << std::setw(12) << var_result.var_99 << "\n";
    std::cout << "  VaR (95%):        $" << std::setw(12) << var_result.var_95 << "\n";
    std::cout << "  Expected Shortfall: $" << std::setw(10) << var_result.expected_shortfall << "\n";
    std::cout << "  Portfolio Delta:  " << std::setw(14) << total_delta << "\n";
    std::cout << "  Net Exposure:     $" << std::setw(12) << agg_result.net_exposure << "\n";
    std::cout << "  Unique Symbols:   " << std::setw(14) << agg_result.by_symbol.size() << "\n";

    // Total timing
    double total_single = mc_single.elapsed_ms + greeks_single.elapsed_ms + agg_single.elapsed_ms;
    double total_multi = mc_multi.elapsed_ms + greeks_multi.elapsed_ms + agg_multi.elapsed_ms;
    double overall_speedup = total_single / total_multi;

    std::cout << "\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << "Total Benchmark Time:\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  Single-threaded: " << std::setw(8) << total_single << " ms\n";
    std::cout << "  Multi-threaded:  " << std::setw(8) << total_multi << " ms ("
              << overall_speedup << "x speedup)\n";

    return 0;
}
