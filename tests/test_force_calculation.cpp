#include "nbody/force_calculator.hpp"
#include "nbody/particle_data.hpp"
#include "nbody/types.hpp"
#include "particle_data_fixture.hpp"
#include "rapidcheck_float.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>

using namespace nbody;
using nbody::test::ScopedParticleData;
using nbody::test::setParticle;

// Unit Tests

TEST(ForceCalculationTest, TwoBodyAcceleration) {
  // Known analytical solution for the two-body problem
  Vec3 p1(0, 0, 0);
  Vec3 p2(1, 0, 0);
  float m2 = 1.0f;
  float G = 1.0f, eps = 0.0f;

  Vec3 acc = computeGravitationalAccelerationCPU(p1, p2, m2, G, eps);

  // Acceleration should point from p1 to p2
  EXPECT_GT(acc.x, 0);
  EXPECT_NEAR(acc.y, 0, 1e-6);
  EXPECT_NEAR(acc.z, 0, 1e-6);

  // Magnitude should be G * m2 / r^2 = 1 (test mass cancels)
  float expected_mag = G * m2 / 1.0f;  // r = 1
  EXPECT_NEAR(acc.length(), expected_mag, 1e-5);
}

TEST(ForceCalculationTest, SofteningPreventsInfinity) {
  Vec3 p1(0, 0, 0);
  Vec3 p2(0.001f, 0, 0);  // Very close
  float m2 = 1.0f;
  float G = 1.0f, eps = 0.1f;  // Softening

  Vec3 acc = computeGravitationalAccelerationCPU(p1, p2, m2, G, eps);

  // Acceleration should be finite
  EXPECT_TRUE(std::isfinite(acc.x));
  EXPECT_TRUE(std::isfinite(acc.y));
  EXPECT_TRUE(std::isfinite(acc.z));
}

TEST(ForceCalculationTest, AccelerationDirection) {
  Vec3 p1(0, 0, 0);
  Vec3 p2(1, 1, 1);
  float m2 = 1.0f;
  float G = 1.0f, eps = 0.01f;

  Vec3 acc = computeGravitationalAccelerationCPU(p1, p2, m2, G, eps);
  Vec3 direction = (p2 - p1).normalized();
  Vec3 acc_dir = acc.normalized();

  // Acceleration direction should match p1 -> p2 direction
  EXPECT_NEAR(acc_dir.x, direction.x, 1e-5);
  EXPECT_NEAR(acc_dir.y, direction.y, 1e-5);
  EXPECT_NEAR(acc_dir.z, direction.z, 1e-5);
}

// GPU-vs-CPU differential tests: the kernel must reproduce the CPU
// reference, not merely produce finite numbers.

TEST(DirectForceCalculatorTest, SingleParticleHasZeroAcceleration) {
  // N=1: there is nothing to interact with, so the acceleration must be
  // exactly zero (this pins down self-interaction exclusion).
  ScopedParticleData particles(1);
  setParticle(particles.host, 0, Vec3(1.0f, 2.0f, 3.0f), Vec3(), 5.0f);
  ParticleDataManager::copyToDevice(particles.device, particles.host);

  DirectForceCalculator calc(256);
  calc.setGravitationalConstant(1.0f);
  calc.setSofteningParameter(0.1f);
  calc.computeForces(&particles.device);

  ParticleDataManager::copyToHost(particles.host, particles.device);
  EXPECT_FLOAT_EQ(particles.host.acc_x[0], 0.0f);
  EXPECT_FLOAT_EQ(particles.host.acc_y[0], 0.0f);
  EXPECT_FLOAT_EQ(particles.host.acc_z[0], 0.0f);
}

