#ifndef LIB_SYSTEM_H_
#define LIB_SYSTEM_H_

#include <string>
#include <vector>

namespace trading {

struct SystemConfig {
    std::vector<int> cpu_affinity;  // CPUs to bind to
    int numa_node = -1;              // NUMA node (-1 = no binding)
    bool lock_memory = false;        // Lock pages in RAM
    bool realtime_priority = false;  // Use realtime scheduling
    bool isolate_cpus = false;       // Isolate from OS scheduler
    bool prefault_memory = false;    // Pre-fault memory pages
    size_t preallocate_mb = 0;       // Pre-allocate memory (MB)
};

struct SystemInfo {
    int num_cpus;
    int num_numa_nodes;
    std::vector<std::vector<int>> numa_cpu_map;  // CPUs per NUMA node
    size_t total_memory_mb;
    std::string cpu_model;
};

// Get system information
SystemInfo get_system_info();

// Apply system configuration
bool apply_system_config(const SystemConfig& config);

// Set CPU affinity for current thread
bool set_thread_affinity(const std::vector<int>& cpus);

// Set CPU affinity for all threads in a range
bool set_thread_affinity(int start_cpu, int num_cpus);

// Bind to NUMA node
bool bind_numa_node(int node);

// Lock memory pages
bool lock_memory();

// Set realtime priority
bool set_realtime_priority(int priority = 50);

// Pre-fault memory to avoid page faults during execution
void prefault_stack(size_t size_kb = 64);
void prefault_heap(size_t size_mb);

// Print system configuration
void print_system_info(const SystemInfo& info);
void print_applied_config(const SystemConfig& config);

// Parse CPU list string (e.g., "0,1,2" or "0-3" or "0,2-4")
std::vector<int> parse_cpu_list(const std::string& cpu_str);

}  // namespace trading

#endif  // LIB_SYSTEM_H_
