#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "nbody/integrator.hpp"
#include "nbody/force_calculator.hpp"
#include "nbody/particle_data.hpp"
#include "nbody/types.hpp"
#include <cmath>
#include <vector>

using namespace nbody;

// Unit Tests

TEST(IntegratorTest, SingleStepPositionUpdate) {
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, 1);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, 1);
    
    // Initial state: position at origin, velocity (1,0,0), no acceleration
    h_particles.pos_x[0] = 0.0f;
    h_particles.pos_y[0] = 0.0f;
    h_particles.pos_z[0] = 0.0f;
    h_particles.vel_x[0] = 1.0f;
    h_particles.vel_y[0] = 0.0f;
    h_particles.vel_z[0] = 0.0f;
    h_particles.acc_x[0] = 0.0f;
    h_particles.acc_y[0] = 0.0f;
    h_particles.acc_z[0] = 0.0f;
    h_particles.mass[0] = 1.0f;
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    Integrator integrator;
    float dt = 0.1f;
    integrator.updatePositions(&d_particles, dt);
    
    ParticleDataManager::copyToHost(h_particles, d_particles);
    
    // Expected: x = 0 + 1*0.1 + 0 = 0.1
    EXPECT_NEAR(h_particles.pos_x[0], 0.1f, 1e-5);
    EXPECT_NEAR(h_particles.pos_y[0], 0.0f, 1e-5);
    EXPECT_NEAR(h_particles.pos_z[0], 0.0f, 1e-5);
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
}

TEST(IntegratorTest, KineticEnergyCalculation) {
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, 2);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, 2);
    
    // Two particles with known velocities
    h_particles.vel_x[0] = 1.0f; h_particles.vel_y[0] = 0.0f; h_particles.vel_z[0] = 0.0f;
    h_particles.vel_x[1] = 0.0f; h_particles.vel_y[1] = 2.0f; h_particles.vel_z[1] = 0.0f;
    h_particles.mass[0] = 1.0f;
    h_particles.mass[1] = 2.0f;
    
    // Zero out other fields
    for (int i = 0; i < 2; i++) {
        h_particles.pos_x[i] = h_particles.pos_y[i] = h_particles.pos_z[i] = 0.0f;
        h_particles.acc_x[i] = h_particles.acc_y[i] = h_particles.acc_z[i] = 0.0f;
    }
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    Integrator integrator;
    float ke = integrator.computeKineticEnergy(&d_particles);
    
    // Expected: 0.5 * 1 * 1^2 + 0.5 * 2 * 2^2 = 0.5 + 4 = 4.5
    EXPECT_NEAR(ke, 4.5f, 1e-4);
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
}

// Property-Based Tests
// Feature: n-body-simulation, Property 7: Energy Conservation (Symplectic Integration)

RC_GTEST_PROP(Integrator, EnergyConservationProperty, ()) {
    // Feature: n-body-simulation, Property 7: Energy Conservation (Symplectic Integration)
    // Validates: Requirements 5.1, 5.4
    
    // Create a simple two-body system
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, 2);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, 2);
    
    // Two particles in circular orbit configuration
    float orbit_radius = 5.0f;
    float G = 1.0f;
    float m = 1.0f;
    float v_orbit = std::sqrt(G * m / (2 * orbit_radius));  // Circular orbit velocity
    
    h_particles.pos_x[0] = -orbit_radius; h_particles.pos_y[0] = 0.0f; h_particles.pos_z[0] = 0.0f;
    h_particles.pos_x[1] = orbit_radius;  h_particles.pos_y[1] = 0.0f; h_particles.pos_z[1] = 0.0f;
    h_particles.vel_x[0] = 0.0f; h_particles.vel_y[0] = -v_orbit; h_particles.vel_z[0] = 0.0f;
    h_particles.vel_x[1] = 0.0f; h_particles.vel_y[1] = v_orbit;  h_particles.vel_z[1] = 0.0f;
    h_particles.mass[0] = m;
    h_particles.mass[1] = m;
    
    for (int i = 0; i < 2; i++) {
        h_particles.acc_x[i] = h_particles.acc_y[i] = h_particles.acc_z[i] = 0.0f;
        h_particles.acc_old_x[i] = h_particles.acc_old_y[i] = h_particles.acc_old_z[i] = 0.0f;
    }
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    // Create force calculator and integrator
    DirectForceCalculator force_calc;
    force_calc.setGravitationalConstant(G);
    force_calc.setSofteningParameter(0.01f);
    
    Integrator integrator;
    
    // Compute initial forces
    force_calc.computeForces(&d_particles);
    
    // Compute initial energy
    float initial_energy = integrator.computeTotalEnergy(&d_particles, G, 0.01f);
    
    // Run simulation for several steps
    float dt = 0.001f;
    int num_steps = 100;
    
    for (int step = 0; step < num_steps; step++) {
        integrator.integrate(&d_particles, &force_calc, dt);
    }
    
    // Compute final energy
    float final_energy = integrator.computeTotalEnergy(&d_particles, G, 0.01f);
    
    // Property: Energy drift should be bounded
    float energy_drift = std::abs(final_energy - initial_energy);
    float relative_drift = energy_drift / std::abs(initial_energy);
    
    // Allow up to 1% drift over 100 steps with dt=0.001
    RC_ASSERT(relative_drift < 0.01f);
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
}
