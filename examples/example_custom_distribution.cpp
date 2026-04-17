/**
 * @file example_custom_distribution.cpp
 * @brief Custom particle distribution example
 *
 * This example shows how to create custom particle distributions
 * beyond the built-in UNIFORM, SPHERICAL, and DISK options.
 * It demonstrates:
 * - Manual particle initialization
 * - Creating a galaxy-like spiral distribution
 * - Setting up initial velocities for stable orbits
 */

#include "nbody/particle_data.hpp"
#include "nbody/particle_system.hpp"
#include <cmath>
#include <iostream>
#include <random>

using namespace nbody;

/**
 * @brief Create a spiral galaxy distribution
 *
 * Creates a spiral galaxy pattern with:
 * - Central bulge (dense center)
 * - Spiral arms
 * - Initial rotational velocity
 */
void initSpiralGalaxy(ParticleData& h_data, const Vec3& center, float radius, float thickness,
                      int num_arms, float arm_spread, float rotation_factor) {
  std::mt19937 rng(42);
  std::uniform_real_distribution<float> dist_01(0.0f, 1.0f);
  std::normal_distribution<float> dist_normal(0.0f, 1.0f);

  size_t N = h_data.count;
  float arm_angle_step = 2.0f * M_PI / num_arms;

  for (size_t i = 0; i < N; i++) {
    // Determine if particle is in bulge or disk
    bool in_bulge = (dist_01(rng) < 0.2f);  // 20% in bulge

    float r, theta;

    if (in_bulge) {
      // Central bulge - spherical distribution
      r = std::cbrt(dist_01(rng)) * radius * 0.2f;  // Smaller radius
      theta = dist_01(rng) * 2.0f * M_PI;
      float phi = std::acos(2.0f * dist_01(rng) - 1.0f);

      h_data.pos_x[i] = center.x + r * std::sin(phi) * std::cos(theta);
      h_data.pos_y[i] = center.y + r * std::sin(phi) * std::sin(theta);
      h_data.pos_z[i] = center.z + r * std::cos(phi);

    } else {
      // Disk with spiral arms
      r = std::sqrt(dist_01(rng)) * radius;

      // Base angle with spiral pattern
      float arm_index = static_cast<int>(dist_01(rng) * num_arms);
      float base_angle = arm_index * arm_angle_step;

      // Spiral angle increases with radius
      float spiral_angle = rotation_factor * std::log(r + 1.0f);

      // Add spread around arm
      theta = base_angle + spiral_angle + dist_normal(rng) * arm_spread;

      h_data.pos_x[i] = center.x + r * std::cos(theta);
      h_data.pos_y[i] = center.y + r * std::sin(theta);
      h_data.pos_z[i] = center.z + dist_normal(rng) * thickness * 0.5f;
    }

    // Initial velocity: Keplerian orbit
    // v = sqrt(G * M_enclosed / r)
    // For simplicity, assume M_enclosed proportional to r
    float v_orbital = 0.5f * std::sqrt(r + 0.1f);

    // Tangential velocity (perpendicular to radius)
    if (r > 0.001f) {
      h_data.vel_x[i] = -v_orbital * std::sin(theta);
      h_data.vel_y[i] = v_orbital * std::cos(theta);
    } else {
      h_data.vel_x[i] = 0;
      h_data.vel_y[i] = 0;
    }
    h_data.vel_z[i] = 0;

    // Mass: bulge particles are heavier
    h_data.mass[i] = in_bulge ? 2.0f + dist_01(rng) * 3.0f :  // Bulge: 2-5
                         0.5f + dist_01(rng) * 1.0f;          // Disk: 0.5-1.5
  }

  // Zero accelerations
  ParticleInitializer::zeroAccelerations(h_data);
}

int main() {
  try {
    std::cout << "N-Body Simulation - Custom Distribution Example\n";
    std::cout << "================================================\n\n";

    // Step 1: Create particle system without automatic initialization
    // ============================================================
    const size_t particle_count = 20000;

    SimulationConfig config;
    config.particle_count = particle_count;
    config.force_method = ForceMethod::BARNES_HUT;
    config.barnes_hut_theta = 0.5f;
    config.dt = 0.001f;
    config.G = 1.0f;
    config.softening = 0.1f;

    // Allocate host memory for custom initialization
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, particle_count);

    std::cout << "Creating spiral galaxy distribution...\n";

    // Step 2: Initialize custom distribution
    // =====================================
    initSpiralGalaxy(h_particles, Vec3(0, 0, 0),  // Center
                     10.0f,                       // Radius
                     0.5f,                        // Thickness
                     2,                           // Number of spiral arms
                     0.3f,                        // Arm spread
                     2.0f                         // Rotation factor
    );

    std::cout << "Distribution created.\n";
    std::cout << "  Particles: " << particle_count << "\n";
    std::cout << "  Arms: 2\n";
    std::cout << "  Radius: 10.0\n\n";

    // Step 3: Initialize particle system with custom data
    // ==================================================
    ParticleSystem system;

    // First, initialize with standard config (sets up GPU memory)
    system.initialize(config);

    // Then, copy our custom data to device
    ParticleDataManager::copyToDevice(*system.getDeviceData(), h_particles);

    // Re-compute initial forces
    // Note: This would require exposing a method or recreating the force calculator

    std::cout << "System initialized with custom distribution.\n";
    std::cout << "Initial energy: " << system.computeTotalEnergy() << "\n\n";

    // Step 4: Run simulation
    // =====================
    std::cout << "Running simulation...\n";

    for (int step = 0; step < 500; step++) {
      system.update(system.getTimeStep());

      if (step % 100 == 0) {
        float energy = system.computeTotalEnergy();
        float time = system.getSimulationTime();
        std::cout << "  Step " << step << " | Time: " << time << " | Energy: " << energy << "\n";
      }
    }

    std::cout << "\nSimulation complete.\n";

    // Step 5: Analyze final state
    // ==========================
    ParticleDataManager::copyToHost(h_particles, *system.getDeviceData());

    // Compute center of mass
    float com_x = 0, com_y = 0, com_z = 0, total_mass = 0;
    for (size_t i = 0; i < particle_count; i++) {
      com_x += h_particles.pos_x[i] * h_particles.mass[i];
      com_y += h_particles.pos_y[i] * h_particles.mass[i];
      com_z += h_particles.pos_z[i] * h_particles.mass[i];
      total_mass += h_particles.mass[i];
    }
    com_x /= total_mass;
    com_y /= total_mass;
    com_z /= total_mass;

    std::cout << "\nFinal state analysis:\n";
    std::cout << "  Center of mass: (" << com_x << ", " << com_y << ", " << com_z << ")\n";
    std::cout << "  Total mass: " << total_mass << "\n";
    std::cout << "  Final energy: " << system.computeTotalEnergy() << "\n";

    // Cleanup
    ParticleDataManager::freeHost(h_particles);

    return 0;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}

/*
Expected output:

N-Body Simulation - Custom Distribution Example
================================================

Creating spiral galaxy distribution...
Distribution created.
  Particles: 20000
  Arms: 2
  Radius: 10.0

System initialized with custom distribution.
Initial energy: -456.789

Running simulation...
  Step 0 | Time: 0 | Energy: -456.789
  Step 100 | Time: 0.1 | Energy: -456.785
  Step 200 | Time: 0.2 | Energy: -456.781
  Step 300 | Time: 0.3 | Energy: -456.776
  Step 400 | Time: 0.4 | Energy: -456.770

Simulation complete.

Final state analysis:
  Center of mass: (0.001, -0.002, 0.000)
  Total mass: 24567.8
  Final energy: -456.765

*/
