#include "nbody/error_handling.hpp"
#include "nbody/particle_system.hpp"
#include "nbody/performance_observability.hpp"
#include "nbody/serialization.hpp"
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace nbody;

namespace {

struct BenchmarkOptions {
  std::string benchmark_name = "all";
  size_t particle_count = 4096;
  size_t iterations = 5;
  std::string output_path;
};

struct BenchmarkDefinition {
  std::string name;
  std::string description;
  bool requires_cuda = false;
  BenchmarkRunRecord (*run)(const BenchmarkOptions&);
};

SimulationState makeState(size_t particle_count) {
  SimulationState state;
  state.particle_count = particle_count;
  state.simulation_time = 1.0f;
  state.dt = 0.001f;
  state.G = 1.0f;
  state.softening = 0.1f;
  state.force_method = ForceMethod::DIRECT_N2;
  state.pos_x.resize(particle_count);
  state.pos_y.resize(particle_count);
  state.pos_z.resize(particle_count);
  state.vel_x.resize(particle_count);
  state.vel_y.resize(particle_count);
  state.vel_z.resize(particle_count);
  state.mass.resize(particle_count, 1.0f);

  for (size_t i = 0; i < particle_count; ++i) {
    state.pos_x[i] = static_cast<float>(i % 97) * 0.01f;
    state.pos_y[i] = static_cast<float>((i * 3) % 89) * 0.02f;
    state.pos_z[i] = static_cast<float>((i * 7) % 83) * 0.03f;
    state.vel_x[i] = 0.001f * static_cast<float>(i % 11);
    state.vel_y[i] = 0.002f * static_cast<float>(i % 13);
    state.vel_z[i] = 0.003f * static_cast<float>(i % 17);
  }

  return state;
}

BenchmarkRunRecord runSerializationBenchmark(const BenchmarkOptions& options) {
  BenchmarkRunRecord record;
  record.benchmark_name = "serialization.round_trip";
  record.force_method = ForceMethod::DIRECT_N2;
  record.particle_count = options.particle_count;
  record.iterations = options.iterations;

  const SimulationState state = makeState(options.particle_count);
  consumeGlobalPhaseSnapshot();

  double total_ms = 0.0;
  size_t total_bytes = 0;
  for (size_t iteration = 0; iteration < options.iterations; ++iteration) {
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    const auto start = std::chrono::steady_clock::now();
    Serializer::save(stream, state);
    total_bytes += static_cast<size_t>(stream.tellp());
    stream.seekg(0);
    const SimulationState loaded = Serializer::load(stream);
    const auto end = std::chrono::steady_clock::now();
    total_ms += std::chrono::duration_cast<Milliseconds>(end - start).count();

    if (loaded.particle_count != state.particle_count) {
      throw std::runtime_error("Serialization benchmark round-trip lost particle data");
    }
  }

  record.metrics["wall_time_ms"] = total_ms / static_cast<double>(options.iterations);
  record.metrics["bytes_per_iteration"] =
      static_cast<double>(total_bytes) / static_cast<double>(options.iterations);
  record.parameters["particle_count"] = static_cast<double>(options.particle_count);
  record.phase_timings = consumeGlobalPhaseSnapshot();
  return record;
}

#if defined(NBODY_WITH_CUDA) && NBODY_WITH_CUDA
BenchmarkRunRecord runForceBenchmark(const BenchmarkOptions& options, ForceMethod method,
                                     const std::string& benchmark_name) {
  SimulationConfig config;
  config.particle_count = options.particle_count;
  config.force_method = method;
  config.init_distribution = InitDistribution::SPHERICAL;

  ParticleSystem system;
  system.initialize(config);

  BenchmarkRunRecord record;
  record.benchmark_name = benchmark_name;
  record.force_method = method;
  record.particle_count = options.particle_count;
  record.iterations = options.iterations;
  record.parameters["particle_count"] = static_cast<double>(options.particle_count);
  record.parameters["cuda_block_size"] = static_cast<double>(config.cuda_block_size);
  if (method == ForceMethod::BARNES_HUT) {
    record.parameters["theta"] = config.barnes_hut_theta;
  } else if (method == ForceMethod::SPATIAL_HASH) {
    record.parameters["cell_size"] = config.spatial_hash_cell_size;
    record.parameters["cutoff_radius"] = config.spatial_hash_cutoff;
  }

  auto calculator = createForceCalculator(method, config);
  consumeGlobalPhaseSnapshot();

  double total_ms = 0.0;
  for (size_t iteration = 0; iteration < options.iterations; ++iteration) {
    const auto start = std::chrono::steady_clock::now();
    calculator->computeForces(system.getDeviceData());
    const auto end = std::chrono::steady_clock::now();
    total_ms += std::chrono::duration_cast<Milliseconds>(end - start).count();
  }

  record.metrics["wall_time_ms"] = total_ms / static_cast<double>(options.iterations);
  record.phase_timings = consumeGlobalPhaseSnapshot();
  return record;
}

BenchmarkRunRecord runIntegrationBenchmark(const BenchmarkOptions& options) {
  SimulationConfig config;
  config.particle_count = options.particle_count;
  config.force_method = ForceMethod::DIRECT_N2;
  config.init_distribution = InitDistribution::SPHERICAL;

  ParticleSystem system;
  system.initialize(config);

  BenchmarkRunRecord record;
  record.benchmark_name = "integration.velocity_verlet";
  record.force_method = config.force_method;
  record.particle_count = options.particle_count;
  record.iterations = options.iterations;
  record.parameters["particle_count"] = static_cast<double>(options.particle_count);
  record.parameters["dt"] = config.dt;
  record.parameters["cuda_block_size"] = static_cast<double>(config.cuda_block_size);

  consumeGlobalPhaseSnapshot();

  double total_ms = 0.0;
  for (size_t iteration = 0; iteration < options.iterations; ++iteration) {
    const auto start = std::chrono::steady_clock::now();
    system.update(system.getTimeStep());
    const auto end = std::chrono::steady_clock::now();
    total_ms += std::chrono::duration_cast<Milliseconds>(end - start).count();
  }

  record.metrics["wall_time_ms"] = total_ms / static_cast<double>(options.iterations);
  record.phase_timings = consumeGlobalPhaseSnapshot();
  return record;
}

BenchmarkRunRecord runDirectBenchmark(const BenchmarkOptions& options) {
  return runForceBenchmark(options, ForceMethod::DIRECT_N2, "force.direct_n2");
}

BenchmarkRunRecord runBarnesHutBenchmark(const BenchmarkOptions& options) {
  SimulationConfig config;
  config.particle_count = options.particle_count;
  config.force_method = ForceMethod::BARNES_HUT;
  config.init_distribution = InitDistribution::SPHERICAL;

  ParticleSystem system;
  system.initialize(config);

  BenchmarkRunRecord record;
  record.benchmark_name = "force.barnes_hut";
  record.force_method = ForceMethod::BARNES_HUT;
  record.particle_count = options.particle_count;
  record.iterations = options.iterations;
  record.parameters["particle_count"] = static_cast<double>(options.particle_count);
  record.parameters["theta"] = config.barnes_hut_theta;

  auto calculator = createForceCalculator(ForceMethod::BARNES_HUT, config);
  consumeGlobalPhaseSnapshot();

  double total_ms = 0.0;
  for (size_t iteration = 0; iteration < options.iterations; ++iteration) {
    const auto start = std::chrono::steady_clock::now();
    calculator->computeForces(system.getDeviceData());
    const auto end = std::chrono::steady_clock::now();
    total_ms += std::chrono::duration_cast<Milliseconds>(end - start).count();
  }

  record.metrics["wall_time_ms"] = total_ms / static_cast<double>(options.iterations);

  // Capture phase timings for Barnes-Hut breakdown
  record.phase_timings = consumeGlobalPhaseSnapshot();

  // Add phase-specific metrics from timings
  for (const auto& phase : record.phase_timings) {
    if (phase.samples > 0) {
      record.metrics[phase.name + "_ms"] = phase.total_duration.count() / phase.samples;
    }
  }

  return record;
}

BenchmarkRunRecord runSpatialHashBenchmark(const BenchmarkOptions& options) {
  return runForceBenchmark(options, ForceMethod::SPATIAL_HASH, "force.spatial_hash");
}
#endif

std::vector<BenchmarkDefinition> makeBenchmarks() {
  std::vector<BenchmarkDefinition> benchmarks{
      {"serialization.round_trip", "Binary checkpoint serialization and load round-trip", false,
       runSerializationBenchmark},
  };

#if defined(NBODY_WITH_CUDA) && NBODY_WITH_CUDA
  benchmarks.push_back(
      {"force.direct_n2", "Direct N^2 force calculation", true, runDirectBenchmark});
  benchmarks.push_back(
      {"force.barnes_hut", "Barnes-Hut force calculation", true, runBarnesHutBenchmark});
  benchmarks.push_back(
      {"force.spatial_hash", "Spatial hash force calculation", true, runSpatialHashBenchmark});
  benchmarks.push_back({"integration.velocity_verlet", "Velocity Verlet integration step", true,
                        runIntegrationBenchmark});
#endif

  return benchmarks;
}

void printUsage(const std::vector<BenchmarkDefinition>& benchmarks) {
  std::cout << "Usage: nbody_benchmarks [--benchmark <name|all>] [--particle-count N]\n"
               "                        [--iterations N] [--output path] [--list]\n\n"
               "Available benchmarks:\n";
  for (const auto& benchmark : benchmarks) {
    std::cout << "  - " << benchmark.name;
    if (benchmark.requires_cuda) {
      std::cout << " (CUDA)";
    }
    std::cout << ": " << benchmark.description << "\n";
  }
}

BenchmarkOptions parseOptions(int argc, char* argv[],
                              const std::vector<BenchmarkDefinition>& benchmarks) {
  BenchmarkOptions options;
  for (int i = 1; i < argc; ++i) {
    const std::string argument = argv[i];
    if (argument == "--benchmark" && i + 1 < argc) {
      options.benchmark_name = argv[++i];
    } else if (argument == "--particle-count" && i + 1 < argc) {
      options.particle_count = static_cast<size_t>(std::stoull(argv[++i]));
    } else if (argument == "--iterations" && i + 1 < argc) {
      options.iterations = static_cast<size_t>(std::stoull(argv[++i]));
    } else if (argument == "--output" && i + 1 < argc) {
      options.output_path = argv[++i];
    } else if (argument == "--list") {
      printUsage(benchmarks);
      std::exit(0);
    } else if (argument == "--help" || argument == "-h") {
      printUsage(benchmarks);
      std::exit(0);
    } else {
      throw ValidationException("Unknown benchmark argument: " + argument);
    }
  }

  if (options.iterations == 0) {
    throw ValidationException("Benchmark iterations must be greater than zero");
  }

  return options;
}

}  // namespace

int main(int argc, char* argv[]) {
  try {
    const auto benchmarks = makeBenchmarks();
    const BenchmarkOptions options = parseOptions(argc, argv, benchmarks);

    std::vector<BenchmarkRunRecord> results;
    for (const auto& benchmark : benchmarks) {
      if (options.benchmark_name != "all" && options.benchmark_name != benchmark.name) {
        continue;
      }
      results.push_back(benchmark.run(options));
    }

    if (results.empty()) {
      throw ValidationException("Requested benchmark was not found");
    }

    const std::string report = serializeBenchmarkRunRecords(results);
    std::cout << report << "\n";
    if (!options.output_path.empty()) {
      writeBenchmarkRunRecords(options.output_path, results);
    }
  } catch (const std::exception& error) {
    std::cerr << "Benchmark error: " << error.what() << "\n";
    return 1;
  }

  return 0;
}
