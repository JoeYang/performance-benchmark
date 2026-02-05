# Trading System CPU Benchmark Results

**Date:** 2026-02-05
**Platform:** macOS Darwin 24.2.0
**CPU Threads:** 10

## Configuration

| Parameter | Value |
|-----------|-------|
| Positions | 10,000 |
| Monte Carlo Simulations | 100,000 |
| Time Horizon | 1 day (1/252 years) |
| Random Seed | 42 |

## Performance Results

### Monte Carlo VaR

| Mode | Time | Speedup |
|------|------|---------|
| Single-threaded | 113,618.8 ms | - |
| Multi-threaded | 14,883.1 ms | **7.6x** |

### Greeks Calculation (Black-Scholes)

| Mode | Time | Speedup |
|------|------|---------|
| Single-threaded | 1.8 ms | - |
| Multi-threaded | 0.6 ms | **2.9x** |

### Position Aggregation

| Mode | Time | Speedup |
|------|------|---------|
| Single-threaded | 1.8 ms | - |
| Multi-threaded | 1.8 ms | 1.0x |

### Total Benchmark Time

| Mode | Time | Speedup |
|------|------|---------|
| Single-threaded | 113,622.4 ms (~1.9 min) | - |
| Multi-threaded | 14,885.4 ms (~15 sec) | **7.6x** |

## Risk Metrics Output

| Metric | Value |
|--------|-------|
| VaR (99%) | $1,233,844.05 |
| VaR (95%) | $873,598.39 |
| Expected Shortfall | $1,408,734.57 |
| Portfolio Delta | -13,846.06 |
| Net Exposure | $16,221,654.84 |
| Unique Symbols | 500 |

## Observations

1. **Monte Carlo VaR** is the dominant CPU workload, accounting for >99% of total runtime
2. **Multi-threading scales well** with 7.6x speedup on 10 threads (76% efficiency)
3. **Greeks calculation** shows good parallelization (2.9x on light workload)
4. **Position aggregation** shows no speedup due to hash map merge overhead exceeding computation cost

## Commands

```bash
# Run with default settings
bazel run //apps:risk_benchmark

# Custom parameters
bazel run //apps:risk_benchmark -- --positions 5000 --simulations 50000 --threads 4

# Run all tests
./scripts/run_tests.sh
```
