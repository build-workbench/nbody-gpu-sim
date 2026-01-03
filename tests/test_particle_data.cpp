#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "nbody/particle_data.hpp"
#include "nbody/types.hpp"
#include <cmath>

using namespace nbody;

// Unit Tests

TEST(ParticleDataTest, AllocateAndFreeDevice) {
    ParticleData data;
    ParticleDataManager::allocateDevice(data, 1000);
    
    EXPECT_NE(data.pos_x, nullptr);
    EXPECT_NE(data.pos_y, nullptr);
    EXPECT_NE(data.pos_z, nullptr);
    EXPECT_NE(data.vel_x, nullptr);
    EXPECT_NE(data.vel_y, nullptr);
    EXPECT_NE(data.vel_z, nullptr);
    EXPECT_NE(data.mass, nullptr);
    EXPECT_EQ(data.count, 1000);
    
    ParticleDataManager::freeDevice(data);
    EXPECT_EQ(data.count, 0);
}

TEST(ParticleDataTest, AllocateAndFreeHost) {
    ParticleData data;
    ParticleDataManager::allocateHost(data, 500);
    
    EXPECT_NE(data.pos_x, nullptr);
    EXPECT_EQ(data.count, 500);
    
    ParticleDataManager::freeHost(data);
    EXPECT_EQ(data.count, 0);
}

TEST(ParticleInitializerTest, UniformDistribution) {
    ParticleData data;
    ParticleDataManager::allocateHost(data, 100);
    
    UniformDistParams params;
    params.min_bounds = Vec3(-5, -5, -5);
    params.max_bounds = Vec3(5, 5, 5);
    params.min_mass = 1.0f;
    params.max_mass = 2.0f;
    
    ParticleInitializer::initUniform(data, params, 42);
    
    // Check all particles are within bounds
    for (size_t i = 0; i < data.count; i++) {
        EXPECT_GE(data.pos_x[i], params.min_bounds.x);
        EXPECT_LE(data.pos_x[i], params.max_bounds.x);
        EXPECT_GE(data.pos_y[i], params.min_bounds.y);
        EXPECT_LE(data.pos_y[i], params.max_bounds.y);
        EXPECT_GE(data.pos_z[i], params.min_bounds.z);
        EXPECT_LE(data.pos_z[i], params.max_bounds.z);
        EXPECT_GE(data.mass[i], params.min_mass);
        EXPECT_LE(data.mass[i], params.max_mass);
    }
    
    ParticleDataManager::freeHost(data);
}

TEST(ParticleInitializerTest, SphericalDistribution) {
    ParticleData data;
    ParticleDataManager::allocateHost(data, 100);
    
    SphericalDistParams params;
    params.center = Vec3(0, 0, 0);
    params.radius = 10.0f;
    
    ParticleInitializer::initSpherical(data, params, 42);
    
    // Check all particles are within sphere
    for (size_t i = 0; i < data.count; i++) {
        float dx = data.pos_x[i] - params.center.x;
        float dy = data.pos_y[i] - params.center.y;
        float dz = data.pos_z[i] - params.center.z;
        float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
        EXPECT_LE(dist, params.radius + 0.001f);
    }
    
    ParticleDataManager::freeHost(data);
}

TEST(ParticleInitializerTest, DiskDistribution) {
    ParticleData data;
    ParticleDataManager::allocateHost(data, 100);
    
    DiskDistParams params;
    params.center = Vec3(0, 0, 0);
    params.radius = 10.0f;
    params.thickness = 1.0f;
    
    ParticleInitializer::initDisk(data, params, 42);
    
    // Check all particles are within disk
    for (size_t i = 0; i < data.count; i++) {
        float dx = data.pos_x[i] - params.center.x;
        float dy = data.pos_y[i] - params.center.y;
        float dz = data.pos_z[i] - params.center.z;
        float radial_dist = std::sqrt(dx*dx + dy*dy);
        
        EXPECT_LE(radial_dist, params.radius + 0.001f);
        EXPECT_LE(std::abs(dz), params.thickness / 2.0f + 0.001f);
    }
    
    ParticleDataManager::freeHost(data);
}

