#include <gtest/gtest.h>

#if NBODY_WITH_HDF5

#include "nbody/hdf5_io.hpp"
#include "nbody/types.hpp"
#include <cmath>
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace nbody;

class HDF5IOTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create unique temp filename
    char template_path[] = "/tmp/nbody_test_XXXXXX";
    int fd = mkstemp(template_path);
    close(fd);
    temp_filename_ = std::string(template_path) + ".h5";
    unlink(template_path);
  }

  void TearDown() override {
    // Clean up temp file
    unlink(temp_filename_.c_str());
  }

  std::string temp_filename_;
  SimulationState createTestState(size_t count) {
    SimulationState state;
    state.particle_count = count;
    state.simulation_time = 1.5f;
    state.dt = 0.001f;
    state.G = 1.0f;
    state.softening = 0.1f;
    state.force_method = ForceMethod::BARNES_HUT;

    state.pos_x.resize(count);
    state.pos_y.resize(count);
    state.pos_z.resize(count);
    state.vel_x.resize(count);
    state.vel_y.resize(count);
    state.vel_z.resize(count);
    state.mass.resize(count);

    for (size_t i = 0; i < count; i++) {
      state.pos_x[i] = static_cast<float>(i);
      state.pos_y[i] = static_cast<float>(i * 2);
      state.pos_z[i] = static_cast<float>(i * 3);
      state.vel_x[i] = 0.1f * i;
      state.vel_y[i] = 0.2f * i;
      state.vel_z[i] = 0.3f * i;
      state.mass[i] = 1.0f;
    }

    return state;
  }
};

TEST_F(HDF5IOTest, ExportCreatesValidFile) {
  SimulationState state = createTestState(10);

  HDF5IO::exportToFile(temp_filename_, state);

  EXPECT_TRUE(HDF5IO::validateFile(temp_filename_));
}

TEST_F(HDF5IOTest, RoundTripPreservesData) {
  SimulationState original = createTestState(100);

  // Export
  HDF5IO::exportToFile(temp_filename_, original);

  // Import
  SimulationState loaded = HDF5IO::importFromFile(temp_filename_);

  // Verify
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

TEST_F(HDF5IOTest, ValidateRejectsNonExistentFile) {
  EXPECT_FALSE(HDF5IO::validateFile("/nonexistent/file.h5"));
}

TEST_F(HDF5IOTest, ValidateRejectsNonHDF5File) {
  // Write a plain text file
  std::ofstream out(temp_filename_);
  out << "This is not an HDF5 file";
  out.close();

  EXPECT_FALSE(HDF5IO::validateFile(temp_filename_));
}

TEST_F(HDF5IOTest, ExportWithLargeParticleCount) {
  SimulationState state = createTestState(10000);

  HDF5IO::exportToFile(temp_filename_, state);

  EXPECT_TRUE(HDF5IO::validateFile(temp_filename_));

  SimulationState loaded = HDF5IO::importFromFile(temp_filename_);
  EXPECT_EQ(loaded.particle_count, 10000u);
}

TEST_F(HDF5IOTest, PreservesForceMethod) {
  SimulationState state = createTestState(10);
  state.force_method = ForceMethod::SPATIAL_HASH;

  HDF5IO::exportToFile(temp_filename_, state);
  SimulationState loaded = HDF5IO::importFromFile(temp_filename_);

  EXPECT_EQ(loaded.force_method, ForceMethod::SPATIAL_HASH);
}

#endif  // NBODY_WITH_HDF5

#if !NBODY_WITH_HDF5
// When HDF5 is disabled, provide a dummy test
TEST(HDF5GatingTest, HDF5DisabledWhenHDF5NotEnabled) {
  SUCCEED() << "HDF5 tests skipped: NBODY_WITH_HDF5 is OFF";
}
#endif
