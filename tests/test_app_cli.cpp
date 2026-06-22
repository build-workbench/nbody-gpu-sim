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
  const char* argv[] = {"nbody_sim",  "--particles", "5000",        "--method",
                        "barnes-hut", "--export",    "state.nbody"};

  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);

  EXPECT_EQ(options.particle_count, 5000u);
  EXPECT_EQ(options.force_method, ForceMethod::BARNES_HUT);
  EXPECT_EQ(options.export_path, "state.nbody");
}
