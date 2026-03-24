#include "nbody/simulation_state.hpp"
#include "nbody/serialization.hpp"
#include <cmath>

namespace nbody {

bool SimulationState::operator==(const SimulationState &other) const {
  if (particle_count != other.particle_count)
    return false;
  if (std::abs(simulation_time - other.simulation_time) > 1e-6f)
    return false;
  if (std::abs(dt - other.dt) > 1e-6f)
    return false;
  if (std::abs(G - other.G) > 1e-6f)
    return false;
  if (std::abs(softening - other.softening) > 1e-6f)
    return false;
  if (force_method != other.force_method)
    return false;

  for (size_t i = 0; i < particle_count; i++) {
    if (std::abs(pos_x[i] - other.pos_x[i]) > 1e-6f)
      return false;
    if (std::abs(pos_y[i] - other.pos_y[i]) > 1e-6f)
      return false;
    if (std::abs(pos_z[i] - other.pos_z[i]) > 1e-6f)
      return false;
    if (std::abs(vel_x[i] - other.vel_x[i]) > 1e-6f)
      return false;
    if (std::abs(vel_y[i] - other.vel_y[i]) > 1e-6f)
      return false;
    if (std::abs(vel_z[i] - other.vel_z[i]) > 1e-6f)
      return false;
    if (std::abs(mass[i] - other.mass[i]) > 1e-6f)
      return false;
  }

  return true;
}

void SimulationState::serialize(std::ostream &out) const {
  Serializer::save(out, *this);
}

SimulationState SimulationState::deserialize(std::istream &in) {
  return Serializer::load(in);
}

} // namespace nbody
