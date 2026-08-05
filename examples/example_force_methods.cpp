/**
 * @file example_force_methods.cpp
 * @brief Comparison of different force calculation methods
 *
 * This example demonstrates the three force calculation algorithms:
 * - Direct N² (O(N²)): Exact calculation, suitable for small systems
 * - Barnes-Hut (O(N log N)): Approximate tree-based method
 * - Spatial Hash (O(N)): Efficient for short-range forces
 *
 * It shows how to switch between methods and compare their performance
 * and accuracy.
 */

#include "nbody/particle_system.hpp"
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace nbody;

// Helper function to measure execution time
template <typename Func>
double measureTime(Func func, int iterations = 1) {
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iterations; i++) {
    func();
  }
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double, std::milli>(end - start).count() / iterations;
}

// RMS position drift between two states with the same particle count.
double rmsPositionDrift(const SimulationState& a, const SimulationState& b) {
  double sum_sq = 0.0;
  for (size_t i = 0; i < a.particle_count; i++) {
    double dx = static_cast<double>(a.pos_x[i]) - b.pos_x[i];
    double dy = static_cast<double>(a.pos_y[i]) - b.pos_y[i];
    double dz = static_cast<double>(a.pos_z[i]) - b.pos_z[i];
    sum_sq += dx * dx + dy * dy + dz * dz;
  }
  return std::sqrt(sum_sq / static_cast<double>(a.particle_count));
}

