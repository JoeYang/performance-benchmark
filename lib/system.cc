#include "lib/system.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#ifdef __linux__
#ifdef HAVE_NUMA
#include <numa.h>
#endif
#include <sched.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <pthread.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

namespace trading {

SystemInfo get_system_info() {
    SystemInfo info;
    info.num_cpus = std::thread::hardware_concurrency();
    info.num_numa_nodes = 1;

#ifdef __linux__
    // Read CPU model
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                info.cpu_model = line.substr(pos + 2);
            }
            break;
        }
    }

    // Read memory info
    std::ifstream meminfo("/proc/meminfo");
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal") != std::string::npos) {
            size_t kb;
            sscanf(line.c_str(), "MemTotal: %zu kB", &kb);
            info.total_memory_mb = kb / 1024;
            break;
        }
    }

    // NUMA info
#ifdef HAVE_NUMA
    if (numa_available() >= 0) {
        info.num_numa_nodes = numa_max_node() + 1;
        info.numa_cpu_map.resize(info.num_numa_nodes);

        for (int node = 0; node < info.num_numa_nodes; ++node) {
            struct bitmask* cpumask = numa_allocate_cpumask();
            if (numa_node_to_cpus(node, cpumask) == 0) {
                for (int cpu = 0; cpu < info.num_cpus; ++cpu) {
                    if (numa_bitmask_isbitset(cpumask, cpu)) {
                        info.numa_cpu_map[node].push_back(cpu);
                    }
                }
            }
            numa_free_cpumask(cpumask);
        }
    }
#endif
#endif

#ifdef __APPLE__
    // Get CPU model
    char cpu_brand[256];
    size_t size = sizeof(cpu_brand);
    if (sysctlbyname("machdep.cpu.brand_string", cpu_brand, &size, nullptr, 0) == 0) {
        info.cpu_model = cpu_brand;
    }

    // Get memory
    int64_t memsize;
    size = sizeof(memsize);
    if (sysctlbyname("hw.memsize", &memsize, &size, nullptr, 0) == 0) {
        info.total_memory_mb = memsize / (1024 * 1024);
    }

    // macOS doesn't have NUMA
    info.numa_cpu_map.resize(1);
    for (int i = 0; i < info.num_cpus; ++i) {
        info.numa_cpu_map[0].push_back(i);
    }
#endif

    return info;
}

bool set_thread_affinity(const std::vector<int>& cpus) {
    if (cpus.empty()) return false;

#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (int cpu : cpus) {
        CPU_SET(cpu, &cpuset);
    }
    return sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0;
#endif

#ifdef __APPLE__
    // macOS doesn't support true CPU affinity, but we can use affinity tags
    // This is a hint to the scheduler, not a hard binding
    thread_affinity_policy_data_t policy = {static_cast<integer_t>(cpus[0])};
    thread_port_t thread = pthread_mach_thread_np(pthread_self());
    return thread_policy_set(thread, THREAD_AFFINITY_POLICY,
                            (thread_policy_t)&policy, 1) == KERN_SUCCESS;
#endif

    return false;
}

bool set_thread_affinity(int start_cpu, int num_cpus) {
    std::vector<int> cpus;
    for (int i = 0; i < num_cpus; ++i) {
        cpus.push_back(start_cpu + i);
    }
    return set_thread_affinity(cpus);
}

bool bind_numa_node(int node) {
#if defined(__linux__) && defined(HAVE_NUMA)
    if (numa_available() < 0) return false;
    if (node < 0 || node > numa_max_node()) return false;

    numa_set_preferred(node);
    numa_set_localalloc();

    // Also set CPU affinity to CPUs on this NUMA node
    struct bitmask* cpumask = numa_allocate_cpumask();
    if (numa_node_to_cpus(node, cpumask) == 0) {
        numa_sched_setaffinity(0, cpumask);
    }
    numa_free_cpumask(cpumask);
    return true;
#else
    (void)node;
    return false;
#endif
}

bool lock_memory() {
#ifdef __linux__
    return mlockall(MCL_CURRENT | MCL_FUTURE) == 0;
#endif
#ifdef __APPLE__
    // macOS doesn't support mlockall, but we can hint
    return true;
#endif
    return false;
}

