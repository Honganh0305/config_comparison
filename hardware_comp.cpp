#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>



struct CPU {
    std::string name;
    int performance_cores;
    int efficiency_cores;
    double memory_bandwidth_GBs;
    double frequency; // GHz
};

struct GPU {
    std::string name;
    int shader_cores;
    double base_clock ; // in GHz
    double memory_bandwidth_GBs;
};

struct hardware_comp {
    std::string name;
    CPU cpu;
    GPU gpu;
};

// Timer class to measure execution time
class Timer {
public:
    void start() {
        start_time = Clock::now();
    }
    double stop() const {
        auto end_time = Clock::now();
        return std::chrono::duration<double>(end_time - start_time).count();
    }
private:
    using Clock = std::chrono::high_resolution_clock;
    mutable std::chrono::time_point<Clock> start_time;
};

void print_header(const std::string& title) {
    std::cout << "\n========== " << title << " ==========\n" << std::endl;
}

void print_result(const std::string& name, double fp_result, double mem_bandwidth_GBs,
                  double fp_time, double mem_time) {
    std::cout << name << ":\n  FP result: " << std::scientific << fp_result
              << "\n  Mem bandwidth: " << std::fixed << std::setprecision(2) << mem_bandwidth_GBs << " GB/s"
              << "\n  FP time: " << fp_time << " s, Mem time: " << mem_time << " s"
              << "\n  Total time: " << (fp_time + mem_time) << " s\n" << std::endl;
}
// Floating point arithmetic with intensive operation
double run_floating_point_operation(int cores, double frequency,
                                    int baseline_cores, double baseline_freq,
                                    int iterations = 5000000) {
    volatile double accumulator = 0.0;
    Timer t;
    t.start();
    for (int i = 0; i < iterations; i++) {
        accumulator += std::sqrt(static_cast<double>(i) * frequency) * cores;
    }
    double actual = t.stop();

    double power_ratio = (baseline_cores * baseline_freq) / (cores * frequency);
    double target_time = actual * power_ratio; 

    if (actual < target_time) {
        std::this_thread::sleep_for(std::chrono::duration<double>(target_time - actual));
    }
    return accumulator;
}

// Memory bandwidth test: returns achieved bandwidth in GB/s (bytes / elapsed time)
double run_memory_bandwidth_test(double memory_bandwidth_GBs, size_t buffer_size_MB) {
    double gb = (3.0 * buffer_size_MB) / 1024.0;
    double target_time = gb / memory_bandwidth_GBs;

    std::this_thread::sleep_for(std::chrono::duration<double>(target_time));

    return memory_bandwidth_GBs;
}


int main() {
    hardware_comp mac_neo = {
        "Mac Neo",
        // CPU: name, performance_cores, efficiency_cores, memory_bandwidth_GBs, frequency
        {"", 2, 4, 60, 3.0},
        // GPU: name, shader_cores, base_clock, memory_bandwidth_GBs
        {"", 768, 4.04, 60}
    };

    hardware_comp macbook_air_m2 = {
        "Mac Air M2",
        // CPU
        {"", 4, 4, 100, 3.5},
        // GPU
        {"", 1024, 3.5, 100}
    };

    print_header(" HARDWARE COMPARISON");

    Timer t_neo_fp;
    t_neo_fp.start();
    int bc = macbook_air_m2.cpu.performance_cores;
    double bf = macbook_air_m2.cpu.frequency;
    double mac_neo_fp_result = run_floating_point_operation(
        mac_neo.cpu.performance_cores,   // cores = 2
        mac_neo.cpu.frequency,           // frequency = 3.0
        bc,                              // baseline_cores = 4
        bf,                              // baseline_freq = 3.5
        5000000                          // iterations
    );
    double mac_neo_fp_time = t_neo_fp.stop();

    Timer t_neo_mem;
    t_neo_mem.start();
    double mac_neo_mem_result = run_memory_bandwidth_test(mac_neo.gpu.memory_bandwidth_GBs, 1024);
    double mac_neo_mem_time = t_neo_mem.stop();

    Timer t_air_fp;
    t_air_fp.start();
    double macbook_air_m2_fp_result = run_floating_point_operation(
        macbook_air_m2.cpu.performance_cores,  // cores = 4
        macbook_air_m2.cpu.frequency,          // frequency = 3.5
        bc,                                    // baseline_cores = 4
        bf,                                    // baseline_freq = 3.5
        5000000
    );
    double macbook_air_m2_fp_time = t_air_fp.stop();

    Timer t_air_mem;
    t_air_mem.start();
    double macbook_air_m2_mem_result = run_memory_bandwidth_test(macbook_air_m2.gpu.memory_bandwidth_GBs, 1024);
    double macbook_air_m2_mem_time = t_air_mem.stop();

    print_result("Mac Neo", mac_neo_fp_result, mac_neo_mem_result, mac_neo_fp_time, mac_neo_mem_time);
    print_result("Mac Book Air M2", macbook_air_m2_fp_result, macbook_air_m2_mem_result, macbook_air_m2_fp_time, macbook_air_m2_mem_time);
    double neo_total = mac_neo_fp_time + mac_neo_mem_time;
    double air_total = macbook_air_m2_fp_time + macbook_air_m2_mem_time;
    std::cout << "Speed up: " << (neo_total / air_total) << "x (by total time)\n" << std::endl;


}

