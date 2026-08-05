#include "nbody/force_calculator.hpp"
#include "nbody/particle_data.hpp"
#include "nbody/spatial_hash_grid.hpp"
#include "nbody/types.hpp"
#include "particle_data_fixture.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include <set>

using namespace nbody;
using nbody::test::ScopedParticleData;
using nbody::test::setParticle;

// Unit Tests

TEST(SpatialHashGridTest, BuildGrid) {
  ScopedParticleData particles(100);

  UniformDistParams params;
  params.min_bounds = Vec3(-10, -10, -10);
  params.max_bounds = Vec3(10, 10, 10);
  ParticleInitializer::initUniform(particles.host, params);

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  SpatialHashGrid grid(100, 2.0f);
  grid.build(&particles.device);

  EXPECT_GT(grid.getTotalCells(), 0);
}

TEST(SpatialHashGridTest, CellIndexCalculation) {
  float cell_size = 2.0f;

  // Test cell index calculation
  int3 cell1 = SpatialHashGrid::getCellIndex(0.5f, 0.5f, 0.5f, cell_size);
  EXPECT_EQ(cell1.x, 0);
  EXPECT_EQ(cell1.y, 0);
  EXPECT_EQ(cell1.z, 0);

  int3 cell2 = SpatialHashGrid::getCellIndex(2.5f, 4.5f, 6.5f, cell_size);
  EXPECT_EQ(cell2.x, 1);
  EXPECT_EQ(cell2.y, 2);
  EXPECT_EQ(cell2.z, 3);
}

