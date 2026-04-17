/**
 * @file example_basic.cpp
 * @brief Basic N-Body simulation example
 *
 * This example demonstrates the minimal setup required to run an N-Body
 * simulation using the particle system. It shows how to:
 * - Configure the simulation
 * - Initialize the particle system
 * - Run a basic simulation loop
 * - Save and load simulation state
 */

#include "nbody/particle_system.hpp"
#include <chrono>
#include <iostream>

using namespace nbody;

int main() {
  try {
    std::cout << "N-Body Simulation - Basic Example\n";
    std::cout << "==================================\n\n";

    // Step 1: Configure the simulation
    // =================================
    SimulationConfig config;
    config.particle_count = 10000;                           // Number of particles
    config.init_distribution = InitDistribution::SPHERICAL;  // Initial distribution
    config.force_method = ForceMethod::BARNES_HUT;           // Force algorithm
    config.dt = 0.001f;                                      // Time step
    config.G = 1.0f;                                         // Gravitational constant
    config.softening = 0.1f;                                 // Softening parameter
    config.barnes_hut_theta = 0.5f;                          // Barnes-Hut accuracy

    std::cout << "Configuration:\n";
    std::cout << "  Particles: " << config.particle_count << "\n";
    std::cout << "  Algorithm: Barnes-Hut (theta=" << config.barnes_hut_theta << ")\n";
    std::cout << "  Time step: " << config.dt << "\n\n";

    // Step 2: Initialize the particle system
    // ======================================
    ParticleSystem system;
    system.initialize(config);

    std::cout << "System initialized successfully.\n";
    std::cout << "Initial energy: " << system.computeTotalEnergy() << "\n\n";

    // Step 3: Run simulation loop
    // ===========================
    const int num_steps = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int step = 0; step < num_steps; step++) {
      // Advance simulation by one time step
      system.update(system.getTimeStep());

      // Print progress every 100 steps
      if (step % 100 == 0) {
        float energy = system.computeTotalEnergy();
        float time = system.getSimulationTime();
        std::cout << "Step " << step << " | Time: " << time << " | Energy: " << energy << "\n";
      }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "\nSimulation completed:\n";
    std::cout << "  Steps: " << num_steps << "\n";
    std::cout << "  Time elapsed: " << duration.count() << " ms\n";
    std::cout << "  Steps per second: " << (num_steps * 1000.0 / duration.count()) << "\n";

    // Step 4: Save state
    // ==================
    system.saveState("simulation_checkpoint.nbody");
    std::cout << "\nState saved to 'simulation_checkpoint.nbody'\n";

    // Step 5: Load state (demonstration)
    // ==================================
    ParticleSystem loaded_system;
    loaded_system.loadState("simulation_checkpoint.nbody");

    std::cout << "State loaded successfully.\n";
    std::cout << "Final energy (loaded): " << loaded_system.computeTotalEnergy() << "\n";

    return 0;

  } catch (const CudaException& e) {
    std::cerr << "CUDA Error: " << e.what() << "\n";
    return 1;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}

/*
Expected output:

N-Body Simulation - Basic Example
==================================

Configuration:
  Particles: 10000
  Algorithm: Barnes-Hut (theta=0.5)
  Time step: 0.001

System initialized successfully.
Initial energy: -123.456

Step 0 | Time: 0 | Energy: -123.456
Step 100 | Time: 0.1 | Energy: -123.455
Step 200 | Time: 0.2 | Energy: -123.454
...
Step 900 | Time: 0.9 | Energy: -123.448

Simulation completed:
  Steps: 1000
  Time elapsed: 1234 ms
  Steps per second: 810.37

State saved to 'simulation_checkpoint.nbody'
State loaded successfully.
Final energy (loaded): -123.447

*/
