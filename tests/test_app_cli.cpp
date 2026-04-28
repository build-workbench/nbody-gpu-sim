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
  const char* argv[] = {"nbody_sim",  "--export", "output.nbody", "--export-format",
                        "checkpoint", "--import", "input.nbody"};

  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);

  EXPECT_EQ(options.export_path, "output.nbody");
  EXPECT_EQ(options.export_format, "checkpoint");
  EXPECT_EQ(options.import_path, "input.nbody");
}

TEST(AppCliTest, ParsesListAlgorithmsFlag) {
  const char* argv[] = {"nbody_sim", "--list-algorithms"};

  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);

  EXPECT_TRUE(options.list_algorithms);
}

TEST(AppCliTest, ParsesDiagnosticsFlag) {
  const char* argv[] = {"nbody_sim", "--diagnostics"};

  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);

  EXPECT_TRUE(options.show_diagnostics);
}

TEST(AppCliTest, ParsesCombinedOptions) {
  const char* argv[] = {"nbody_sim",  "--particles", "5000",        "--method",
                        "barnes-hut", "--export",    "state.nbody", "--diagnostics"};

  const AppCliOptions options = parseAppCliOptions(static_cast<int>(std::size(argv)), argv);

  EXPECT_EQ(options.particle_count, 5000u);
  EXPECT_EQ(options.force_method, ForceMethod::BARNES_HUT);
  EXPECT_EQ(options.export_path, "state.nbody");
  EXPECT_TRUE(options.show_diagnostics);
}
