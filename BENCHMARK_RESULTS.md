# Trading System CPU Benchmark Results

**Date:** 2026-02-05
**Platform:** Linux 6.14.0-37-generic
**CPU:** Intel Core Ultra 7 255HX
**CPU Threads:** 20
**Memory:** 64 GB

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
| Single-threaded | 137,181.1 ms | - |
| Multi-threaded | 10,983.1 ms | **12.5x** |

### Greeks Calculation (Black-Scholes)

| Mode | Time | Speedup |
|------|------|---------|
| Single-threaded | 1.8 ms | - |
| Multi-threaded | 0.6 ms | **3.0x** |

### Position Aggregation

| Mode | Time | Speedup |
|------|------|---------|
| Single-threaded | 0.7 ms | - |
| Multi-threaded | 1.1 ms | 0.6x |

### Total Benchmark Time

| Mode | Time | Speedup |
|------|------|---------|
| Single-threaded | 137,183.5 ms (~2.3 min) | - |
| Multi-threaded | 10,984.8 ms (~11 sec) | **12.5x** |

## Risk Metrics Output

| Metric | Value |
|--------|-------|
| VaR (99%) | $1,198,635.09 |
| VaR (95%) | $850,932.78 |
| Expected Shortfall | $1,372,763.86 |
| Portfolio Delta | 17,006.27 |
| Net Exposure | $12,638,955.45 |
| Unique Symbols | 500 |

## Observations

1. **Monte Carlo VaR** is the dominant CPU workload, accounting for >99% of total runtime
2. **Multi-threading scales well** with 12.5x speedup on 20 threads (62.5% efficiency)
3. **Greeks calculation** shows good parallelization (3.0x on light workload)
4. **Position aggregation** shows negative speedup due to hash map merge overhead exceeding computation cost

## Comparison with Apple M1 (10 threads)

| Benchmark | M1 Multi-threaded | Linux Multi-threaded | Difference |
|-----------|-------------------|---------------------|------------|
| Monte Carlo VaR | 14,883.1 ms | 10,983.1 ms | **36% faster** |
| Greeks Calculation | 0.6 ms | 0.6 ms | Same |
| Position Aggregation | 1.8 ms | 1.1 ms | **39% faster** |
| Total Time | 14,885.4 ms | 10,984.8 ms | **36% faster** |

Note: M1 achieves better per-thread efficiency (7.6x on 10 threads = 76%) compared to Linux (12.5x on 20 threads = 62.5%), but Linux wins on absolute throughput due to more cores.

## Commands

```bash
# Run with default settings
bazel run //apps:risk_benchmark

# Custom parameters
bazel run //apps:risk_benchmark -- --positions 5000 --simulations 50000 --threads 4

# Run all tests
./scripts/run_tests.sh
```
