/**
 * @file serialization.hpp
 * @brief Binary serialization for simulation state checkpoints.
 *
 * This is the fast, internal checkpoint format for the n-body simulation.
 * It provides efficient binary serialization suitable for:
 * - Pause/resume state preservation
 * - Fast checkpoint/restart operations
 * - Internal simulation snapshots
 *
 * The .nbody binary format is versioned and includes validation headers,
 * but is intended for internal use within this simulation platform.
 *
 * For external interoperability with other tools and formats, use the
 * HDF5 format when available (conditionally compiled with NBODY_WITH_HDF5).
 * HDF5 provides better interchange with scientific computing tools.
 *
 * @note The binary format uses NBODY_MAGIC and NBODY_VERSION for validation.
 * @note Maximum particle count is limited to prevent memory exhaustion.
 */

#pragma once

#include "nbody/simulation_state.hpp"
#include "nbody/types.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

namespace nbody {

/**
 * @brief Magic number for binary checkpoint format ("NBOD").
 */
constexpr uint32_t NBODY_MAGIC = 0x4E424F44;

/**
 * @brief Current version of the binary checkpoint format.
 */
constexpr uint32_t NBODY_VERSION = 1;

/**
 * @brief Maximum allowed particle count for serialization (100 million).
 *
 * This prevents memory exhaustion from corrupted or malicious files.
 */
constexpr uint64_t MAX_PARTICLE_COUNT = 100'000'000;

/**
 * @brief File header structure for binary checkpoint format.
 *
 * Contains metadata and simulation parameters followed by particle data.
 */
struct FileHeader {
  uint32_t magic;            ///< Magic number for format validation
  uint32_t version;          ///< Format version number
  uint64_t particle_count;   ///< Number of particles in the checkpoint
  float simulation_time;     ///< Current simulation time
  float dt;                  ///< Time step
  float G;                   ///< Gravitational constant
  float softening;           ///< Softening parameter
  uint32_t force_method;     ///< Force calculation method
  uint32_t reserved[4];      ///< Reserved for future use
};

/**
 * @brief Binary serializer for simulation state checkpoints.
 *
 * Provides static methods for saving and loading simulation state
 * in the internal .nbody binary checkpoint format.
 *
 * Usage:
 * @code
 * // Save checkpoint
 * Serializer::save("checkpoint.nbody", system.getState());
 *
 * // Load checkpoint
 * SimulationState state = Serializer::load("checkpoint.nbody");
 * @endcode
 *
 * For HDF5 interoperability (when compiled with NBODY_WITH_HDF5),
 * use the HDF5 serializer instead.
 */
class Serializer {
public:
  /**
   * @brief Save simulation state to a binary checkpoint file.
   * @param filename Path to the checkpoint file.
   * @param state Simulation state to save.
   * @throws std::runtime_error if file cannot be opened for writing.
   */
  static void save(const std::string& filename, const SimulationState& state);

  /**
   * @brief Load simulation state from a binary checkpoint file.
   * @param filename Path to the checkpoint file.
   * @return The loaded simulation state.
   * @throws std::runtime_error if file cannot be opened or is invalid.
   */
  static SimulationState load(const std::string& filename);

  /**
   * @brief Save simulation state to an output stream.
   * @param out Output stream to write to.
   * @param state Simulation state to save.
   */
  static void save(std::ostream& out, const SimulationState& state);

  /**
   * @brief Load simulation state from an input stream.
   * @param in Input stream to read from.
   * @return The loaded simulation state.
   * @throws std::runtime_error if stream is invalid or corrupted.
   */
  static SimulationState load(std::istream& in);

  /**
   * @brief Validate a checkpoint file format.
   * @param filename Path to the checkpoint file.
   * @return true if the file has valid magic number and version.
   */
  static bool validateFile(const std::string& filename);

  /**
   * @brief Validate a checkpoint stream format.
   * @param in Input stream to validate.
   * @return true if the stream has valid magic number and version.
   */
  static bool validateStream(std::istream& in);

private:
  static void writeHeader(std::ostream& out, const SimulationState& state);
  static FileHeader readHeader(std::istream& in);
  static void writeFloatArray(std::ostream& out, const std::vector<float>& data);
  static std::vector<float> readFloatArray(std::istream& in, size_t count);
};

}  // namespace nbody
