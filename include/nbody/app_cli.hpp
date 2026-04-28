#pragma once

#include "nbody/types.hpp"
#include <string>

namespace nbody {

struct AppCliOptions {
  size_t particle_count = 10000;
  ForceMethod force_method = ForceMethod::DIRECT_N2;
  float dt = 0.001f;
  float G = 1.0f;
  float softening = 0.1f;
  float barnes_hut_theta = 0.5f;
  float spatial_hash_cell_size = 1.0f;
  float spatial_hash_cutoff = 2.0f;
  bool benchmark_mode = false;
  size_t benchmark_steps = 120;
  std::string benchmark_output_path;
  bool show_help = false;
  std::string export_path;
  std::string export_format;
  std::string import_path;
  bool list_algorithms = false;
  bool show_diagnostics = false;
};

AppCliOptions parseAppCliOptions(int argc, const char* const argv[]);
std::string appCliUsage();

}  // namespace nbody
