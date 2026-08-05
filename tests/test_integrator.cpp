#include "nbody/force_calculator.hpp"
#include "nbody/integrator.hpp"
#include "nbody/particle_data.hpp"
#include "nbody/types.hpp"
#include "particle_data_fixture.hpp"
#include <cmath>
#include <gtest/gtest.h>

using namespace nbody;
using nbody::test::ScopedParticleData;

// Unit Tests

TEST(IntegratorTest, SingleStepPositionUpdate) {
  ScopedParticleData particles(1);

  // Initial state: position at origin, velocity (1,0,0), no acceleration
  particles.host.pos_x[0] = 0.0f;
  particles.host.pos_y[0] = 0.0f;
  particles.host.pos_z[0] = 0.0f;
  particles.host.vel_x[0] = 1.0f;
  particles.host.vel_y[0] = 0.0f;
  particles.host.vel_z[0] = 0.0f;
  particles.host.acc_x[0] = 0.0f;
  particles.host.acc_y[0] = 0.0f;
  particles.host.acc_z[0] = 0.0f;
  particles.host.mass[0] = 1.0f;

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  Integrator integrator;
  float dt = 0.1f;
  integrator.updatePositions(&particles.device, dt);

  ParticleDataManager::copyToHost(particles.host, particles.device);

  // Expected: x = 0 + 1*0.1 + 0 = 0.1
  EXPECT_NEAR(particles.host.pos_x[0], 0.1f, 1e-5);
  EXPECT_NEAR(particles.host.pos_y[0], 0.0f, 1e-5);
  EXPECT_NEAR(particles.host.pos_z[0], 0.0f, 1e-5);
}

TEST(IntegratorTest, ConstantAccelerationMatchesAnalyticSolution) {
  // With a constant acceleration a (re-applied identically each step), the
  // position after one Velocity-Verlet step must satisfy the exact kinematic
  // equation x = x0 + v0·dt + ½·a·dt².
  ScopedParticleData particles(1);

  particles.host.pos_x[0] = 1.0f;
  particles.host.pos_y[0] = -2.0f;
  particles.host.pos_z[0] = 0.5f;
  particles.host.vel_x[0] = 0.3f;
  particles.host.vel_y[0] = 0.0f;
  particles.host.vel_z[0] = -0.2f;
  particles.host.acc_x[0] = 2.0f;
  particles.host.acc_y[0] = -1.0f;
  particles.host.acc_z[0] = 4.0f;
  particles.host.acc_old_x[0] = 2.0f;
  particles.host.acc_old_y[0] = -1.0f;
  particles.host.acc_old_z[0] = 4.0f;
  particles.host.mass[0] = 1.0f;

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  Integrator integrator;
  const float dt = 0.01f;
  integrator.updatePositions(&particles.device, dt);

  ParticleDataManager::copyToHost(particles.host, particles.device);

  EXPECT_NEAR(particles.host.pos_x[0], 1.0f + 0.3f * dt + 0.5f * 2.0f * dt * dt, 1e-6f);
  EXPECT_NEAR(particles.host.pos_y[0], -2.0f + 0.0f * dt + 0.5f * -1.0f * dt * dt, 1e-6f);
  EXPECT_NEAR(particles.host.pos_z[0], 0.5f + -0.2f * dt + 0.5f * 4.0f * dt * dt, 1e-6f);
}

TEST(IntegratorTest, KineticEnergyCalculation) {
  ScopedParticleData particles(2);

  // Two particles with known velocities
  particles.host.vel_x[0] = 1.0f;
  particles.host.vel_y[0] = 0.0f;
  particles.host.vel_z[0] = 0.0f;
  particles.host.vel_x[1] = 0.0f;
  particles.host.vel_y[1] = 2.0f;
  particles.host.vel_z[1] = 0.0f;
  particles.host.mass[0] = 1.0f;
  particles.host.mass[1] = 2.0f;

  // Zero out other fields
  for (int i = 0; i < 2; i++) {
    particles.host.pos_x[i] = particles.host.pos_y[i] = particles.host.pos_z[i] = 0.0f;
    particles.host.acc_x[i] = particles.host.acc_y[i] = particles.host.acc_z[i] = 0.0f;
  }

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  Integrator integrator;
  float ke = integrator.computeKineticEnergy(&particles.device);

  // Expected: 0.5 * 1 * 1^2 + 0.5 * 2 * 2^2 = 0.5 + 4 = 4.5
  EXPECT_NEAR(ke, 4.5f, 1e-4);
}

// Feature: n-body-simulation, Property 7: Energy Conservation (Symplectic
// Integration)

TEST(IntegratorTest, EnergyConservationTwoBodyOrbit) {
  // Feature: n-body-simulation, Property 7: Energy Conservation (Symplectic
  // Integration). Validates: Requirements 5.1, 5.4

  ScopedParticleData particles(2);

  // Equal-mass two-body circular orbit: both masses circle the common center
  // of mass (the midpoint) at radius r, separation 2r. Balancing gravity
  // G·m²/(2r)² against centripetal m·v²/r gives v = √(G·m / (4r)).
  const float orbit_radius = 5.0f;
  const float G = 1.0f;
  const float m = 1.0f;
  const float eps = 0.01f;
  const float v_orbit = std::sqrt(G * m / (4.0f * orbit_radius));

  particles.host.pos_x[0] = -orbit_radius;
  particles.host.pos_y[0] = 0.0f;
  particles.host.pos_z[0] = 0.0f;
  particles.host.pos_x[1] = orbit_radius;
  particles.host.pos_y[1] = 0.0f;
  particles.host.pos_z[1] = 0.0f;
  particles.host.vel_x[0] = 0.0f;
  particles.host.vel_y[0] = -v_orbit;
  particles.host.vel_z[0] = 0.0f;
  particles.host.vel_x[1] = 0.0f;
  particles.host.vel_y[1] = v_orbit;
  particles.host.vel_z[1] = 0.0f;
  particles.host.mass[0] = m;
  particles.host.mass[1] = m;

  for (int i = 0; i < 2; i++) {
    particles.host.acc_x[i] = particles.host.acc_y[i] = particles.host.acc_z[i] = 0.0f;
    particles.host.acc_old_x[i] = particles.host.acc_old_y[i] = particles.host.acc_old_z[i] = 0.0f;
  }

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  DirectForceCalculator force_calc;
  force_calc.setGravitationalConstant(G);
  force_calc.setSofteningParameter(eps);

  Integrator integrator;

  // Compute initial forces a(0), required before the first Verlet step.
  force_calc.computeForces(&particles.device);

  const float initial_energy = integrator.computeTotalEnergy(&particles.device, G, eps);

  const float dt = 0.001f;
  const int num_steps = 100;
  for (int step = 0; step < num_steps; step++) {
    integrator.integrate(&particles.device, &force_calc, dt);
  }

  const float final_energy = integrator.computeTotalEnergy(&particles.device, G, eps);

  // A symplectic integrator on a near-circular orbit with dt=0.001 drifts on
  // the order of 1e-5 over 100 steps; 1e-3 leaves headroom for float error
  // while still failing a genuinely broken integrator.
  const float relative_drift = std::abs(final_energy - initial_energy) / std::abs(initial_energy);
  EXPECT_LT(relative_drift, 1e-3f);
}
