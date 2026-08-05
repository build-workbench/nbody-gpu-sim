#include "nbody/app_cli.hpp"
#include "nbody/error_handling.hpp"
#include <gtest/gtest.h>

using namespace nbody;

TEST(AppCliTest, ParsesStructuredSimulationOptions) {
  const char* argv[] = {"nbody_sim",
                        "--particles",
                        "2048",
                        "--method",
                        "barnes-hut",
                        "--dt",
                        "0.002",
                        "--theta",
                        "0.7",
                        "--softening",
                        "0.05",
                        "--benchmark",
                        "--benchmark-steps",
                        "12"};

  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);

  EXPECT_EQ(options.particle_count, 2048u);
  EXPECT_EQ(options.force_method, ForceMethod::BARNES_HUT);
  EXPECT_FLOAT_EQ(options.dt, 0.002f);
  EXPECT_FLOAT_EQ(options.barnes_hut_theta, 0.7f);
  EXPECT_FLOAT_EQ(options.softening, 0.05f);
  EXPECT_TRUE(options.benchmark_mode);
  EXPECT_EQ(options.benchmark_steps, 12u);
}

TEST(AppCliTest, RejectsUnknownForceMethod) {
  const char* argv[] = {"nbody_sim", "--method", "mystery"};
  EXPECT_THROW(parseAppCliOptions(static_cast<int>(std::size(argv)), argv), ValidationException);
}

TEST(AppCliTest, ParsesExportImportOptions) {
  const char* argv[] = {"nbody_sim", "--export", "output.nbody", "--import", "input.nbody"};

  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);

  EXPECT_EQ(options.export_path, "output.nbody");
  EXPECT_EQ(options.import_path, "input.nbody");
}

TEST(AppCliTest, RejectsUnknownArgument) {
  const char* argv[] = {"nbody_sim", "--bogus-flag"};
  EXPECT_THROW(parseAppCliOptions(static_cast<int>(std::size(argv)), argv), ValidationException);
}

TEST(AppCliTest, ParsesCombinedOptions) {
  const char* argv[] = {"nbody_sim",  "--particles", "5000",       "--method",
                        "barnes-hut", "--export",    "state.nbody"};

  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);

  EXPECT_EQ(options.particle_count, 5000u);
  EXPECT_EQ(options.force_method, ForceMethod::BARNES_HUT);
  EXPECT_EQ(options.export_path, "state.nbody");
}

TEST(AppCliTest, ParsesPositionalParticleCount) {
  const char* argv[] = {"nbody_sim", "12345"};
  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);
  EXPECT_EQ(options.particle_count, 12345u);
}

TEST(AppCliTest, ParsesHelpFlag) {
  const char* argv[] = {"nbody_sim", "--help"};
  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);
  EXPECT_TRUE(options.show_help);
}

TEST(AppCliTest, ParsesGravityAndSpatialHashOptions) {
  const char* argv[] = {"nbody_sim", "--gravity", "2.5", "--cell-size", "3.0", "--cutoff", "2.0"};
  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);
  EXPECT_FLOAT_EQ(options.G, 2.5f);
  EXPECT_FLOAT_EQ(options.spatial_hash_cell_size, 3.0f);
  EXPECT_FLOAT_EQ(options.spatial_hash_cutoff, 2.0f);
}

TEST(AppCliTest, BenchmarkOutputEnablesBenchmarkMode) {
  const char* argv[] = {"nbody_sim", "--benchmark-output", "results.json"};
  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);
  EXPECT_TRUE(options.benchmark_mode);
  EXPECT_EQ(options.benchmark_output_path, "results.json");
}

TEST(AppCliTest, RejectsFlagMissingValue) {
  const char* argv[] = {"nbody_sim", "--particles"};
  EXPECT_THROW(parseAppCliOptions(static_cast<int>(std::size(argv)), argv), ValidationException);
}

TEST(AppCliTest, RejectsNonNumericParticleCount) {
  const char* argv[] = {"nbody_sim", "--particles", "not-a-number"};
  EXPECT_THROW(parseAppCliOptions(static_cast<int>(std::size(argv)), argv), ValidationException);
}

TEST(AppCliTest, RejectsNonNumericFloatValue) {
  const char* argv[] = {"nbody_sim", "--dt", "fast"};
  EXPECT_THROW(parseAppCliOptions(static_cast<int>(std::size(argv)), argv), ValidationException);
}

TEST(AppCliTest, RejectsNonNumericPositionalCount) {
  const char* argv[] = {"nbody_sim", "lots"};
  EXPECT_THROW(parseAppCliOptions(static_cast<int>(std::size(argv)), argv), ValidationException);
}

TEST(AppCliTest, RejectsZeroBenchmarkSteps) {
  const char* argv[] = {"nbody_sim", "--benchmark-steps", "0"};
  EXPECT_THROW(parseAppCliOptions(static_cast<int>(std::size(argv)), argv), ValidationException);
}

TEST(AppCliTest, RejectsNonPositiveGravity) {
  const char* argv[] = {"nbody_sim", "--gravity", "0"};
  EXPECT_THROW(parseAppCliOptions(static_cast<int>(std::size(argv)), argv), ValidationException);
}

TEST(AppCliTest, RejectsNonPositiveDt) {
  const char* argv[] = {"nbody_sim", "--dt", "-0.1"};
  EXPECT_THROW(parseAppCliOptions(static_cast<int>(std::size(argv)), argv), ValidationException);
}

TEST(AppCliTest, RejectsZeroSoftening) {
  const char* argv[] = {"nbody_sim", "--softening", "0"};
  EXPECT_THROW(parseAppCliOptions(static_cast<int>(std::size(argv)), argv), ValidationException);
}

TEST(AppCliTest, RejectsOutOfRangeTheta) {
  const char* argv[] = {"nbody_sim", "--theta", "2.5"};
  EXPECT_THROW(parseAppCliOptions(static_cast<int>(std::size(argv)), argv), ValidationException);
}