TEST(SpatialHashCalculatorTest, ComputeForces) {
  ScopedParticleData particles(100);

  UniformDistParams params;
  params.min_bounds = Vec3(-5, -5, -5);
  params.max_bounds = Vec3(5, 5, 5);
  ParticleInitializer::initUniform(particles.host, params);

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  // cell_size must be >= cutoff for the 3x3x3 neighbor search to be complete.
  SpatialHashCalculator calc(2.0f, 2.0f);
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

// Feature: n-body-simulation, Property 6: Spatial Hash Neighbor Cutoff
//
// Deterministic two-particle scenarios on both sides of the cutoff boundary:
// outside the cutoff the force must be exactly zero; inside it must equal
// the analytical softened value. This pins down the cutoff behavior that a
// "some force exists" assertion could never catch.
TEST(SpatialHashGridTest, CutoffBoundaryBehavior) {
  const float G = 1.0f;
  const float eps = 0.01f;
  const float cutoff = 2.0f;
  const float cell_size = 2.0f;  // must be >= cutoff
  const float delta = 0.25f;

  // Case 1: separation = cutoff + delta → strictly no interaction.
  {
    ScopedParticleData particles(2);
    setParticle(particles.host, 0, Vec3(0, 0, 0), Vec3(), 1.0f);
    setParticle(particles.host, 1, Vec3(cutoff + delta, 0, 0), Vec3(), 1.0f);
    ParticleDataManager::copyToDevice(particles.device, particles.host);

    SpatialHashCalculator calc(cell_size, cutoff);
    calc.setGravitationalConstant(G);
    calc.setSofteningParameter(eps);
    calc.computeForces(&particles.device);

    ParticleDataManager::copyToHost(particles.host, particles.device);
    EXPECT_FLOAT_EQ(particles.host.acc_x[0], 0.0f);
    EXPECT_FLOAT_EQ(particles.host.acc_y[0], 0.0f);
    EXPECT_FLOAT_EQ(particles.host.acc_z[0], 0.0f);
    EXPECT_FLOAT_EQ(particles.host.acc_x[1], 0.0f);
  }

  // Case 2: separation = cutoff - delta → analytical softened acceleration.
  {
    ScopedParticleData particles(2);
    const float r = cutoff - delta;
    setParticle(particles.host, 0, Vec3(0, 0, 0), Vec3(), 1.0f);
    setParticle(particles.host, 1, Vec3(r, 0, 0), Vec3(), 1.0f);
    ParticleDataManager::copyToDevice(particles.device, particles.host);

    SpatialHashCalculator calc(cell_size, cutoff);
    calc.setGravitationalConstant(G);
    calc.setSofteningParameter(eps);
    calc.computeForces(&particles.device);

    ParticleDataManager::copyToHost(particles.host, particles.device);

    const float dist2 = r * r + eps * eps;
    const float expected = G * 1.0f * r / (dist2 * std::sqrt(dist2));
    EXPECT_NEAR(particles.host.acc_x[0], expected, 1e-4f);
    EXPECT_NEAR(particles.host.acc_x[1], -expected, 1e-4f);
  }
}

// Property-Based Tests
// Feature: n-body-simulation, Property 5: Spatial Hash Cell Assignment
// Correctness

RC_GTEST_PROP(SpatialHashGrid, CellAssignmentCorrectness, (float cell_size)) {
  // Feature: n-body-simulation, Property 5: Spatial Hash Cell Assignment
  // Correctness Validates: Requirements 4.1, 4.2

  RC_PRE(cell_size > 0.5f && cell_size < 10.0f);

  size_t N = 50;

  ScopedParticleData particles(N);

  UniformDistParams params;
  params.min_bounds = Vec3(-10, -10, -10);
  params.max_bounds = Vec3(10, 10, 10);
  ParticleInitializer::initUniform(particles.host, params);

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  SpatialHashGrid grid(N, cell_size);
  grid.build(&particles.device);

  // Get cell data
  std::vector<int> cell_start, cell_end, particle_cells, sorted_indices;
  grid.copyCellDataToHost(cell_start, cell_end, particle_cells, sorted_indices);

  // Property: Each particle is assigned to exactly one cell
  std::set<int> seen_particles;
  for (int cell = 0; cell < grid.getTotalCells(); cell++) {
    for (int i = cell_start[cell]; i < cell_end[cell]; i++) {
      int particle_idx = sorted_indices[i];
      RC_ASSERT(seen_particles.find(particle_idx) == seen_particles.end());
      seen_particles.insert(particle_idx);
    }
  }
  RC_ASSERT(seen_particles.size() == N);
}

// Feature: n-body-simulation, Property 4: Force Method Equivalence

TEST(ForceMethodEquivalence, DirectVsBarnesHut) {
  // Feature: n-body-simulation, Property 4: Force Method Equivalence
  // Validates: Requirements 3.5
  //
  // With theta=0.1 the Barnes-Hut approximation must agree with the direct
  // calculation per component — comparing magnitudes only would accept a
  // completely wrong direction.

  constexpr size_t N = 30;
  ScopedParticleData particles(N);

  SphericalDistParams params;
  params.center = Vec3(0, 0, 0);
  params.radius = 5.0f;
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

  // Compute Barnes-Hut forces with small theta
  BarnesHutCalculator bh_calc(0.1f);  // Small theta for accuracy
  bh_calc.setGravitationalConstant(1.0f);
  bh_calc.setSofteningParameter(0.1f);
  bh_calc.computeForces(&particles.device);

  ParticleDataManager::copyToHost(particles.host, particles.device);

  // Per-component relative error with an absolute floor, so components near
  // zero do not blow up the relative measure.
  float max_relative_error = 0.0f;
  const auto compare = [&max_relative_error](float bh, float direct) {
    const float scale = std::max(1e-4f, std::abs(direct));
    max_relative_error = std::max(max_relative_error, std::abs(bh - direct) / scale);
  };

  for (size_t i = 0; i < N; i++) {
    compare(particles.host.acc_x[i], direct_acc_x[i]);
    compare(particles.host.acc_y[i], direct_acc_y[i]);
    compare(particles.host.acc_z[i], direct_acc_z[i]);
  }

  // theta=0.1 typically yields <1% error; 2% leaves float slack while still
  // failing a degraded approximation.
  EXPECT_LT(max_relative_error, 0.02f);
}
