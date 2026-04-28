#include "nbody/performance_observability.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace nbody {

namespace {

std::string escapeJson(std::string_view value) {
  std::ostringstream escaped;
  for (const char ch : value) {
    switch (ch) {
    case '\\':
      escaped << "\\\\";
      break;
    case '"':
      escaped << "\\\"";
      break;
    case '\n':
      escaped << "\\n";
      break;
    case '\r':
      escaped << "\\r";
      break;
    case '\t':
      escaped << "\\t";
      break;
    default:
      escaped << ch;
      break;
    }
  }
  return escaped.str();
}

void appendNumberMap(std::ostringstream& out, const std::map<std::string, double>& values) {
  out << "{";
  bool first = true;
  for (const auto& [key, value] : values) {
    if (!first) {
      out << ",";
    }
    first = false;
    out << "\"" << escapeJson(key) << "\":" << value;
  }
  out << "}";
}

PhaseProfiler global_profiler;

}  // namespace

void PhaseProfiler::record(std::string_view name, Milliseconds duration) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& phase : phases_) {
    if (phase.name == name) {
      phase.total_duration += duration;
      phase.samples += 1;
      return;
    }
  }

  PhaseTiming phase;
  phase.name = std::string(name);
  phase.total_duration = duration;
  phase.samples = 1;
  phases_.push_back(std::move(phase));
}

std::vector<PhaseTiming> PhaseProfiler::snapshot() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return phases_;
}

void PhaseProfiler::reset() {
  std::lock_guard<std::mutex> lock(mutex_);
  phases_.clear();
}

ScopedPhaseProfile::ScopedPhaseProfile(PhaseProfiler& profiler, std::string name)
    : profiler_(profiler), name_(std::move(name)), start_time_(std::chrono::steady_clock::now()) {}

ScopedPhaseProfile::~ScopedPhaseProfile() {
  const auto end_time = std::chrono::steady_clock::now();
  profiler_.record(name_, std::chrono::duration_cast<Milliseconds>(end_time - start_time_));
}

std::string forceMethodToString(ForceMethod method) {
  switch (method) {
  case ForceMethod::DIRECT_N2:
    return "direct_n2";
  case ForceMethod::BARNES_HUT:
    return "barnes_hut";
  case ForceMethod::SPATIAL_HASH:
    return "spatial_hash";
  default:
    return "unknown";
  }
}

std::string serializeBenchmarkRunRecord(const BenchmarkRunRecord& record) {
  std::ostringstream out;
  out << "{";
  out << "\"benchmark_name\":\"" << escapeJson(record.benchmark_name) << "\",";
  out << "\"force_method\":\"" << forceMethodToString(record.force_method) << "\",";
  out << "\"particle_count\":" << record.particle_count << ",";
  out << "\"iterations\":" << record.iterations << ",";
  out << "\"metrics\":";
  appendNumberMap(out, record.metrics);
  out << ",";
  out << "\"parameters\":";
  appendNumberMap(out, record.parameters);
  out << ",";
  out << "\"phase_timings\":[";
  bool first_phase = true;
  for (const auto& phase : record.phase_timings) {
    if (!first_phase) {
      out << ",";
    }
    first_phase = false;
    out << "{";
    out << "\"name\":\"" << escapeJson(phase.name) << "\",";
    out << "\"total_duration_ms\":" << phase.total_duration.count() << ",";
    out << "\"samples\":" << phase.samples;
    out << "}";
  }
  out << "]";
  out << "}";
  return out.str();
}

std::string serializeBenchmarkRunRecords(const std::vector<BenchmarkRunRecord>& records) {
  std::ostringstream out;
  out << "{\"benchmarks\":[";
  for (size_t i = 0; i < records.size(); ++i) {
    if (i != 0) {
      out << ",";
    }
    out << serializeBenchmarkRunRecord(records[i]);
  }
  out << "]}";
  return out.str();
}

void writeBenchmarkRunRecords(const std::string& path,
                              const std::vector<BenchmarkRunRecord>& records) {
  std::ofstream output(path);
  if (!output) {
    throw std::runtime_error("Failed to open benchmark output file: " + path);
  }
  output << serializeBenchmarkRunRecords(records) << "\n";
}

PhaseProfiler& globalPhaseProfiler() {
  return global_profiler;
}

std::vector<PhaseTiming> consumeGlobalPhaseSnapshot() {
  auto phases = global_profiler.snapshot();
  global_profiler.reset();
  return phases;
}

}  // namespace nbody
