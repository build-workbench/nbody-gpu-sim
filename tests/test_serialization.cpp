#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "nbody/serialization.hpp"
#include "nbody/particle_system.hpp"
#include "nbody/types.hpp"
#include <sstream>
#include <cmath>

using namespace nbody;

// Unit Tests

TEST(SerializationTest, SaveAndLoadState) {
    SimulationState original;
    original.particle_count = 10;
    original.simulation_time = 1.5f;
    original.dt = 0.001f;
    original.G = 1.0f;
    original.softening = 0.01f;
    original.force_method = ForceMethod::DIRECT_N2;
    
    original.pos_x.resize(10);
    original.pos_y.resize(10);
    original.pos_z.resize(10);
    original.vel_x.resize(10);
    original.vel_y.resize(10);
    original.vel_z.resize(10);
    original.mass.resize(10);
    
    for (int i = 0; i < 10; i++) {
        original.pos_x[i] = static_cast<float>(i);
        original.pos_y[i] = static_cast<float>(i * 2);
        original.pos_z[i] = static_cast<float>(i * 3);
        original.vel_x[i] = 0.1f * i;
        original.vel_y[i] = 0.2f * i;
        original.vel_z[i] = 0.3f * i;
        original.mass[i] = 1.0f;
    }
    
    // Save to stream
    std::stringstream ss;
    Serializer::save(ss, original);
    
    // Load from stream
    ss.seekg(0);
    SimulationState loaded = Serializer::load(ss);
    
    EXPECT_EQ(loaded.particle_count, original.particle_count);
    EXPECT_NEAR(loaded.simulation_time, original.simulation_time, 1e-6);
    EXPECT_NEAR(loaded.dt, original.dt, 1e-6);
    EXPECT_NEAR(loaded.G, original.G, 1e-6);
    EXPECT_NEAR(loaded.softening, original.softening, 1e-6);
    EXPECT_EQ(loaded.force_method, original.force_method);
    
    for (size_t i = 0; i < original.particle_count; i++) {
        EXPECT_NEAR(loaded.pos_x[i], original.pos_x[i], 1e-6);
        EXPECT_NEAR(loaded.pos_y[i], original.pos_y[i], 1e-6);
        EXPECT_NEAR(loaded.pos_z[i], original.pos_z[i], 1e-6);
        EXPECT_NEAR(loaded.vel_x[i], original.vel_x[i], 1e-6);
        EXPECT_NEAR(loaded.vel_y[i], original.vel_y[i], 1e-6);
        EXPECT_NEAR(loaded.vel_z[i], original.vel_z[i], 1e-6);
        EXPECT_NEAR(loaded.mass[i], original.mass[i], 1e-6);
    }
}

TEST(SerializationTest, ValidateStream) {
    SimulationState state;
    state.particle_count = 5;
    state.simulation_time = 0.0f;
    state.dt = 0.001f;
    state.G = 1.0f;
    state.softening = 0.01f;
    state.force_method = ForceMethod::BARNES_HUT;
    
    state.pos_x.resize(5, 0.0f);
    state.pos_y.resize(5, 0.0f);
    state.pos_z.resize(5, 0.0f);
    state.vel_x.resize(5, 0.0f);
    state.vel_y.resize(5, 0.0f);
    state.vel_z.resize(5, 0.0f);
    state.mass.resize(5, 1.0f);
    
    std::stringstream ss;
    Serializer::save(ss, state);
    
    ss.seekg(0);
    EXPECT_TRUE(Serializer::validateStream(ss));
}

TEST(SerializationTest, InvalidMagicNumber) {
    std::stringstream ss;
    uint32_t bad_magic = 0x12345678;
    ss.write(reinterpret_cast<const char*>(&bad_magic), sizeof(bad_magic));
    
    ss.seekg(0);
    EXPECT_FALSE(Serializer::validateStream(ss));
}

// Property-Based Tests
// Feature: n-body-simulation, Property 12: Save/Load State Round-Trip

RC_GTEST_PROP(Serialization, RoundTripPreservesState,
              (size_t particle_count, float sim_time, float dt, float G, float softening)) {
    // Feature: n-body-simulation, Property 12: Save/Load State Round-Trip
    // Validates: Requirements 8.4
    
    RC_PRE(particle_count > 0 && particle_count <= 100);
    RC_PRE(sim_time >= 0.0f && sim_time < 1000.0f);
    RC_PRE(dt > 0.0001f && dt < 1.0f);
    RC_PRE(G > 0.0f && G < 100.0f);
    RC_PRE(softening >= 0.0f && softening < 10.0f);
    RC_PRE(std::isfinite(sim_time) && std::isfinite(dt) && std::isfinite(G) && std::isfinite(softening));
    
    SimulationState original;
    original.particle_count = particle_count;
    original.simulation_time = sim_time;
    original.dt = dt;
    original.G = G;
    original.softening = softening;
    original.force_method = ForceMethod::DIRECT_N2;
    
    original.pos_x.resize(particle_count);
    original.pos_y.resize(particle_count);
    original.pos_z.resize(particle_count);
    original.vel_x.resize(particle_count);
    original.vel_y.resize(particle_count);
    original.vel_z.resize(particle_count);
    original.mass.resize(particle_count);
    
    // Generate random particle data
    for (size_t i = 0; i < particle_count; i++) {
        original.pos_x[i] = *rc::gen::inRange(-100.0f, 100.0f);
        original.pos_y[i] = *rc::gen::inRange(-100.0f, 100.0f);
        original.pos_z[i] = *rc::gen::inRange(-100.0f, 100.0f);
        original.vel_x[i] = *rc::gen::inRange(-10.0f, 10.0f);
        original.vel_y[i] = *rc::gen::inRange(-10.0f, 10.0f);
        original.vel_z[i] = *rc::gen::inRange(-10.0f, 10.0f);
        original.mass[i] = *rc::gen::inRange(0.1f, 10.0f);
    }
    
    // Round-trip through serialization
    std::stringstream ss;
    Serializer::save(ss, original);
    ss.seekg(0);
    SimulationState loaded = Serializer::load(ss);
    
    // Property: Loaded state equals original state
    RC_ASSERT(loaded == original);
}

// Feature: n-body-simulation, Property 11: Pause/Resume State Preservation

RC_GTEST_PROP(SimulationControl, PauseResumePreservesState, ()) {
    // Feature: n-body-simulation, Property 11: Pause/Resume State Preservation
    // Validates: Requirements 8.1
    
    // Create a simulation
    SimulationConfig config;
    config.particle_count = 50;
    config.init_distribution = InitDistribution::SPHERICAL;
    config.force_method = ForceMethod::DIRECT_N2;
    config.dt = 0.001f;
    
    ParticleSystem system;
    system.initialize(config);
    
    // Run a few steps
    for (int i = 0; i < 10; i++) {
        system.update(config.dt);
    }
    
    // Get state before pause
    SimulationState state_before = system.getState();
    
    // Pause
    system.pause();
    RC_ASSERT(system.isPaused());
    
    // Try to update (should not change state)
    for (int i = 0; i < 10; i++) {
        system.update(config.dt);
    }
    
    // Get state after pause
    SimulationState state_after = system.getState();
    
    // Property: State should be unchanged during pause
    RC_ASSERT(state_before == state_after);
    
    // Resume and verify simulation continues
    system.resume();
    RC_ASSERT(!system.isPaused());
    
    system.update(config.dt);
    SimulationState state_resumed = system.getState();
    
    // State should have changed after resume
    RC_ASSERT(!(state_resumed == state_after));
}
