#include "nbody/app_cli.hpp"
#include "nbody/error_handling.hpp"
#include <cstdlib>
#include <sstream>
#include <stdexcept>

namespace nbody {

namespace {

ForceMethod parseForceMethod(const std::string& value) {
  if (value == "direct-n2" || value == "direct_n2") {
    return ForceMethod::DIRECT_N2;
  }
  if (value == "barnes-hut" || value == "barnes_hut") {
    return ForceMethod::BARNES_HUT;
  }
  if (value == "spatial-hash" || value == "spatial_hash") {
    return ForceMethod::SPATIAL_HASH;
  }
  throw ValidationException("Unsupported force method: " + value);
}

std::string requireValue(int argc, const char* const argv[], int& index, const std::string& flag) {
  if (index + 1 >= argc) {
    throw ValidationException("Missing value for " + flag);
  }
  return argv[++index];
}

size_t parseSizeValue(const std::string& value, const std::string& flag) {
  try {
    return static_cast<size_t>(std::stoull(value));
  } catch (const std::exception&) {
    throw ValidationException("Invalid numeric value for " + flag + ": " + value);
  }
}

float parseFloatValue(const std::string& value, const std::string& flag) {
  try {
    return std::stof(value);
  } catch (const std::exception&) {
    throw ValidationException("Invalid numeric value for " + flag + ": " + value);
  }
}

}  // namespace

AppCliOptions parseAppCliOptions(int argc, const char* const argv[]) {
  AppCliOptions options;

  for (int i = 1; i < argc; ++i) {
    const std::string argument = argv[i];
    if (argument == "--help" || argument == "-h") {
      options.show_help = true;
      continue;
    }
    if (argument == "--particles") {
      options.particle_count = parseSizeValue(requireValue(argc, argv, i, argument), argument);
      continue;
    }
    if (argument == "--method") {
      options.force_method = parseForceMethod(requireValue(argc, argv, i, argument));
      continue;
    }
    if (argument == "--dt") {
      options.dt = parseFloatValue(requireValue(argc, argv, i, argument), argument);
      continue;
    }
    if (argument == "--gravity") {
      options.G = parseFloatValue(requireValue(argc, argv, i, argument), argument);
      continue;
    }
    if (argument == "--softening") {
      options.softening = parseFloatValue(requireValue(argc, argv, i, argument), argument);
      continue;
    }
    if (argument == "--theta") {
      options.barnes_hut_theta = parseFloatValue(requireValue(argc, argv, i, argument), argument);
      continue;
    }
    if (argument == "--cell-size") {
      options.spatial_hash_cell_size =
          parseFloatValue(requireValue(argc, argv, i, argument), argument);
      continue;
    }
    if (argument == "--cutoff") {
      options.spatial_hash_cutoff =
          parseFloatValue(requireValue(argc, argv, i, argument), argument);
      continue;
    }
    if (argument == "--benchmark") {
      options.benchmark_mode = true;
      continue;
    }
    if (argument == "--benchmark-steps") {
      options.benchmark_steps = parseSizeValue(requireValue(argc, argv, i, argument), argument);
      options.benchmark_mode = true;
      continue;
    }
    if (argument == "--benchmark-output") {
      options.benchmark_output_path = requireValue(argc, argv, i, argument);
      options.benchmark_mode = true;
      continue;
    }
    if (argument == "--export") {
      options.export_path = requireValue(argc, argv, i, argument);
      continue;
    }
    if (argument == "--export-format") {
      options.export_format = requireValue(argc, argv, i, argument);
      continue;
    }
    if (argument == "--import") {
      options.import_path = requireValue(argc, argv, i, argument);
      continue;
    }
    if (argument == "--list-algorithms") {
      options.list_algorithms = true;
      continue;
    }
    if (argument == "--diagnostics") {
      options.show_diagnostics = true;
      continue;
    }
    if (!argument.empty() && argument.front() == '-') {
      throw ValidationException("Unknown argument: " + argument);
    }

    options.particle_count = parseSizeValue(argument, "particle count");
  }

  validateParticleCountRange(options.particle_count);
  validateTimeStep(options.dt);
  validateSoftening(options.softening);
  validateTheta(options.barnes_hut_theta);
  if (options.G <= 0.0f) {
    throw ValidationException("Gravitational constant must be positive");
  }
  if (options.spatial_hash_cell_size <= 0.0f) {
    throw ValidationException("Spatial hash cell size must be positive");
  }
  if (options.spatial_hash_cutoff <= 0.0f) {
    throw ValidationException("Spatial hash cutoff must be positive");
  }
  if (options.benchmark_steps == 0) {
    throw ValidationException("Benchmark steps must be greater than zero");
  }

  return options;
}

std::string appCliUsage() {
  std::ostringstream usage;
  usage << "Usage: nbody_sim [particle_count] [options]\n\n"
        << "Simulation options:\n"
        << "  --particles N          Set particle count\n"
        << "  --method NAME          direct-n2 | barnes-hut | spatial-hash\n"
        << "  --dt VALUE             Set integration time step\n"
        << "  --gravity VALUE        Set gravitational constant\n"
        << "  --softening VALUE      Set softening parameter\n"
        << "  --theta VALUE          Set Barnes-Hut theta\n"
        << "  --cell-size VALUE      Set spatial hash cell size\n"
        << "  --cutoff VALUE         Set spatial hash cutoff radius\n"
        << "  --benchmark            Run a non-interactive benchmark and exit\n"
        << "  --benchmark-steps N    Set benchmark update steps\n"
        << "  --benchmark-output P   Write benchmark JSON to path P\n"
        << "\nData export/import:\n"
        << "  --export PATH          Export particle state to file\n"
        << "  --export-format FMT    Export format: checkpoint (default)\n"
        << "  --import PATH          Import particle state from file\n"
        << "\nDiagnostics:\n"
        << "  --list-algorithms      List available force methods and exit\n"
        << "  --diagnostics          Output diagnostic information\n"
        << "  --help                 Show this message\n";
  return usage.str();
}

}  // namespace nbody
