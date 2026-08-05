#include "nbody/serialization.hpp"
#include "nbody/error_handling.hpp"
#include "nbody/performance_observability.hpp"
#include <cstring>
#include <fstream>

namespace nbody {

void Serializer::save(const std::string& filename, const SimulationState& state) {
  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    throw IOException("Failed to open file for writing: " + filename);
  }
  save(file, state);
}

SimulationState Serializer::load(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    throw IOException("Failed to open file for reading: " + filename);
  }
  return load(file);
}

void Serializer::save(std::ostream& out, const SimulationState& state) {
  NBODY_PROFILE_SCOPE("serialization.save");
  writeHeader(out, state);

  // Write particle data
  writeFloatArray(out, state.pos_x);
  writeFloatArray(out, state.pos_y);
  writeFloatArray(out, state.pos_z);
  writeFloatArray(out, state.vel_x);
  writeFloatArray(out, state.vel_y);
  writeFloatArray(out, state.vel_z);
  writeFloatArray(out, state.mass);
}

SimulationState Serializer::load(std::istream& in) {
  NBODY_PROFILE_SCOPE("serialization.load");
  FileHeader header = readHeader(in);

  // Validate particle count to prevent memory exhaustion
  if (header.particle_count > MAX_PARTICLE_COUNT) {
    throw ValidationException("Particle count (" + std::to_string(header.particle_count) +
                              ") exceeds maximum allowed (" + std::to_string(MAX_PARTICLE_COUNT) +
                              ")");
  }

  // Validate the force method before casting to the enum: an out-of-range
  // value from a corrupted file would otherwise be silently accepted.
  if (header.force_method > static_cast<uint32_t>(ForceMethod::SPATIAL_HASH)) {
    throw ValidationException("Invalid force method in file header: " +
                              std::to_string(header.force_method));
  }

  SimulationState state;
  state.particle_count = header.particle_count;
  state.simulation_time = header.simulation_time;
  state.dt = header.dt;
  state.G = header.G;
  state.softening = header.softening;
  state.force_method = static_cast<ForceMethod>(header.force_method);

  // Read particle data
  state.pos_x = readFloatArray(in, state.particle_count);
  state.pos_y = readFloatArray(in, state.particle_count);
  state.pos_z = readFloatArray(in, state.particle_count);
  state.vel_x = readFloatArray(in, state.particle_count);
  state.vel_y = readFloatArray(in, state.particle_count);
  state.vel_z = readFloatArray(in, state.particle_count);
  state.mass = readFloatArray(in, state.particle_count);

  return state;
}

bool Serializer::validateFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file)
    return false;
  return validateStream(file);
}

bool Serializer::validateStream(std::istream& in) {
  try {
    FileHeader header = readHeader(in);
    return header.magic == NBODY_MAGIC && header.version == NBODY_VERSION;
  } catch (const std::exception&) {
    return false;
  }
}

void Serializer::writeHeader(std::ostream& out, const SimulationState& state) {
  FileHeader header;
  // Zero the entire struct, including compiler-inserted tail padding, so the
  // on-disk bytes are deterministic and no uninitialized stack memory leaks
  // into the checkpoint file.
  std::memset(&header, 0, sizeof(header));
  header.magic = NBODY_MAGIC;
  header.version = NBODY_VERSION;
  header.particle_count = state.particle_count;
  header.simulation_time = state.simulation_time;
  header.dt = state.dt;
  header.G = state.G;
  header.softening = state.softening;
  header.force_method = static_cast<uint32_t>(state.force_method);

  out.write(reinterpret_cast<const char*>(&header), sizeof(header));
}

FileHeader Serializer::readHeader(std::istream& in) {
  FileHeader header;
  in.read(reinterpret_cast<char*>(&header), sizeof(header));

  // Check if read succeeded
  if (!in || in.gcount() != sizeof(header)) {
    throw ValidationException("Failed to read file header: file may be truncated or corrupted");
  }

  if (header.magic != NBODY_MAGIC) {
    throw ValidationException("Invalid file format: wrong magic number");
  }

  if (header.version != NBODY_VERSION) {
    throw ValidationException("Unsupported file version");
  }

  return header;
}

void Serializer::writeFloatArray(std::ostream& out, const std::vector<float>& data) {
  out.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(float));
}

std::vector<float> Serializer::readFloatArray(std::istream& in, size_t count) {
  std::vector<float> data(count);
  in.read(reinterpret_cast<char*>(data.data()), count * sizeof(float));

  // Check if read succeeded
  if (!in || in.gcount() != static_cast<std::streamsize>(count * sizeof(float))) {
    throw ValidationException("Failed to read particle data: file may be truncated or corrupted");
  }

  return data;
}

}  // namespace nbody