TEST(DirectForceCalculatorTest, TwoBodyAccelerationsEqualAndOpposite) {
  // N=2 with equal masses: Newton's third law requires the accelerations to
  // be equal in magnitude and opposite in direction.
  ScopedParticleData particles(2);
  setParticle(particles.host, 0, Vec3(-0.5f, 0.0f, 0.0f), Vec3(), 1.0f);
  setParticle(particles.host, 1, Vec3(0.5f, 0.0f, 0.0f), Vec3(), 1.0f);
  ParticleDataManager::copyToDevice(particles.device, particles.host);

  DirectForceCalculator calc(256);
  calc.setGravitationalConstant(1.0f);
  calc.setSofteningParameter(0.01f);
  calc.computeForces(&particles.device);

  ParticleDataManager::copyToHost(particles.host, particles.device);
  EXPECT_NEAR(particles.host.acc_x[0], -particles.host.acc_x[1], 1e-5f);
  EXPECT_NEAR(particles.host.acc_y[0], -particles.host.acc_y[1], 1e-5f);
  EXPECT_NEAR(particles.host.acc_z[0], -particles.host.acc_z[1], 1e-5f);
  // And particle 0 must be pulled toward particle 1 (+x).
  EXPECT_GT(particles.host.acc_x[0], 0.0f);
}

TEST(DirectForceCalculatorTest, MatchesCPUReferencePerComponent) {
  // N=32 on a deterministic grid: every component of every particle's GPU
  // acceleration must match the CPU reference within float tolerance
  // (rsqrtf vs 1/sqrtf and summation order account for the slack).
  constexpr size_t N = 32;
  const float G = 1.0f;
  const float eps = 0.1f;

  ScopedParticleData particles(N);
  for (size_t i = 0; i < N; i++) {
    const Vec3 pos(static_cast<float>((i * 7) % 11) * 0.3f, static_cast<float>((i * 3) % 13) * 0.3f,
                   static_cast<float>((i * 5) % 9) * 0.3f);
    setParticle(particles.host, i, pos, Vec3(), 0.5f + 0.1f * static_cast<float>(i % 5));
  }
  ParticleDataManager::copyToDevice(particles.device, particles.host);

  DirectForceCalculator calc(256);
  calc.setGravitationalConstant(G);
  calc.setSofteningParameter(eps);
  calc.computeForces(&particles.device);

  ParticleDataManager::copyToHost(particles.host, particles.device);

  for (size_t i = 0; i < N; i++) {
    Vec3 ref(0, 0, 0);
    for (size_t j = 0; j < N; j++) {
      if (i == j)
        continue;
      const Vec3 pj(particles.host.pos_x[j], particles.host.pos_y[j], particles.host.pos_z[j]);
      const Vec3 pi(particles.host.pos_x[i], particles.host.pos_y[i], particles.host.pos_z[i]);
      ref += computeGravitationalAccelerationCPU(pi, pj, particles.host.mass[j], G, eps);
    }

    EXPECT_NEAR(particles.host.acc_x[i], ref.x, 1e-3f * (std::abs(ref.x) + 1e-3f))
        << "particle " << i << " acc_x";
    EXPECT_NEAR(particles.host.acc_y[i], ref.y, 1e-3f * (std::abs(ref.y) + 1e-3f))
        << "particle " << i << " acc_y";
    EXPECT_NEAR(particles.host.acc_z[i], ref.z, 1e-3f * (std::abs(ref.z) + 1e-3f))
        << "particle " << i << " acc_z";
  }
}

TEST(DirectForceCalculatorTest, ComputeForces) {
  // Create particles on device
  ScopedParticleData particles(100);

  SphericalDistParams params;
  params.center = Vec3(0, 0, 0);
  params.radius = 5.0f;
  ParticleInitializer::initSpherical(particles.host, params);

  ParticleDataManager::copyToDevice(particles.device, particles.host);

  // Compute forces
  DirectForceCalculator calc(256);
  calc.setGravitationalConstant(1.0f);
  calc.setSofteningParameter(0.1f);
  calc.computeForces(&particles.device);

  // Copy back and verify
  ParticleDataManager::copyToHost(particles.host, particles.device);

  // Check accelerations are finite
  for (size_t i = 0; i < particles.host.count; i++) {
    EXPECT_TRUE(std::isfinite(particles.host.acc_x[i]));
    EXPECT_TRUE(std::isfinite(particles.host.acc_y[i]));
    EXPECT_TRUE(std::isfinite(particles.host.acc_z[i]));
  }
}