// Property-Based Tests
// Feature: n-body-simulation, Property 14: Particle Distribution Bounds

RC_GTEST_PROP(ParticleDistribution, UniformBoundsProperty,
              (float min_x, float max_x, float min_y, float max_y, float min_z, float max_z)) {
    // Feature: n-body-simulation, Property 14: Particle Distribution Bounds
    // Validates: Requirements 1.3
    
    // Ensure valid bounds
    RC_PRE(max_x > min_x && max_y > min_y && max_z > min_z);
    RC_PRE(std::abs(min_x) < 100 && std::abs(max_x) < 100);
    RC_PRE(std::abs(min_y) < 100 && std::abs(max_y) < 100);
    RC_PRE(std::abs(min_z) < 100 && std::abs(max_z) < 100);
    
    ParticleData data;
    ParticleDataManager::allocateHost(data, 100);
    
    UniformDistParams params;
    params.min_bounds = Vec3(min_x, min_y, min_z);
    params.max_bounds = Vec3(max_x, max_y, max_z);
    
    ParticleInitializer::initUniform(data, params);
    
    // Property: All particles within specified bounding box
    for (size_t i = 0; i < data.count; i++) {
        RC_ASSERT(data.pos_x[i] >= min_x && data.pos_x[i] <= max_x);
        RC_ASSERT(data.pos_y[i] >= min_y && data.pos_y[i] <= max_y);
        RC_ASSERT(data.pos_z[i] >= min_z && data.pos_z[i] <= max_z);
    }
    
    ParticleDataManager::freeHost(data);
}

RC_GTEST_PROP(ParticleDistribution, SphericalBoundsProperty,
              (float cx, float cy, float cz, float radius)) {
    // Feature: n-body-simulation, Property 14: Particle Distribution Bounds
    // Validates: Requirements 1.3
    
    RC_PRE(radius > 0.1f && radius < 100.0f);
    RC_PRE(std::abs(cx) < 100 && std::abs(cy) < 100 && std::abs(cz) < 100);
    
    ParticleData data;
    ParticleDataManager::allocateHost(data, 100);
    
    SphericalDistParams params;
    params.center = Vec3(cx, cy, cz);
    params.radius = radius;
    
    ParticleInitializer::initSpherical(data, params);
    
    // Property: All particles within specified radius from center
    for (size_t i = 0; i < data.count; i++) {
        float dx = data.pos_x[i] - cx;
        float dy = data.pos_y[i] - cy;
        float dz = data.pos_z[i] - cz;
        float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
        RC_ASSERT(dist <= radius + 0.01f);
    }
    
    ParticleDataManager::freeHost(data);
}

RC_GTEST_PROP(ParticleDistribution, DiskBoundsProperty,
              (float cx, float cy, float cz, float radius, float thickness)) {
    // Feature: n-body-simulation, Property 14: Particle Distribution Bounds
    // Validates: Requirements 1.3
    
    RC_PRE(radius > 0.1f && radius < 100.0f);
    RC_PRE(thickness > 0.01f && thickness < 10.0f);
    RC_PRE(std::abs(cx) < 100 && std::abs(cy) < 100 && std::abs(cz) < 100);
    
    ParticleData data;
    ParticleDataManager::allocateHost(data, 100);
    
    DiskDistParams params;
    params.center = Vec3(cx, cy, cz);
    params.radius = radius;
    params.thickness = thickness;
    
    ParticleInitializer::initDisk(data, params);
    
    // Property: All particles within specified radius and thickness
    for (size_t i = 0; i < data.count; i++) {
        float dx = data.pos_x[i] - cx;
        float dy = data.pos_y[i] - cy;
        float dz = data.pos_z[i] - cz;
        float radial_dist = std::sqrt(dx*dx + dy*dy);
        
        RC_ASSERT(radial_dist <= radius + 0.01f);
        RC_ASSERT(std::abs(dz) <= thickness / 2.0f + 0.01f);
    }
    
    ParticleDataManager::freeHost(data);
}