int main() {
  try {
    std::cout << "N-Body Simulation - Force Methods Comparison\n";
    std::cout << "=============================================\n\n";

    // Configuration
    SimulationConfig config;
    config.particle_count = 5000;  // Keep small for comparison
    config.init_distribution = InitDistribution::SPHERICAL;
    config.dt = 0.001f;
    config.G = 1.0f;
    config.softening = 0.1f;

    std::cout << "Configuration:\n";
    std::cout << "  Particles: " << config.particle_count << "\n\n";

    // Initialize system with reference method
    ParticleSystem system;
    config.force_method = ForceMethod::DIRECT_N2;
    system.initialize(config);

    // Store initial state for reset
    auto initial_state = system.getState();

    std::cout << "Comparing Force Calculation Methods:\n";
    std::cout << std::string(70, '-') << "\n";
    std::cout << std::left << std::setw(20) << "Method" << std::setw(15) << "Time (ms)"
              << std::setw(15) << "RMS Drift" << std::setw(20) << "Notes" << "\n";
    std::cout << std::string(70, '-') << "\n";

    // Reference trajectory: evolve the exact Direct N² method 10 steps from
    // the initial state. Approximate methods are scored by how far their own
    // 10-step trajectory drifts from this reference.
    constexpr int kCompareSteps = 10;
    system.setState(initial_state);
    system.setForceMethod(ForceMethod::DIRECT_N2);
    for (int i = 0; i < kCompareSteps; i++) {
      system.update(system.getTimeStep());
    }
    const SimulationState reference_state = system.getState();

    // Test each method
    struct MethodTest {
      ForceMethod method;
      std::string name;
      std::string notes;
    };

    std::vector<MethodTest> methods = {
        {ForceMethod::DIRECT_N2, "Direct N²", "Exact calculation"},
        {ForceMethod::BARNES_HUT, "Barnes-Hut (θ=0.5)", "Default accuracy"},
        {ForceMethod::BARNES_HUT, "Barnes-Hut (θ=0.3)", "High accuracy"},
        {ForceMethod::SPATIAL_HASH, "Spatial Hash", "Short-range only"}};

    for (const auto& test : methods) {
      // Reset to initial state
      system.setState(initial_state);

      // Set force method
      system.setForceMethod(test.method);

      // Special configuration for Barnes-Hut
      if (test.method == ForceMethod::BARNES_HUT) {
        float theta = (test.name.find("0.3") != std::string::npos) ? 0.3f : 0.5f;
        system.setBarnesHutTheta(theta);
      }

      // Measure time for the same 10 steps used for the reference trajectory
      double time_ms = measureTime([&]() { system.update(system.getTimeStep()); }, kCompareSteps);

      // Position drift vs the Direct N² reference after the same 10 steps.
      // N/A for Spatial Hash: it models different (short-range) physics, so
      // its trajectory is not comparable to the gravitational reference.
      std::string error_str = "N/A";
      if (test.method != ForceMethod::SPATIAL_HASH) {
        const SimulationState state = system.getState();
        std::ostringstream oss;
        oss << std::scientific << std::setprecision(2) << rmsPositionDrift(state, reference_state);
        error_str = oss.str();
      }

      std::cout << std::left << std::setw(20) << test.name << std::setw(15) << std::fixed
                << std::setprecision(2) << time_ms << std::setw(15) << error_str << std::setw(20)
                << test.notes << "\n";
    }

    std::cout << std::string(70, '-') << "\n\n";

    // Performance scaling test
    std::cout << "Performance Scaling:\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << std::left << std::setw(15) << "Particles" << std::setw(15) << "Direct N²"
              << std::setw(15) << "Barnes-Hut" << std::setw(15) << "Spatial Hash" << "\n";
    std::cout << std::string(60, '-') << "\n";

    std::vector<size_t> sizes = {1000, 5000, 10000, 20000};

    for (size_t N : sizes) {
      config.particle_count = N;

      std::cout << std::setw(15) << N;

      for (auto method :
           {ForceMethod::DIRECT_N2, ForceMethod::BARNES_HUT, ForceMethod::SPATIAL_HASH}) {
        config.force_method = method;
        system.initialize(config);

        double time_ms = measureTime([&]() { system.update(system.getTimeStep()); }, 5);

        std::cout << std::setw(15) << std::fixed << std::setprecision(2) << time_ms;
      }
      std::cout << "\n";
    }

    std::cout << std::string(60, '-') << "\n\n";

    std::cout << "Key Observations:\n";
    std::cout << "  1. Direct N² scales quadratically - best for N < 10,000\n";
    std::cout << "  2. Barnes-Hut scales as O(N log N) - good for large N\n";
    std::cout << "  3. Spatial Hash scales linearly - ideal for short-range forces\n";
    std::cout << "  4. Lower θ in Barnes-Hut = higher accuracy, lower speed\n";

    return 0;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}

/*
Expected output (times and drift values will differ on your hardware):

N-Body Simulation - Force Methods Comparison
=============================================

Configuration:
  Particles: 5000

Comparing Force Calculation Methods:
----------------------------------------------------------------------
Method              Time (ms)      RMS Drift      Notes
----------------------------------------------------------------------
Direct N²           12.34          0.00e+00       Exact calculation
Barnes-Hut (θ=0.5)  3.45           1.23e-03       Default accuracy
Barnes-Hut (θ=0.3)  5.67           4.56e-04       High accuracy
Spatial Hash        1.23           N/A            Short-range only
----------------------------------------------------------------------

"RMS Drift" is the per-particle RMS position difference from the exact
Direct N² trajectory after the same 10 steps. Spatial Hash is not
comparable: it models short-range interactions, not full gravity.

Performance Scaling:
------------------------------------------------------------
Particles      Direct N²      Barnes-Hut     Spatial Hash
------------------------------------------------------------
1000           2.34           1.23           0.89
5000           12.34          3.45           1.23
10000          45.67          5.89           2.34
20000          178.90         10.23          4.56
------------------------------------------------------------

Key Observations:
  1. Direct N² scales quadratically - best for N < 10,000
  2. Barnes-Hut scales as O(N log N) - good for large N
  3. Spatial Hash scales linearly - ideal for short-range forces
  4. Lower θ in Barnes-Hut = higher accuracy, lower speed

*/
