#ifndef LIB_BENCHMARK_H_
#define LIB_BENCHMARK_H_

#include <chrono>
#include <functional>
#include <string>

namespace trading {

class Timer {
public:
    Timer();
    void reset();
    double elapsed_ms() const;
    double elapsed_seconds() const;

private:
    std::chrono::high_resolution_clock::time_point start_;
};

struct BenchmarkResult {
    std::string name;
    double elapsed_ms;
    double result_value;
};

BenchmarkResult run_benchmark(const std::string& name,
                              std::function<double()> func);

void print_comparison(const BenchmarkResult& single,
                      const BenchmarkResult& multi);

}  // namespace trading

#endif  // LIB_BENCHMARK_H_
