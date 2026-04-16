#pragma once

#include "nbody/types.hpp"
#include <iosfwd>
#include <vector>

namespace nbody {

// Simulation state for serialization
struct SimulationState {
  std::vector<float> pos_x, pos_y, pos_z;
  std::vector<float> vel_x, vel_y, vel_z;
  std::vector<float> mass;
  size_t particle_count = 0;
  float simulation_time = 0.0f;
  float dt = 0.0f;
  float G = 0.0f;
  float softening = 0.0f;
  ForceMethod force_method = ForceMethod::DIRECT_N2;

  // Serialization
  void serialize(std::ostream& out) const;
  static SimulationState deserialize(std::istream& in);

  // Comparison for testing
  bool operator==(const SimulationState& other) const;
};

}  // namespace nbody
