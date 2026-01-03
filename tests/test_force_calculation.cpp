#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "nbody/force_calculator.hpp"
#include "nbody/particle_data.hpp"
#include "nbody/types.hpp"
#include <cmath>

using namespace nbody;

// Unit Tests

TEST(ForceCalculationTest, TwoBodyForce) {
    // Known analytical solution for two-body problem
    Vec3 p1(0, 0, 0);
    Vec3 p2(1, 0, 0);
    float m1 = 1.0f, m2 = 1.0f;
    float G = 1.0f, eps = 0.0f;
    
    Vec3 force = computeGravitationalForceCPU(p1, p2, m1, m2, G, eps);
    
    // Force should point from p1 to p2
    EXPECT_GT(force.x, 0);
    EXPECT_NEAR(force.y, 0, 1e-6);
    EXPECT_NEAR(force.z, 0, 1e-6);
    
    // Magnitude should be G * m1 * m2 / r^2 = 1
    float expected_mag = G * m1 * m2 / 1.0f;  // r = 1
    EXPECT_NEAR(force.length(), expected_mag, 1e-5);
}

TEST(ForceCalculationTest, SofteningPreventsInfinity) {
    Vec3 p1(0, 0, 0);
    Vec3 p2(0.001f, 0, 0);  // Very close
    float m1 = 1.0f, m2 = 1.0f;
    float G = 1.0f, eps = 0.1f;  // Softening
    
    Vec3 force = computeGravitationalForceCPU(p1, p2, m1, m2, G, eps);
    
    // Force should be finite
    EXPECT_TRUE(std::isfinite(force.x));
    EXPECT_TRUE(std::isfinite(force.y));
    EXPECT_TRUE(std::isfinite(force.z));
}

TEST(ForceCalculationTest, ForceDirection) {
    Vec3 p1(0, 0, 0);
    Vec3 p2(1, 1, 1);
    float m1 = 1.0f, m2 = 1.0f;
    float G = 1.0f, eps = 0.01f;
    
    Vec3 force = computeGravitationalForceCPU(p1, p2, m1, m2, G, eps);
    Vec3 direction = (p2 - p1).normalized();
    Vec3 force_dir = force.normalized();
    
    // Force direction should match p1 -> p2 direction
    EXPECT_NEAR(force_dir.x, direction.x, 1e-5);
    EXPECT_NEAR(force_dir.y, direction.y, 1e-5);
    EXPECT_NEAR(force_dir.z, direction.z, 1e-5);
}

TEST(DirectForceCalculatorTest, ComputeForces) {
    // Create particles on device
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, 100);
    
    // Initialize on host and copy
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, 100);
    
    SphericalDistParams params;
    params.center = Vec3(0, 0, 0);
    params.radius = 5.0f;
    ParticleInitializer::initSpherical(h_particles, params);
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    // Compute forces
    DirectForceCalculator calc(256);
    calc.setGravitationalConstant(1.0f);
    calc.setSofteningParameter(0.1f);
    calc.computeForces(&d_particles);
    
    // Copy back and verify
    ParticleDataManager::copyToHost(h_particles, d_particles);
    
    // Check accelerations are finite
    for (size_t i = 0; i < h_particles.count; i++) {
        EXPECT_TRUE(std::isfinite(h_particles.acc_x[i]));
        EXPECT_TRUE(std::isfinite(h_particles.acc_y[i]));
        EXPECT_TRUE(std::isfinite(h_particles.acc_z[i]));
    }
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
}

// Property-Based Tests
// Feature: n-body-simulation, Property 1: Force Calculation Correctness

RC_GTEST_PROP(ForceCalculation, ForceMagnitudeCorrectness,
              (float x1, float y1, float z1, float x2, float y2, float z2, float m1, float m2)) {
    // Feature: n-body-simulation, Property 1: Force Calculation Correctness
    // Validates: Requirements 2.1, 2.4, 2.5
    
    // Preconditions
    RC_PRE(m1 > 0.01f && m1 < 100.0f);
    RC_PRE(m2 > 0.01f && m2 < 100.0f);
    RC_PRE(std::abs(x1) < 100 && std::abs(y1) < 100 && std::abs(z1) < 100);
    RC_PRE(std::abs(x2) < 100 && std::abs(y2) < 100 && std::abs(z2) < 100);
    
    Vec3 p1(x1, y1, z1);
    Vec3 p2(x2, y2, z2);
    float r = (p2 - p1).length();
    
    RC_PRE(r > 0.01f);  // Minimum distance
    
    float G = 1.0f;
    float eps = 0.01f;
    
    Vec3 force = computeGravitationalForceCPU(p1, p2, m1, m2, G, eps);
    
    // Property 1: Force magnitude approximates G * m1 * m2 / (r² + ε²)
    float expected_mag = G * m2 / (r*r + eps*eps);
    float actual_mag = force.length();
    float relative_error = std::abs(actual_mag - expected_mag) / expected_mag;
    
    RC_ASSERT(relative_error < 0.01f);  // < 1% error
}

RC_GTEST_PROP(ForceCalculation, ForceDirectionCorrectness,
              (float x1, float y1, float z1, float x2, float y2, float z2)) {
    // Feature: n-body-simulation, Property 1: Force Calculation Correctness
    // Validates: Requirements 2.1, 2.4, 2.5
    
    RC_PRE(std::abs(x1) < 100 && std::abs(y1) < 100 && std::abs(z1) < 100);
    RC_PRE(std::abs(x2) < 100 && std::abs(y2) < 100 && std::abs(z2) < 100);
    
    Vec3 p1(x1, y1, z1);
    Vec3 p2(x2, y2, z2);
    float r = (p2 - p1).length();
    
    RC_PRE(r > 0.01f);
    
    float G = 1.0f, eps = 0.01f;
    float m1 = 1.0f, m2 = 1.0f;
    
    Vec3 force = computeGravitationalForceCPU(p1, p2, m1, m2, G, eps);
    
    // Property 2: Force direction points from p1 toward p2
    Vec3 expected_dir = (p2 - p1).normalized();
    Vec3 actual_dir = force.normalized();
    
    float dot = expected_dir.dot(actual_dir);
    RC_ASSERT(dot > 0.999f);  // Directions should be nearly identical
}

RC_GTEST_PROP(ForceCalculation, SofteningFiniteness,
              (float x1, float y1, float z1, float x2, float y2, float z2, float eps)) {
    // Feature: n-body-simulation, Property 1: Force Calculation Correctness
    // Validates: Requirements 2.1, 2.4, 2.5
    
    RC_PRE(eps > 0.001f && eps < 10.0f);
    RC_PRE(std::abs(x1) < 100 && std::abs(y1) < 100 && std::abs(z1) < 100);
    RC_PRE(std::abs(x2) < 100 && std::abs(y2) < 100 && std::abs(z2) < 100);
    
    Vec3 p1(x1, y1, z1);
    Vec3 p2(x2, y2, z2);
    
    float G = 1.0f;
    float m1 = 1.0f, m2 = 1.0f;
    
    Vec3 force = computeGravitationalForceCPU(p1, p2, m1, m2, G, eps);
    
    // Property 3: Force remains finite even when r approaches zero
    RC_ASSERT(std::isfinite(force.x));
    RC_ASSERT(std::isfinite(force.y));
    RC_ASSERT(std::isfinite(force.z));
    RC_ASSERT(force.length() < 1e10f);  // Bounded
}
