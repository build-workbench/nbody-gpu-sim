#pragma once

#include "nbody/simulation_state.hpp"
#include "nbody/types.hpp"
#include <string>

namespace nbody {

/**
 * @class HDF5IO
 * @brief HDF5-based import/export for scientific data interoperability
 *
 * Provides standards-compliant export and import of particle simulation
 * data in HDF5 format for use with external analysis tools (Python h5py,
 * MATLAB, ParaView, etc.).
 *
 * This class is only available when NBODY_WITH_HDF5 is enabled.
 *
 * @section hdf5_format HDF5 File Format
 *
 * The HDF5 files follow this structure:
 * @code
 * /particles/
 *   /position (Nx3 float32) - particle positions [x, y, z]
 *   /velocity (Nx3 float32) - particle velocities [vx, vy, vz]
 *   /mass (N float32) - particle masses
 * /metadata/
 *   attributes: simulation_time, dt, G, softening, force_method, particle_count
 * @endcode
 */
class HDF5IO {
public:
  /**
   * @brief Export simulation state to HDF5 file
   *
   * @param filename Output file path
   * @param state Simulation state to export
   * @throws std::runtime_error if file cannot be created or written
   */
  static void exportToFile(const std::string& filename, const SimulationState& state);

  /**
   * @brief Import simulation state from HDF5 file
   *
   * @param filename Input file path
   * @return SimulationState loaded from file
   * @throws std::runtime_error if file cannot be read or is invalid
   */
  static SimulationState importFromFile(const std::string& filename);

  /**
   * @brief Validate HDF5 file format
   *
   * @param filename File path to validate
   * @return true if file is a valid HDF5 file with expected structure
   */
  static bool validateFile(const std::string& filename);

private:
  static void writeMetadata(void* file_id, const SimulationState& state);
  static void readMetadata(void* file_id, SimulationState& state);
};

}  // namespace nbody
