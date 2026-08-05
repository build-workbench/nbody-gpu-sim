/**
 * @file test_serialization.cpp
 * @brief Tests for the binary checkpoint format (.nbody).
 *
 * The Serializer class provides fast binary checkpoint serialization
 * for pause/resume and checkpoint/restart operations.
 *
 * These tests verify:
 * - Basic save/load round-trip correctness
 * - Stream validation and format detection
 * - Property-based testing for state preservation
 *
 * Pure-Serializer tests only: they run in the headless core test binary.
 * Pause/resume tests that drive a real ParticleSystem (CUDA backend) live
 * in test_simulation_control.cpp.
 */

#include "nbody/error_handling.hpp"
#include "nbody/serialization.hpp"
#include "nbody/types.hpp"
#include "rapidcheck_float.hpp"
#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include <sstream>

using namespace nbody;
using nbody::test::genFloatInRange;

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

namespace {
// Build a small, fully populated state for error-path tests.
SimulationState makeTestState(size_t n) {
  SimulationState state;
  state.particle_count = n;
  state.simulation_time = 1.0f;
  state.dt = 0.001f;
  state.G = 1.0f;
  state.softening = 0.1f;
  state.force_method = ForceMethod::BARNES_HUT;
  state.pos_x.resize(n, 0.1f);
  state.pos_y.resize(n, 0.2f);
  state.pos_z.resize(n, 0.3f);
  state.vel_x.resize(n, 0.0f);
  state.vel_y.resize(n, 0.0f);
  state.vel_z.resize(n, 0.0f);
  state.mass.resize(n, 1.0f);
  return state;
}

// Write a header with the given overrides onto a fresh stream.
std::stringstream makeHeaderStream(uint64_t particle_count, uint32_t version,
                                   uint32_t force_method) {
  FileHeader header{};  // zero-initializes padding too
  header.magic = NBODY_MAGIC;
  header.version = version;
  header.particle_count = particle_count;
  header.force_method = force_method;
  std::stringstream ss;
  ss.write(reinterpret_cast<const char*>(&header), sizeof(header));
  ss.seekg(0);
  return ss;
}
}  // namespace

TEST(SerializationTest, RejectsTruncatedHeader) {
  std::stringstream ss;
  uint32_t magic = NBODY_MAGIC;
  ss.write(reinterpret_cast<const char*>(&magic), sizeof(magic));  // 4 of 56 bytes
  ss.seekg(0);
  EXPECT_THROW(Serializer::load(ss), ValidationException);
}

TEST(SerializationTest, RejectsTruncatedParticleData) {
  std::stringstream full;
  Serializer::save(full, makeTestState(8));
  const std::string bytes = full.str();
  ASSERT_GT(bytes.size(), sizeof(FileHeader) + sizeof(float));

  std::stringstream truncated;
  truncated.write(bytes.data(), static_cast<std::streamsize>(bytes.size() - sizeof(float)));
  truncated.seekg(0);
  EXPECT_THROW(Serializer::load(truncated), ValidationException);
}

TEST(SerializationTest, RejectsUnsupportedVersion) {
  std::stringstream ss = makeHeaderStream(1, NBODY_VERSION + 1, 0);
  EXPECT_THROW(Serializer::load(ss), ValidationException);
}

TEST(SerializationTest, RejectsExcessiveParticleCount) {
  std::stringstream ss = makeHeaderStream(MAX_PARTICLE_COUNT + 1, NBODY_VERSION, 0);
  EXPECT_THROW(Serializer::load(ss), ValidationException);
}

TEST(SerializationTest, RejectsInvalidForceMethod) {
  std::stringstream ss = makeHeaderStream(0, NBODY_VERSION, 99);
  EXPECT_THROW(Serializer::load(ss), ValidationException);
}

TEST(SerializationTest, LoadMissingFileThrowsIOException) {
  EXPECT_THROW(Serializer::load("definitely_missing_file_9f3a.nbody"), IOException);
}

TEST(SerializationTest, SaveToUnwritablePathThrowsIOException) {
  const SimulationState state = makeTestState(1);
  EXPECT_THROW(Serializer::save("/nonexistent_dir_9f3a/state.nbody", state), IOException);
}

