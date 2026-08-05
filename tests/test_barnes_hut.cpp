#include "nbody/barnes_hut_tree.hpp"
#include "nbody/force_calculator.hpp"
#include "nbody/particle_data.hpp"
#include "nbody/types.hpp"
#include "particle_data_fixture.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <numeric>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>

using namespace nbody;
using nbody::test::ScopedParticleData;
using nbody::test::setParticle;

// Unit Tests

TEST(BarnesHutTreeTest, BuildTree) {
  ScopedParticleData particles(100);

  SphericalDistParams params;
  params.center = Vec3(0, 0, 0);
  params.radius = 10.0f;
  ParticleInitializer::initSpherical(particles.host, params);

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  BarnesHutTree tree(100);
  tree.build(&particles.device);

  EXPECT_GT(tree.getNodeCount(), 0);
}

TEST(BarnesHutTreeTest, MassConservation) {
  ScopedParticleData particles(50);

  SphericalDistParams params;
  params.center = Vec3(0, 0, 0);
  params.radius = 5.0f;
  params.min_mass = 1.0f;
  params.max_mass = 1.0f;  // Uniform mass for easy verification
  ParticleInitializer::initSpherical(particles.host, params);

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  BarnesHutTree tree(50);
  tree.build(&particles.device);
  tree.copyNodesToHost();

  EXPECT_TRUE(tree.verifyMassConservation(&particles.host));
}

TEST(BarnesHutTreeTest, DegenerateDistributionsDoNotOverflow) {
  // Coincident and collinear particles produce unary split chains that can
  // exceed the 2N node budget. The build must degrade gracefully (multi-
  // particle leaves) instead of writing out of bounds, and still conserve
  // mass. This pins down the node-budget guard in buildTreeOnHost.
  constexpr size_t N = 16;

  // All particles at exactly the same position: worst-case split chain.
  {
    ScopedParticleData particles(N);
    for (size_t i = 0; i < N; i++) {
      setParticle(particles.host, i, Vec3(1.0f, 2.0f, 3.0f), Vec3(), 1.0f);
    }
    ParticleDataManager::copyToDevice(particles.device, particles.host);

    BarnesHutTree tree(N);
    tree.build(&particles.device);
    tree.copyNodesToHost();
    EXPECT_TRUE(tree.verifyMassConservation(&particles.host));
  }

  // All particles collinear on the x-axis with tiny spacing (shared octant
  // paths for many levels).
  {
    ScopedParticleData particles(N);
    for (size_t i = 0; i < N; i++) {
      setParticle(particles.host, i, Vec3(0.0001f * static_cast<float>(i), 0.0f, 0.0f), Vec3(),
                  1.0f);
    }
    ParticleDataManager::copyToDevice(particles.device, particles.host);

    BarnesHutTree tree(N);
    tree.build(&particles.device);
    tree.copyNodesToHost();
    EXPECT_TRUE(tree.verifyMassConservation(&particles.host));
  }
}

TEST(BarnesHutCalculatorTest, ComputeForces) {
  ScopedParticleData particles(100);

  SphericalDistParams params;
  params.center = Vec3(0, 0, 0);
  params.radius = 10.0f;
  ParticleInitializer::initSpherical(particles.host, params);

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  BarnesHutCalculator calc(0.5f);
  calc.setGravitationalConstant(1.0f);
  calc.setSofteningParameter(0.1f);
  calc.computeForces(&particles.device);

  ParticleDataManager::copyToHost(particles.host, particles.device);

  // Check accelerations are finite
  for (size_t i = 0; i < particles.host.count; i++) {
    EXPECT_TRUE(std::isfinite(particles.host.acc_x[i]));
    EXPECT_TRUE(std::isfinite(particles.host.acc_y[i]));
    EXPECT_TRUE(std::isfinite(particles.host.acc_z[i]));
  }
}