bool set_realtime_priority(int priority) {
#ifdef __linux__
    struct sched_param param;
    param.sched_priority = priority;
    return sched_setscheduler(0, SCHED_FIFO, &param) == 0;
#endif
#ifdef __APPLE__
    // macOS uses thread QoS
    return pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0) == 0;
#endif
    (void)priority;
    return false;
}

void prefault_stack(size_t size_kb) {
    // Use heap allocation since VLAs are not standard C++
    size_t size = size_kb * 1024;
    volatile char* stack = new char[size];
    memset((void*)stack, 0, size);
    delete[] stack;
}

void prefault_heap(size_t size_mb) {
    size_t size = size_mb * 1024 * 1024;
    volatile char* mem = new char[size];
    memset((void*)mem, 0, size);
    delete[] mem;
}

bool apply_system_config(const SystemConfig& config) {
    bool success = true;

    if (!config.cpu_affinity.empty()) {
        if (!set_thread_affinity(config.cpu_affinity)) {
            std::cerr << "Warning: Failed to set CPU affinity\n";
            success = false;
        }
    }

    if (config.numa_node >= 0) {
        if (!bind_numa_node(config.numa_node)) {
            std::cerr << "Warning: Failed to bind NUMA node "
                      << config.numa_node << "\n";
            success = false;
        }
    }

    if (config.lock_memory) {
        if (!lock_memory()) {
            std::cerr << "Warning: Failed to lock memory (may need root)\n";
            success = false;
        }
    }

    if (config.realtime_priority) {
        if (!set_realtime_priority()) {
            std::cerr << "Warning: Failed to set realtime priority (may need root)\n";
            success = false;
        }
    }

    if (config.prefault_memory) {
        prefault_stack(64);
    }

    if (config.preallocate_mb > 0) {
        prefault_heap(config.preallocate_mb);
    }

    return success;
}

void print_system_info(const SystemInfo& info) {
    std::cout << "System Information:\n";
    std::cout << "  CPU: " << info.cpu_model << "\n";
    std::cout << "  Cores: " << info.num_cpus << "\n";
    std::cout << "  NUMA nodes: " << info.num_numa_nodes << "\n";
    std::cout << "  Memory: " << info.total_memory_mb << " MB\n";

    if (info.num_numa_nodes > 1) {
        for (int node = 0; node < info.num_numa_nodes; ++node) {
            std::cout << "  NUMA " << node << " CPUs: ";
            for (size_t i = 0; i < info.numa_cpu_map[node].size(); ++i) {
                if (i > 0) std::cout << ",";
                std::cout << info.numa_cpu_map[node][i];
            }
            std::cout << "\n";
        }
    }
}

void print_applied_config(const SystemConfig& config) {
    std::cout << "Applied Configuration:\n";

    if (!config.cpu_affinity.empty()) {
        std::cout << "  CPU affinity: ";
        for (size_t i = 0; i < config.cpu_affinity.size(); ++i) {
            if (i > 0) std::cout << ",";
            std::cout << config.cpu_affinity[i];
        }
        std::cout << "\n";
    }

    if (config.numa_node >= 0) {
        std::cout << "  NUMA node: " << config.numa_node << "\n";
    }

    if (config.lock_memory) {
        std::cout << "  Memory locked: yes\n";
    }

    if (config.realtime_priority) {
        std::cout << "  Realtime priority: yes\n";
    }
}

std::vector<int> parse_cpu_list(const std::string& cpu_str) {
    std::vector<int> cpus;
    std::stringstream ss(cpu_str);
    std::string token;

    while (std::getline(ss, token, ',')) {
        size_t dash = token.find('-');
        if (dash != std::string::npos) {
            int start = std::stoi(token.substr(0, dash));
            int end = std::stoi(token.substr(dash + 1));
            for (int i = start; i <= end; ++i) {
                cpus.push_back(i);
            }
        } else {
            cpus.push_back(std::stoi(token));
        }
    }

    std::sort(cpus.begin(), cpus.end());
    cpus.erase(std::unique(cpus.begin(), cpus.end()), cpus.end());
    return cpus;
}

}  // namespace trading