// Property-Based Tests
// Feature: n-body-simulation, Property 1: Force Calculation Correctness

RC_GTEST_PROP(ForceCalculation, ForceMagnitudeCorrectness, ()) {
  // Feature: n-body-simulation, Property 1: Force Calculation Correctness
  // Validates: Requirements 2.1, 2.4, 2.5

  // Constructive generation: filtering eight arbitrary floats with RC_PRE
  // discards most test cases and risks "gave up" failures.
  const float m2 = *nbody::test::genFloatInRange(0.01f, 100.0f);
  const float r = *nbody::test::genFloatInRange(0.05f, 100.0f);

  const float G = 1.0f;
  const float eps = 0.01f;

  const Vec3 p1(0, 0, 0);
  const Vec3 p2(r, 0, 0);

  const Vec3 acc = computeGravitationalAccelerationCPU(p1, p2, m2, G, eps);

  // Property 1: softened acceleration magnitude is G·m2·r / (r² + ε²)^{3/2}.
  const float expected_mag = G * m2 * r / std::pow(r * r + eps * eps, 1.5f);
  const float actual_mag = acc.length();
  const float relative_error = std::abs(actual_mag - expected_mag) / expected_mag;

  RC_ASSERT(relative_error < 0.01f);  // < 1% error
}

RC_GTEST_PROP(ForceCalculation, ForceDirectionCorrectness, ()) {
  // Feature: n-body-simulation, Property 1: Force Calculation Correctness
  // Validates: Requirements 2.1, 2.4, 2.5

  const float r = *nbody::test::genFloatInRange(0.05f, 100.0f);
  const float dx = *nbody::test::genFloatInRange(-1.0f, 1.0f);
  const float dy = *nbody::test::genFloatInRange(-1.0f, 1.0f);
  const float dz = *nbody::test::genFloatInRange(-1.0f, 1.0f);
  RC_PRE(dx * dx + dy * dy + dz * dz > 0.01f);  // non-degenerate direction

  const Vec3 p1(0, 0, 0);
  const Vec3 direction(dx, dy, dz);
  const Vec3 p2 = direction.normalized() * r;

  const float G = 1.0f, eps = 0.01f;
  const float m2 = 1.0f;

  const Vec3 acc = computeGravitationalAccelerationCPU(p1, p2, m2, G, eps);

  // Property 2: Acceleration points from p1 toward p2
  const Vec3 expected_dir = (p2 - p1).normalized();
  const Vec3 actual_dir = acc.normalized();

  const float dot = expected_dir.dot(actual_dir);
  RC_ASSERT(dot > 0.999f);  // Directions should be nearly identical
}

RC_GTEST_PROP(ForceCalculation, SofteningFiniteness, ()) {
  // Feature: n-body-simulation, Property 1: Force Calculation Correctness
  // Validates: Requirements 2.1, 2.4, 2.5

  const float eps = *nbody::test::genFloatInRange(0.001f, 10.0f);
  const float r = *nbody::test::genFloatInRange(0.0f, 100.0f);

  const Vec3 p1(0, 0, 0);
  const Vec3 p2(r, 0, 0);

  const float G = 1.0f;
  const float m2 = 1.0f;

  const Vec3 acc = computeGravitationalAccelerationCPU(p1, p2, m2, G, eps);

  // Property 3: Acceleration remains finite even when r approaches zero
  RC_ASSERT(std::isfinite(acc.x));
  RC_ASSERT(std::isfinite(acc.y));
  RC_ASSERT(std::isfinite(acc.z));
  RC_ASSERT(acc.length() < 1e10f);  // Bounded
}
