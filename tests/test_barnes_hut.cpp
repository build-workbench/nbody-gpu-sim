#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "nbody/barnes_hut_tree.hpp"
#include "nbody/force_calculator.hpp"
#include "nbody/particle_data.hpp"
#include "nbody/types.hpp"
#include <cmath>
#include <numeric>

using namespace nbody;

// Unit Tests

TEST(BarnesHutTreeTest, BuildTree) {
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, 100);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, 100);
    
    SphericalDistParams params;
    params.center = Vec3(0, 0, 0);
    params.radius = 10.0f;
    ParticleInitializer::initSpherical(h_particles, params);
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    BarnesHutTree tree(100);
    tree.build(&d_particles);
    
    EXPECT_GT(tree.getNodeCount(), 0);
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
}

TEST(BarnesHutTreeTest, MassConservation) {
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, 50);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, 50);
    
    SphericalDistParams params;
    params.center = Vec3(0, 0, 0);
    params.radius = 5.0f;
    params.min_mass = 1.0f;
    params.max_mass = 1.0f;  // Uniform mass for easy verification
    ParticleInitializer::initSpherical(h_particles, params);
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    BarnesHutTree tree(50);
    tree.build(&d_particles);
    tree.copyNodesToHost();
    
    EXPECT_TRUE(tree.verifyMassConservation(&h_particles));
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
}

TEST(BarnesHutCalculatorTest, ComputeForces) {
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, 100);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, 100);
    
    SphericalDistParams params;
    params.center = Vec3(0, 0, 0);
    params.radius = 10.0f;
    ParticleInitializer::initSpherical(h_particles, params);
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    BarnesHutCalculator calc(0.5f);
    calc.setGravitationalConstant(1.0f);
    calc.setSofteningParameter(0.1f);
    calc.computeForces(&d_particles);
    
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
// Feature: n-body-simulation, Property 2: Barnes-Hut Tree Structure Correctness

RC_GTEST_PROP(BarnesHutTree, TreeContainsAllParticles, (int seed)) {
    // Feature: n-body-simulation, Property 2: Barnes-Hut Tree Structure Correctness
    // Validates: Requirements 3.1, 3.2
    
    size_t N = 50;  // Small for testing
    
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, N);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, N);
    
    SphericalDistParams params;
    params.center = Vec3(0, 0, 0);
    params.radius = 10.0f;
    ParticleInitializer::initSpherical(h_particles, params, seed);
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    BarnesHutTree tree(N);
    tree.build(&d_particles);
    tree.copyNodesToHost();
    
    // Property: Tree contains exactly N particles (mass conservation)
    RC_ASSERT(tree.verifyMassConservation(&h_particles));
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
}

// Feature: n-body-simulation, Property 3: Barnes-Hut Approximation Convergence

RC_GTEST_PROP(BarnesHutTree, ApproximationConvergence, ()) {
    // Feature: n-body-simulation, Property 3: Barnes-Hut Approximation Convergence
    // Validates: Requirements 3.3
    
    size_t N = 50;
    
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, N);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, N);
    
    SphericalDistParams params;
    params.center = Vec3(0, 0, 0);
    params.radius = 10.0f;
    ParticleInitializer::initSpherical(h_particles, params, 42);
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    // Compute direct forces
    DirectForceCalculator direct_calc;
    direct_calc.setGravitationalConstant(1.0f);
    direct_calc.setSofteningParameter(0.1f);
    direct_calc.computeForces(&d_particles);
    
    ParticleDataManager::copyToHost(h_particles, d_particles);
    std::vector<float> direct_acc_x(N), direct_acc_y(N), direct_acc_z(N);
    for (size_t i = 0; i < N; i++) {
        direct_acc_x[i] = h_particles.acc_x[i];
        direct_acc_y[i] = h_particles.acc_y[i];
        direct_acc_z[i] = h_particles.acc_z[i];
    }
    
    // Compute Barnes-Hut forces with different theta values
    float theta1 = 0.8f;
    float theta2 = 0.3f;  // Smaller theta = more accurate
    
    BarnesHutCalculator bh_calc1(theta1);
    bh_calc1.setGravitationalConstant(1.0f);
    bh_calc1.setSofteningParameter(0.1f);
    bh_calc1.computeForces(&d_particles);
    
    ParticleDataManager::copyToHost(h_particles, d_particles);
    float error1 = 0.0f;
    for (size_t i = 0; i < N; i++) {
        float dx = h_particles.acc_x[i] - direct_acc_x[i];
        float dy = h_particles.acc_y[i] - direct_acc_y[i];
        float dz = h_particles.acc_z[i] - direct_acc_z[i];
        error1 += std::sqrt(dx*dx + dy*dy + dz*dz);
    }
    
    BarnesHutCalculator bh_calc2(theta2);
    bh_calc2.setGravitationalConstant(1.0f);
    bh_calc2.setSofteningParameter(0.1f);
    bh_calc2.computeForces(&d_particles);
    
    ParticleDataManager::copyToHost(h_particles, d_particles);
    float error2 = 0.0f;
    for (size_t i = 0; i < N; i++) {
        float dx = h_particles.acc_x[i] - direct_acc_x[i];
        float dy = h_particles.acc_y[i] - direct_acc_y[i];
        float dz = h_particles.acc_z[i] - direct_acc_z[i];
        error2 += std::sqrt(dx*dx + dy*dy + dz*dz);
    }
    
    // Property: Smaller theta should give smaller error
    RC_ASSERT(error2 <= error1 * 1.1f);  // Allow small tolerance
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
}
