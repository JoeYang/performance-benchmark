#include "lib/benchmark.h"

#include <iomanip>
#include <iostream>

namespace trading {

Timer::Timer() : start_(std::chrono::high_resolution_clock::now()) {}

void Timer::reset() {
    start_ = std::chrono::high_resolution_clock::now();
}

double Timer::elapsed_ms() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(now - start_).count();
}

double Timer::elapsed_seconds() const {
    return elapsed_ms() / 1000.0;
}

BenchmarkResult run_benchmark(const std::string& name,
                              std::function<double()> func) {
    Timer timer;
    double result = func();
    double elapsed = timer.elapsed_ms();
    return {name, elapsed, result};
}

void print_comparison(const BenchmarkResult& single,
                      const BenchmarkResult& multi) {
    double speedup = single.elapsed_ms / multi.elapsed_ms;
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  Single-threaded: " << std::setw(8) << single.elapsed_ms
              << " ms\n";
    std::cout << "  Multi-threaded:  " << std::setw(8) << multi.elapsed_ms
              << " ms (" << speedup << "x speedup)\n";
}

}  // namespace trading
