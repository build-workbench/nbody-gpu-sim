#include "nbody/performance_observability.hpp"
#include <gtest/gtest.h>
#include <thread>

using namespace nbody;

TEST(PerformanceObservabilityTest, BenchmarkReportIncludesConfigurationAndMetrics) {
  BenchmarkRunRecord record;
  record.benchmark_name = "force.direct_n2";
  record.force_method = ForceMethod::DIRECT_N2;
  record.particle_count = 4096;
  record.iterations = 12;
  record.metrics["wall_time_ms"] = 1.25;
  record.metrics["throughput_particles_per_second"] = 32768.0;
  record.parameters["cuda_block_size"] = 256.0;

  const std::string json = serializeBenchmarkRunRecord(record);

  EXPECT_NE(json.find("\"benchmark_name\":\"force.direct_n2\""), std::string::npos);
  EXPECT_NE(json.find("\"particle_count\":4096"), std::string::npos);
  EXPECT_NE(json.find("\"force_method\":\"direct_n2\""), std::string::npos);
  EXPECT_NE(json.find("\"wall_time_ms\":1.25"), std::string::npos);
  EXPECT_NE(json.find("\"cuda_block_size\":256"), std::string::npos);
}

TEST(PerformanceObservabilityTest, ScopedPhaseProfilerAccumulatesNamedDurations) {
  PhaseProfiler profiler;

  {
    ScopedPhaseProfile profile(profiler, "simulation.update");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  const auto snapshot = profiler.snapshot();
  ASSERT_EQ(snapshot.size(), 1u);
  EXPECT_EQ(snapshot.front().name, "simulation.update");
  EXPECT_GE(snapshot.front().total_duration.count(), 0.5);
  EXPECT_EQ(snapshot.front().samples, 1u);
}
