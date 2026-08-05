/**
 * @file test_simulation_control.cpp
 * @brief Pause/resume state-preservation tests for ParticleSystem.
 *
 * These tests drive a real ParticleSystem and therefore require the CUDA
 * backend (ParticleDataManager is implemented in src/cuda/particle_init.cu).
 * They are built only into the CUDA test binary; the pure-Serializer tests
 * live in test_serialization.cpp and run headless.
 */

#include "nbody/particle_system.hpp"
#include "nbody/simulation_state.hpp"
#include "nbody/types.hpp"
#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

using namespace nbody;

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
