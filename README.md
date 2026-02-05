# Trading System CPU Benchmark

A C++ benchmark suite simulating trading system workloads for CPU performance testing. Includes Monte Carlo VaR, options Greeks calculation, and position aggregation with both single-threaded and multi-threaded implementations.

## Features

- **Monte Carlo VaR**: Value-at-Risk simulation using Geometric Brownian Motion
- **Greeks Calculation**: Black-Scholes option pricing with Delta, Gamma, Vega, Theta
- **Position Aggregation**: Portfolio netting and exposure calculation
- **Multi-threading**: Parallel execution with configurable thread count
- **System Tuning**: CPU affinity, NUMA binding, memory locking, realtime priority
- **Cross-platform**: Works on Linux and macOS

## Requirements

### Build from Source
- C++17 compatible compiler (GCC 7+, Clang 5+)
- CMake 3.14+ (recommended) or Bazel 6+
- pthread (Linux)

### Pre-built Package
- No dependencies required

## Quick Start

### Option 1: Build with CMake (Recommended for Linux)

```bash
# Clone and build
git clone <repository>
cd performance-benchmark

# Build
./scripts/build.sh

# Run benchmark
./build/risk_benchmark

# Run tests
cd build && ctest --output-on-failure
```

### Option 2: Build with Bazel

```bash
# Build and run
bazel run //apps:risk_benchmark

# Run tests
bazel test //...
```

### Option 3: Install from Package

```bash
# Extract the package
tar -xzf trading-benchmark-0.1.0-Linux.tar.gz
cd trading-benchmark-0.1.0-Linux

# Run benchmark
./bin/risk_benchmark

# Or install system-wide
sudo cp bin/* /usr/local/bin/
risk_benchmark
```

## Usage

### Running the Benchmark

```bash
# Default settings (10K positions, 100K simulations)
./build/risk_benchmark

# Custom parameters
./build/risk_benchmark --positions 5000 --simulations 50000 --threads 4

# Show help
./build/risk_benchmark --help
```

### Command Line Options

**Benchmark Options:**

| Option | Description | Default |
|--------|-------------|---------|
| `--positions N` | Number of positions to simulate | 10000 |
| `--simulations N` | Number of Monte Carlo simulations | 100000 |
| `--threads N` | Number of threads for parallel execution | auto-detect |

**System Tuning Options:**

| Option | Description | Notes |
|--------|-------------|-------|
| `--cpus LIST` | Pin to specific CPUs | e.g., `0,1,2` or `0-3` or `0,2-4` |
| `--numa-node N` | Bind to NUMA node N | Linux only |
| `--lock-memory` | Lock memory pages in RAM | May require root |
| `--realtime` | Use realtime scheduling priority | Requires root |
| `--prefault` | Pre-fault memory pages | Reduces runtime jitter |
| `--preallocate N` | Pre-allocate N MB of memory | Warm up allocator |
| `--isolate` | Apply all isolation options | Combines lock, prefault, preallocate |

**Information Options:**

| Option | Description |
|--------|-------------|
| `--sysinfo` | Print system information and exit |
| `--help` | Show help message |

### System Tuning Examples

```bash
# Show system info (CPU, NUMA nodes, memory)
./build/risk_benchmark --sysinfo

# Pin to CPUs 0-3 on NUMA node 0 (Linux)
./build/risk_benchmark --numa-node 0 --cpus 0-3

# Full isolation for low-latency testing (requires root)
sudo ./build/risk_benchmark --isolate --realtime --cpus 2-5

# With Solarflare Onload for kernel bypass (if available)
onload ./build/risk_benchmark --isolate --cpus 0-3

# Pre-allocate 512MB and lock memory
./build/risk_benchmark --preallocate 512 --lock-memory
```

### Running Tests

```bash
# Using CMake/CTest
cd build
ctest --output-on-failure

# Run specific test
./build/greeks_test
./build/monte_carlo_test
./build/aggregator_test

# Using Bazel
bazel test //lib:all
```

## Building Packages

Create distribution packages for deployment:

```bash
# Build and package
./scripts/package.sh

# Packages created in build/:
#   trading-benchmark-0.1.0-Linux.tar.gz
#   trading-benchmark-0.1.0-Linux.deb (Debian/Ubuntu)
#   trading-benchmark-0.1.0-Linux.rpm (RHEL/CentOS)
```

## Example Output

```
=== Trading System CPU Benchmark ===
Positions: 10000 | Simulations: 100000 | Threads: 10
--------------------------------------------------

Generating 10000 random positions...
Generated in 4.1 ms

Monte Carlo VaR:
  Single-threaded: 113618.8 ms
  Multi-threaded:   14883.1 ms (7.6x speedup)

Greeks Calculation:
  Single-threaded:      1.8 ms
  Multi-threaded:       0.6 ms (2.9x speedup)

Position Aggregation:
  Single-threaded:      1.8 ms
  Multi-threaded:       1.8 ms (1.0x speedup)

--------------------------------------------------
Results Summary:
  VaR (99%):        $  1233844.05
  VaR (95%):        $   873598.39
  Expected Shortfall: $1408734.57
  Portfolio Delta:       -13846.06
  Net Exposure:     $ 16221654.84
  Unique Symbols:              500
```

## Project Structure

```
performance-benchmark/
├── CMakeLists.txt          # CMake build configuration
├── MODULE.bazel            # Bazel module configuration
├── README.md               # This file
├── BENCHMARK_RESULTS.md    # Sample benchmark results
├── lib/
│   ├── position.h/cc       # Position data structures
│   ├── monte_carlo.h/cc    # Monte Carlo VaR engine
│   ├── greeks.h/cc         # Black-Scholes & Greeks
│   ├── aggregator.h/cc     # Position aggregation
│   ├── benchmark.h/cc      # Timing utilities
│   ├── system.h/cc         # CPU affinity, NUMA, system tuning
│   └── *_test.cc           # Unit tests
├── apps/
│   ├── risk_benchmark.cc   # Main benchmark application
│   └── calculator.cc       # Simple demo app
└── scripts/
    ├── build.sh            # Build script
    ├── package.sh          # Packaging script
    └── run_tests.sh        # Test runner (Bazel)
```

## Platform Notes

### Linux

```bash
# Install build dependencies (Debian/Ubuntu)
sudo apt-get install build-essential cmake libnuma-dev

# Install build dependencies (RHEL/CentOS/Fedora)
sudo yum groupinstall "Development Tools"
sudo yum install cmake numactl-devel

# For Solarflare Onload (optional, for kernel bypass)
# Install from https://github.com/Xilinx-CNS/onload
```

**NUMA Tuning on Linux:**
```bash
# Check NUMA topology
numactl --hardware

# Run benchmark on NUMA node 0
./build/risk_benchmark --numa-node 0 --cpus 0-7

# Or use numactl directly
numactl --cpunodebind=0 --membind=0 ./build/risk_benchmark
```

**Realtime Priority:**
```bash
# Grant realtime permissions to user (add to /etc/security/limits.conf)
# username  -  rtprio  99

# Or run with sudo
sudo ./build/risk_benchmark --realtime --cpus 2-5
```

### macOS

```bash
# Install Xcode command line tools
xcode-select --install

# Install CMake via Homebrew
brew install cmake
```

**Note:** macOS has limited support for CPU affinity (hints only) and no NUMA.
The `--cpus` option provides scheduling hints but not hard binding.

## License

MIT License