TEST(SerializationTest, RoundTripsAllForceMethods) {
  for (const ForceMethod method :
       {ForceMethod::DIRECT_N2, ForceMethod::BARNES_HUT, ForceMethod::SPATIAL_HASH}) {
    SimulationState state = makeTestState(4);
    state.force_method = method;

    std::stringstream ss;
    Serializer::save(ss, state);
    ss.seekg(0);
    const SimulationState loaded = Serializer::load(ss);

    EXPECT_EQ(loaded.force_method, method);
    EXPECT_TRUE(loaded == state);
  }
}

TEST(SimulationStateTest, EqualForIdenticalStates) {
  const SimulationState a = makeTestState(6);
  const SimulationState b = a;
  EXPECT_TRUE(a == b);
}

TEST(SimulationStateTest, NotEqualWhenVectorShorterThanParticleCount) {
  // Malformed states (vectors shorter than particle_count) must compare
  // unequal instead of reading out of bounds.
  SimulationState a = makeTestState(6);
  a.pos_x.resize(2);
  const SimulationState b = a;
  EXPECT_FALSE(a == b);
}

// Property-Based Tests
// Feature: n-body-simulation, Property 12: Save/Load State Round-Trip

RC_GTEST_PROP(Serialization, RoundTripPreservesState, ()) {
  // Feature: n-body-simulation, Property 12: Save/Load State Round-Trip
  // Validates: Requirements 8.4

  // Generate values directly in the valid ranges; RC_PRE-filtering full-range
  // arbitrary values would (almost) never hit and the property would give up.
  const size_t particle_count = *rc::gen::inRange<size_t>(1, 101);
  const float sim_time = *genFloatInRange(0.0f, 1000.0f);
  const float dt = *genFloatInRange(0.001f, 1.0f);
  const float G = *genFloatInRange(0.001f, 100.0f);
  const float softening = *genFloatInRange(0.0f, 10.0f);

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
    original.pos_x[i] = *genFloatInRange(-100.0f, 100.0f);
    original.pos_y[i] = *genFloatInRange(-100.0f, 100.0f);
    original.pos_z[i] = *genFloatInRange(-100.0f, 100.0f);
    original.vel_x[i] = *genFloatInRange(-10.0f, 10.0f);
    original.vel_y[i] = *genFloatInRange(-10.0f, 10.0f);
    original.vel_z[i] = *genFloatInRange(-10.0f, 10.0f);
    original.mass[i] = *genFloatInRange(0.1f, 10.0f);
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
// moved to test_simulation_control.cpp (requires the CUDA backend).

// Test checkpoint round-trip with various particle counts
// Verifies the private .nbody format continues to work correctly

class ParticleCountTestFixture : public ::testing::TestWithParam<size_t> {};

TEST_P(ParticleCountTestFixture, CheckpointRoundTrip) {
  size_t particle_count = GetParam();

  SimulationState original;
  original.particle_count = particle_count;
  original.simulation_time = 42.5f;
  original.dt = 0.002f;
  original.G = 6.674f;
  original.softening = 0.05f;
  original.force_method = ForceMethod::DIRECT_N2;

  original.pos_x.resize(particle_count);
  original.pos_y.resize(particle_count);
  original.pos_z.resize(particle_count);
  original.vel_x.resize(particle_count);
  original.vel_y.resize(particle_count);
  original.vel_z.resize(particle_count);
  original.mass.resize(particle_count);

  // Initialize with deterministic values based on index
  for (size_t i = 0; i < particle_count; i++) {
    original.pos_x[i] = static_cast<float>(i * 1.1);
    original.pos_y[i] = static_cast<float>(i * 2.2);
    original.pos_z[i] = static_cast<float>(i * 3.3);
    original.vel_x[i] = static_cast<float>(i * 0.1);
    original.vel_y[i] = static_cast<float>(i * 0.2);
    original.vel_z[i] = static_cast<float>(i * 0.3);
    original.mass[i] = static_cast<float>(1.0 + i * 0.01);
  }

  // Save to stream
  std::stringstream ss;
  Serializer::save(ss, original);

  // Load from stream
  ss.seekg(0);
  SimulationState loaded = Serializer::load(ss);

  // Verify all fields match
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

// Test with various particle counts: 0, 1, small, medium, and larger counts
INSTANTIATE_TEST_SUITE_P(CheckpointRoundTripVariousCounts, ParticleCountTestFixture,
                         ::testing::Values(0, 1, 10, 100, 1000, 10000));
