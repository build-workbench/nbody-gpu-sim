#pragma once

#include "nbody/types.hpp"
#include <chrono>
#include <cstddef>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace nbody {

using Milliseconds = std::chrono::duration<double, std::milli>;

struct PhaseTiming {
  std::string name;
  Milliseconds total_duration{0.0};
  size_t samples = 0;
};

class PhaseProfiler {
public:
  void record(std::string_view name, Milliseconds duration);
  std::vector<PhaseTiming> snapshot() const;
  void reset();

private:
  mutable std::mutex mutex_;
  std::vector<PhaseTiming> phases_;
};

class ScopedPhaseProfile {
public:
  ScopedPhaseProfile(PhaseProfiler& profiler, std::string name);
  ~ScopedPhaseProfile();

  ScopedPhaseProfile(const ScopedPhaseProfile&) = delete;
  ScopedPhaseProfile& operator=(const ScopedPhaseProfile&) = delete;

private:
  PhaseProfiler& profiler_;
  std::string name_;
  std::chrono::steady_clock::time_point start_time_;
};

struct BenchmarkRunRecord {
  std::string benchmark_name;
  ForceMethod force_method = ForceMethod::DIRECT_N2;
  size_t particle_count = 0;
  size_t iterations = 0;
  std::map<std::string, double> metrics;
  std::map<std::string, double> parameters;
  std::vector<PhaseTiming> phase_timings;
};

std::string forceMethodToString(ForceMethod method);
std::string serializeBenchmarkRunRecord(const BenchmarkRunRecord& record);
std::string serializeBenchmarkRunRecords(const std::vector<BenchmarkRunRecord>& records);
void writeBenchmarkRunRecords(const std::string& path,
                              const std::vector<BenchmarkRunRecord>& records);

PhaseProfiler& globalPhaseProfiler();
std::vector<PhaseTiming> consumeGlobalPhaseSnapshot();

#if defined(NBODY_ENABLE_PROFILING) && NBODY_ENABLE_PROFILING
#define NBODY_PROFILE_SCOPE(name)                                                              \
  ::nbody::ScopedPhaseProfile nbody_scoped_phase_profile_##__LINE__(                           \
      ::nbody::globalPhaseProfiler(), (name))
#else
#define NBODY_PROFILE_SCOPE(name) \
  do {                            \
  } while (0)
#endif

}  // namespace nbody
