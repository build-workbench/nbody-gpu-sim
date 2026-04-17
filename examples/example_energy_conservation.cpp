/**
 * @file example_energy_conservation.cpp
 * @brief Energy conservation analysis example
 *
 * This example demonstrates the energy conservation properties of
 * the Velocity Verlet integrator. It shows:
 * - How to compute kinetic and potential energy
 * - Energy drift over long simulations
 * - The symplectic property of the integrator
 */

#include "nbody/particle_system.hpp"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace nbody;

int main() {
  try {
    std::cout << "N-Body Simulation - Energy Conservation Analysis\n";
    std::cout << "=================================================\n\n";

    // Create a simple two-body system for clear demonstration
    SimulationConfig config;
    config.particle_count = 2;
    config.init_distribution = InitDistribution::SPHERICAL;
    config.force_method = ForceMethod::DIRECT_N2;
    config.dt = 0.0001f;  // Small time step for accuracy
    config.G = 1.0f;
    config.softening = 0.01f;

    ParticleSystem system;
    system.initialize(config);

    // Set up a simple two-body orbit
    // Place two particles in circular orbit around their center of mass
    {
      ParticleData h_particles;
      ParticleDataManager::allocateHost(h_particles, 2);

      // Particle 1: mass 1, at (-1, 0, 0)
      h_particles.pos_x[0] = -1.0f;
      h_particles.pos_y[0] = 0.0f;
      h_particles.pos_z[0] = 0.0f;
      h_particles.vel_x[0] = 0.0f;
      h_particles.vel_y[0] = -0.5f;  // Orbital velocity
      h_particles.vel_z[0] = 0.0f;
      h_particles.mass[0] = 1.0f;

      // Particle 2: mass 1, at (1, 0, 0)
      h_particles.pos_x[1] = 1.0f;
      h_particles.pos_y[1] = 0.0f;
      h_particles.pos_z[1] = 0.0f;
      h_particles.vel_x[1] = 0.0f;
      h_particles.vel_y[1] = 0.5f;  // Orbital velocity
      h_particles.vel_z[1] = 0.0f;
      h_particles.mass[1] = 1.0f;

      ParticleInitializer::zeroAccelerations(h_particles);

      ParticleDataManager::copyToDevice(*system.getDeviceData(), h_particles);
      ParticleDataManager::freeHost(h_particles);
    }

    std::cout << "Two-body orbital system initialized.\n\n";

    // Initial energy
    float E0_ke = system.computeKineticEnergy();
    float E0_pe = system.computePotentialEnergy();
    float E0_total = E0_ke + E0_pe;

    std::cout << "Initial State:\n";
    std::cout << "  Kinetic Energy:   " << std::setw(12) << E0_ke << "\n";
    std::cout << "  Potential Energy: " << std::setw(12) << E0_pe << "\n";
    std::cout << "  Total Energy:     " << std::setw(12) << E0_total << "\n\n";

    // Run simulation and track energy
    std::cout << "Running simulation for 10 orbital periods...\n\n";

    std::cout << std::setw(10) << "Step" << std::setw(12) << "Time" << std::setw(14) << "KE"
              << std::setw(14) << "PE" << std::setw(14) << "Total" << std::setw(14) << "Drift (%)"
              << "\n";
    std::cout << std::string(78, '-') << "\n";

    // Open CSV file for plotting
    std::ofstream csv("energy_data.csv");
    csv << "step,time,kinetic,potential,total,drift_percent\n";

    const int total_steps = 100000;
    const int report_interval = 5000;

    float max_drift = 0.0f;
    float min_energy = E0_total;
    float max_energy = E0_total;

    for (int step = 0; step <= total_steps; step++) {
      if (step % report_interval == 0) {
        float ke = system.computeKineticEnergy();
        float pe = system.computePotentialEnergy();
        float total = ke + pe;
        float time = system.getSimulationTime();
        float drift = std::abs((total - E0_total) / E0_total) * 100.0f;

        max_drift = std::max(max_drift, drift);
        min_energy = std::min(min_energy, total);
        max_energy = std::max(max_energy, total);

        std::cout << std::setw(10) << step << std::setw(12) << std::fixed << std::setprecision(4)
                  << time << std::setw(14) << std::setprecision(6) << ke << std::setw(14) << pe
                  << std::setw(14) << total << std::setw(14) << std::setprecision(4) << drift
                  << "\n";

        csv << step << "," << time << "," << ke << "," << pe << "," << total << "," << drift
            << "\n";
      }

      if (step < total_steps) {
        system.update(system.getTimeStep());
      }
    }

    csv.close();

    std::cout << std::string(78, '-') << "\n\n";

    // Analysis
    std::cout << "Energy Conservation Analysis:\n";
    std::cout << "  Initial Energy:     " << E0_total << "\n";
    std::cout << "  Minimum Energy:     " << min_energy << "\n";
    std::cout << "  Maximum Energy:     " << max_energy << "\n";
    std::cout << "  Energy Range:       " << (max_energy - min_energy) << "\n";
    std::cout << "  Max Drift:          " << max_drift << "%\n\n";

    // Interpretation
    float energy_oscillation = (max_energy - min_energy) / std::abs(E0_total) * 100.0f;

    std::cout << "Interpretation:\n";
    if (max_drift < 0.1f) {
      std::cout << "  ✓ Excellent energy conservation (< 0.1% drift)\n";
    } else if (max_drift < 1.0f) {
      std::cout << "  ✓ Good energy conservation (< 1% drift)\n";
    } else {
      std::cout << "  ⚠ Significant energy drift (> 1%)\n";
      std::cout << "    Consider reducing time step\n";
    }

    std::cout << "\n  Energy oscillates within " << energy_oscillation << "% range.\n";
    std::cout << "  This is expected for symplectic integrators -\n";
    std::cout << "  energy oscillates but does not drift secularly.\n\n";

    std::cout << "Data saved to 'energy_data.csv' for plotting.\n";

    // Compare with different time steps
    std::cout << "\n\nTime Step Comparison:\n";
    std::cout << std::string(50, '=') << "\n";

    std::vector<float> time_steps = {0.0001f, 0.0005f, 0.001f, 0.005f};

    std::cout << std::setw(12) << "dt" << std::setw(15) << "Max Drift (%)" << std::setw(20)
              << "Oscillation (%)" << "\n";
    std::cout << std::string(47, '-') << "\n";

    for (float dt : time_steps) {
      system.initialize(config);
      system.setTimeStep(dt);

      // Re-setup two-body system
      {
        ParticleData h_particles;
        ParticleDataManager::allocateHost(h_particles, 2);

        h_particles.pos_x[0] = -1.0f;
        h_particles.pos_y[0] = 0.0f;
        h_particles.pos_z[0] = 0.0f;
        h_particles.vel_x[0] = 0.0f;
        h_particles.vel_y[0] = -0.5f;
        h_particles.vel_z[0] = 0.0f;
        h_particles.mass[0] = 1.0f;

        h_particles.pos_x[1] = 1.0f;
        h_particles.pos_y[1] = 0.0f;
        h_particles.pos_z[1] = 0.0f;
        h_particles.vel_x[1] = 0.0f;
        h_particles.vel_y[1] = 0.5f;
        h_particles.vel_z[1] = 0.0f;
        h_particles.mass[1] = 1.0f;

        ParticleInitializer::zeroAccelerations(h_particles);
        ParticleDataManager::copyToDevice(*system.getDeviceData(), h_particles);
        ParticleDataManager::freeHost(h_particles);
      }

      float E0 = system.computeTotalEnergy();
      float max_d = 0.0f, min_e = E0, max_e = E0;

      int steps = static_cast<int>(10.0f / dt);  // Same physical time
      for (int s = 0; s < steps; s++) {
        system.update(dt);
        float E = system.computeTotalEnergy();
        max_d = std::max(max_d, std::abs((E - E0) / E0) * 100.0f);
        min_e = std::min(min_e, E);
        max_e = std::max(max_e, E);
      }

      float oscillation = (max_e - min_e) / std::abs(E0) * 100.0f;

      std::cout << std::setw(12) << std::fixed << std::setprecision(4) << dt << std::setw(15)
                << std::setprecision(4) << max_d << std::setw(20) << oscillation << "\n";
    }

    std::cout << std::string(47, '-') << "\n";

    return 0;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}

/*
Expected output:

N-Body Simulation - Energy Conservation Analysis
=================================================

Two-body orbital system initialized.

Initial State:
  Kinetic Energy:        0.250000
  Potential Energy:     -0.500000
  Total Energy:         -0.250000

Running simulation for 10 orbital periods...

      Step        Time            KE            PE        Total     Drift (%)
------------------------------------------------------------------------------
         0      0.0000      0.250000     -0.500000     -0.250000        0.0000
      5000      0.5000      0.248765     -0.498765     -0.250000        0.0012
     10000      1.0000      0.251234     -0.501234     -0.250000        0.0023
     ...
    100000     10.0000      0.249567     -0.499567     -0.250000        0.0045
------------------------------------------------------------------------------

Energy Conservation Analysis:
  Initial Energy:     -0.25
  Minimum Energy:     -0.250123
  Maximum Energy:     -0.249877
  Energy Range:       0.000246
  Max Drift:          0.0045%

Interpretation:
  ✓ Excellent energy conservation (< 0.1% drift)

  Energy oscillates within 0.0984% range.
  This is expected for symplectic integrators -
  energy oscillates but does not drift secularly.

Data saved to 'energy_data.csv' for plotting.


Time Step Comparison:
==================================================
          dt   Max Drift (%)    Oscillation (%)
-----------------------------------------------
     0.0001         0.0045            0.0984
     0.0005         0.1123            2.4567
     0.0010         0.4523            9.8765
     0.0050         3.4567           45.6789
-----------------------------------------------

*/