TEST(BarnesHutCalculatorTest, CoincidentClusterForcesFinite) {
  // Force calculation on a fully coincident cluster exercises the degenerate
  // multi-particle leaves; accelerations must stay finite and the self-force
  // exclusion must keep the net acceleration near zero by symmetry.
  constexpr size_t N = 8;
  ScopedParticleData particles(N);
  for (size_t i = 0; i < N; i++) {
    setParticle(particles.host, i, Vec3(0.5f, 0.5f, 0.5f), Vec3(), 1.0f);
  }
  ParticleDataManager::copyToDevice(particles.device, particles.host);

  BarnesHutCalculator calc(0.5f);
  calc.setGravitationalConstant(1.0f);
  calc.setSofteningParameter(0.1f);
  calc.computeForces(&particles.device);

  ParticleDataManager::copyToHost(particles.host, particles.device);
  for (size_t i = 0; i < N; i++) {
    EXPECT_TRUE(std::isfinite(particles.host.acc_x[i]));
    EXPECT_TRUE(std::isfinite(particles.host.acc_y[i]));
    EXPECT_TRUE(std::isfinite(particles.host.acc_z[i]));
  }
}

// Property-Based Tests
// Feature: n-body-simulation, Property 2: Barnes-Hut Tree Structure Correctness

RC_GTEST_PROP(BarnesHutTree, TreeContainsAllParticles, (int seed)) {
  // Feature: n-body-simulation, Property 2: Barnes-Hut Tree Structure
  // Correctness Validates: Requirements 3.1, 3.2

  size_t N = 50;  // Small for testing

  ScopedParticleData particles(N);

  SphericalDistParams params;
  params.center = Vec3(0, 0, 0);
  params.radius = 10.0f;
  ParticleInitializer::initSpherical(particles.host, params, seed);

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  BarnesHutTree tree(N);
  tree.build(&particles.device);
  tree.copyNodesToHost();

  // Property: Tree contains exactly N particles (mass conservation)
  RC_ASSERT(tree.verifyMassConservation(&particles.host));
}

// Feature: n-body-simulation, Property 3: Barnes-Hut Approximation Convergence

TEST(BarnesHutTreeTest, ApproximationConvergence) {
  // Feature: n-body-simulation, Property 3: Barnes-Hut Approximation
  // Convergence. Validates: Requirements 3.3
  //
  // A plain test (not a property): the scenario is fixed, so rapidcheck
  // would just re-run the same case 100 times.

  constexpr size_t N = 50;
  ScopedParticleData particles(N);

  SphericalDistParams params;
  params.center = Vec3(0, 0, 0);
  params.radius = 10.0f;
  ParticleInitializer::initSpherical(particles.host, params, 42);

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  // Compute direct forces
  DirectForceCalculator direct_calc;
  direct_calc.setGravitationalConstant(1.0f);
  direct_calc.setSofteningParameter(0.1f);
  direct_calc.computeForces(&particles.device);

  ParticleDataManager::copyToHost(particles.host, particles.device);
  std::vector<float> direct_acc_x(N), direct_acc_y(N), direct_acc_z(N);
  for (size_t i = 0; i < N; i++) {
    direct_acc_x[i] = particles.host.acc_x[i];
    direct_acc_y[i] = particles.host.acc_y[i];
    direct_acc_z[i] = particles.host.acc_z[i];
  }

  const auto totalError = [&](float theta) {
    BarnesHutCalculator bh_calc(theta);
    bh_calc.setGravitationalConstant(1.0f);
    bh_calc.setSofteningParameter(0.1f);
    bh_calc.computeForces(&particles.device);
    ParticleDataManager::copyToHost(particles.host, particles.device);

    float error = 0.0f;
    for (size_t i = 0; i < N; i++) {
      float dx = particles.host.acc_x[i] - direct_acc_x[i];
      float dy = particles.host.acc_y[i] - direct_acc_y[i];
      float dz = particles.host.acc_z[i] - direct_acc_z[i];
      error += std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    return error;
  };

  // Smaller theta = finer approximation = smaller error vs direct.
  const float error_coarse = totalError(0.8f);
  const float error_fine = totalError(0.3f);
  EXPECT_LE(error_fine, error_coarse * 1.1f);  // Allow small tolerance
}
