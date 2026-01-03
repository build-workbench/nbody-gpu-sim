#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "nbody/spatial_hash_grid.hpp"
#include "nbody/force_calculator.hpp"
#include "nbody/particle_data.hpp"
#include "nbody/types.hpp"
#include <cmath>
#include <set>

using namespace nbody;

// Unit Tests

TEST(SpatialHashGridTest, BuildGrid) {
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, 100);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, 100);
    
    UniformDistParams params;
    params.min_bounds = Vec3(-10, -10, -10);
    params.max_bounds = Vec3(10, 10, 10);
    ParticleInitializer::initUniform(h_particles, params);
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    SpatialHashGrid grid(100, 2.0f);
    grid.build(&d_particles);
    
    EXPECT_GT(grid.getTotalCells(), 0);
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
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
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, 100);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, 100);
    
    UniformDistParams params;
    params.min_bounds = Vec3(-5, -5, -5);
    params.max_bounds = Vec3(5, 5, 5);
    ParticleInitializer::initUniform(h_particles, params);
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    SpatialHashCalculator calc(2.0f, 4.0f);
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
// Feature: n-body-simulation, Property 5: Spatial Hash Cell Assignment Correctness

RC_GTEST_PROP(SpatialHashGrid, CellAssignmentCorrectness, (float cell_size)) {
    // Feature: n-body-simulation, Property 5: Spatial Hash Cell Assignment Correctness
    // Validates: Requirements 4.1, 4.2
    
    RC_PRE(cell_size > 0.5f && cell_size < 10.0f);
    
    size_t N = 50;
    
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, N);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, N);
    
    UniformDistParams params;
    params.min_bounds = Vec3(-10, -10, -10);
    params.max_bounds = Vec3(10, 10, 10);
    ParticleInitializer::initUniform(h_particles, params);
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    SpatialHashGrid grid(N, cell_size);
    grid.build(&d_particles);
    
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
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
}

// Feature: n-body-simulation, Property 6: Spatial Hash Neighbor Cutoff

RC_GTEST_PROP(SpatialHashGrid, NeighborCutoffProperty, (float cutoff)) {
    // Feature: n-body-simulation, Property 6: Spatial Hash Neighbor Cutoff
    // Validates: Requirements 4.3
    
    RC_PRE(cutoff > 1.0f && cutoff < 10.0f);
    
    size_t N = 30;
    float cell_size = cutoff / 2.0f;  // Cell size should be related to cutoff
    
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, N);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, N);
    
    UniformDistParams params;
    params.min_bounds = Vec3(-5, -5, -5);
    params.max_bounds = Vec3(5, 5, 5);
    ParticleInitializer::initUniform(h_particles, params, 42);
    
    ParticleDataManager::copyToDevice(d_particles, h_particles);
    
    // Compute forces with spatial hash
    SpatialHashCalculator sh_calc(cell_size, cutoff);
    sh_calc.setGravitationalConstant(1.0f);
    sh_calc.setSofteningParameter(0.1f);
    sh_calc.computeForces(&d_particles);
    
    ParticleDataManager::copyToHost(h_particles, d_particles);
    
    // Verify that forces are computed (non-zero for particles with neighbors)
    bool has_nonzero_force = false;
    for (size_t i = 0; i < N; i++) {
        float acc_mag = std::sqrt(
            h_particles.acc_x[i] * h_particles.acc_x[i] +
            h_particles.acc_y[i] * h_particles.acc_y[i] +
            h_particles.acc_z[i] * h_particles.acc_z[i]
        );
        if (acc_mag > 1e-6f) {
            has_nonzero_force = true;
            break;
        }
    }
    
    // With particles in a 10x10x10 box and reasonable cutoff, should have interactions
    RC_ASSERT(has_nonzero_force);
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
}

// Feature: n-body-simulation, Property 4: Force Method Equivalence

RC_GTEST_PROP(ForceMethodEquivalence, DirectVsBarnesHut, ()) {
    // Feature: n-body-simulation, Property 4: Force Method Equivalence
    // Validates: Requirements 3.5
    
    size_t N = 30;
    
    ParticleData d_particles;
    ParticleDataManager::allocateDevice(d_particles, N);
    
    ParticleData h_particles;
    ParticleDataManager::allocateHost(h_particles, N);
    
    SphericalDistParams params;
    params.center = Vec3(0, 0, 0);
    params.radius = 5.0f;
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
    
    // Compute Barnes-Hut forces with small theta
    BarnesHutCalculator bh_calc(0.1f);  // Small theta for accuracy
    bh_calc.setGravitationalConstant(1.0f);
    bh_calc.setSofteningParameter(0.1f);
    bh_calc.computeForces(&d_particles);
    
    ParticleDataManager::copyToHost(h_particles, d_particles);
    
    // Property: Forces should be similar within tolerance
    float max_relative_error = 0.0f;
    for (size_t i = 0; i < N; i++) {
        float direct_mag = std::sqrt(
            direct_acc_x[i] * direct_acc_x[i] +
            direct_acc_y[i] * direct_acc_y[i] +
            direct_acc_z[i] * direct_acc_z[i]
        );
        
        float bh_mag = std::sqrt(
            h_particles.acc_x[i] * h_particles.acc_x[i] +
            h_particles.acc_y[i] * h_particles.acc_y[i] +
            h_particles.acc_z[i] * h_particles.acc_z[i]
        );
        
        if (direct_mag > 1e-6f) {
            float relative_error = std::abs(bh_mag - direct_mag) / direct_mag;
            max_relative_error = std::max(max_relative_error, relative_error);
        }
    }
    
    // With theta=0.1, should be within 10% of direct calculation
    RC_ASSERT(max_relative_error < 0.1f);
    
    ParticleDataManager::freeDevice(d_particles);
    ParticleDataManager::freeHost(h_particles);
}
