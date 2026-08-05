#pragma once

#include "nbody/error_handling.hpp"
#include "nbody/simulation_state.hpp"
#include "nbody/types.hpp"
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace nbody {

constexpr uint32_t NBODY_MAGIC = 0x4E424F44;
constexpr uint32_t NBODY_VERSION = 1;
constexpr uint64_t MAX_PARTICLE_COUNT = 100'000'000;

// On-disk checkpoint header. Written/read as a raw blob, so the layout is
// part of the file format: native byte order (little-endian on all supported
// targets) and the exact size/offsets pinned by the static_assert below.
// Writers must zero the whole struct (including tail padding) before writing.
struct FileHeader {
  uint32_t magic;
  uint32_t version;
  uint64_t particle_count;
  float simulation_time;
  float dt;
  float G;
  float softening;
  uint32_t force_method;
  uint32_t reserved[4];
};
static_assert(sizeof(FileHeader) == 56, "FileHeader layout changed; on-disk format would break");

class Serializer {
public:
  static void save(const std::string& filename, const SimulationState& state);
  static SimulationState load(const std::string& filename);
  static void save(std::ostream& out, const SimulationState& state);
  static SimulationState load(std::istream& in);
  static bool validateFile(const std::string& filename);
  static bool validateStream(std::istream& in);

private:
  static void writeHeader(std::ostream& out, const SimulationState& state);
  static FileHeader readHeader(std::istream& in);
  static void writeFloatArray(std::ostream& out, const std::vector<float>& data);
  static std::vector<float> readFloatArray(std::istream& in, size_t count);
};

}  // namespace nbody
