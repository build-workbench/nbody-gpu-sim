#include "nbody/hdf5_io.hpp"

#if NBODY_WITH_HDF5

#include <H5Cpp.h>
#include <stdexcept>

namespace nbody {

void HDF5IO::exportToFile(const std::string& filename, const SimulationState& state) {
  // Create HDF5 file
  H5::H5File file(filename, H5F_ACC_TRUNC);

  // Create groups
  H5::Group particles_group = file.createGroup("/particles");

  // Dataset dimensions
  hsize_t dims_scalar[1] = {state.particle_count};
  hsize_t dims_vector[1] = {state.particle_count * 3};

  // Create dataspace for scalar data (mass)
  H5::DataSpace scalar_dataspace(1, dims_scalar);

  // Create dataspace for vector data (position, velocity)
  H5::DataSpace vector_dataspace(1, dims_vector);

  // Write position data (interleaved x, y, z)
  {
    std::vector<float> pos_interleaved(state.particle_count * 3);
    for (size_t i = 0; i < state.particle_count; i++) {
      pos_interleaved[i * 3 + 0] = state.pos_x[i];
      pos_interleaved[i * 3 + 1] = state.pos_y[i];
      pos_interleaved[i * 3 + 2] = state.pos_z[i];
    }
    H5::DataSet pos_dataset =
        particles_group.createDataSet("position", H5::PredType::NATIVE_FLOAT, vector_dataspace);
    pos_dataset.write(pos_interleaved.data(), H5::PredType::NATIVE_FLOAT);
  }

  // Write velocity data (interleaved vx, vy, vz)
  {
    std::vector<float> vel_interleaved(state.particle_count * 3);
    for (size_t i = 0; i < state.particle_count; i++) {
      vel_interleaved[i * 3 + 0] = state.vel_x[i];
      vel_interleaved[i * 3 + 1] = state.vel_y[i];
      vel_interleaved[i * 3 + 2] = state.vel_z[i];
    }
    H5::DataSet vel_dataset =
        particles_group.createDataSet("velocity", H5::PredType::NATIVE_FLOAT, vector_dataspace);
    vel_dataset.write(vel_interleaved.data(), H5::PredType::NATIVE_FLOAT);
  }

  // Write mass data
  {
    H5::DataSet mass_dataset =
        particles_group.createDataSet("mass", H5::PredType::NATIVE_FLOAT, scalar_dataspace);
    mass_dataset.write(state.mass.data(), H5::PredType::NATIVE_FLOAT);
  }

  // Write metadata as attributes
  H5::Group metadata_group = file.createGroup("/metadata");

  {
    H5::DataSpace attr_space(H5S_SCALAR);
    H5::Attribute sim_time_attr =
        metadata_group.createAttribute("simulation_time", H5::PredType::NATIVE_FLOAT, attr_space);
    sim_time_attr.write(H5::PredType::NATIVE_FLOAT, &state.simulation_time);

    H5::Attribute dt_attr =
        metadata_group.createAttribute("dt", H5::PredType::NATIVE_FLOAT, attr_space);
    dt_attr.write(H5::PredType::NATIVE_FLOAT, &state.dt);

    H5::Attribute G_attr =
        metadata_group.createAttribute("G", H5::PredType::NATIVE_FLOAT, attr_space);
    G_attr.write(H5::PredType::NATIVE_FLOAT, &state.G);

    H5::Attribute softening_attr =
        metadata_group.createAttribute("softening", H5::PredType::NATIVE_FLOAT, attr_space);
    softening_attr.write(H5::PredType::NATIVE_FLOAT, &state.softening);

    int force_method_int = static_cast<int>(state.force_method);
    H5::Attribute force_method_attr =
        metadata_group.createAttribute("force_method", H5::PredType::NATIVE_INT, attr_space);
    force_method_attr.write(H5::PredType::NATIVE_INT, &force_method_int);

    long long particle_count_ll = static_cast<long long>(state.particle_count);
    H5::Attribute count_attr =
        metadata_group.createAttribute("particle_count", H5::PredType::NATIVE_LLONG, attr_space);
    count_attr.write(H5::PredType::NATIVE_LLONG, &particle_count_ll);
  }

  file.close();
}

SimulationState HDF5IO::importFromFile(const std::string& filename) {
  H5::H5File file(filename, H5F_ACC_RDONLY);

  SimulationState state;

  // Read metadata
  H5::Group metadata_group = file.openGroup("/metadata");

  {
    H5::Attribute sim_time_attr = metadata_group.openAttribute("simulation_time");
    sim_time_attr.read(H5::PredType::NATIVE_FLOAT, &state.simulation_time);

    H5::Attribute dt_attr = metadata_group.openAttribute("dt");
    dt_attr.read(H5::PredType::NATIVE_FLOAT, &state.dt);

    H5::Attribute G_attr = metadata_group.openAttribute("G");
    G_attr.read(H5::PredType::NATIVE_FLOAT, &state.G);

    H5::Attribute softening_attr = metadata_group.openAttribute("softening");
    softening_attr.read(H5::PredType::NATIVE_FLOAT, &state.softening);

    int force_method_int;
    H5::Attribute force_method_attr = metadata_group.openAttribute("force_method");
    force_method_attr.read(H5::PredType::NATIVE_INT, &force_method_int);
    state.force_method = static_cast<ForceMethod>(force_method_int);

    long long particle_count_ll;
    H5::Attribute count_attr = metadata_group.openAttribute("particle_count");
    count_attr.read(H5::PredType::NATIVE_LLONG, &particle_count_ll);
    state.particle_count = static_cast<size_t>(particle_count_ll);
  }

  // Resize arrays
  state.pos_x.resize(state.particle_count);
  state.pos_y.resize(state.particle_count);
  state.pos_z.resize(state.particle_count);
  state.vel_x.resize(state.particle_count);
  state.vel_y.resize(state.particle_count);
  state.vel_z.resize(state.particle_count);
  state.mass.resize(state.particle_count);

  // Read particle data
  H5::Group particles_group = file.openGroup("/particles");

  // Read position data
  {
    std::vector<float> pos_interleaved(state.particle_count * 3);
    H5::DataSet pos_dataset = particles_group.openDataSet("position");
    pos_dataset.read(pos_interleaved.data(), H5::PredType::NATIVE_FLOAT);
    for (size_t i = 0; i < state.particle_count; i++) {
      state.pos_x[i] = pos_interleaved[i * 3 + 0];
      state.pos_y[i] = pos_interleaved[i * 3 + 1];
      state.pos_z[i] = pos_interleaved[i * 3 + 2];
    }
  }

  // Read velocity data
  {
    std::vector<float> vel_interleaved(state.particle_count * 3);
    H5::DataSet vel_dataset = particles_group.openDataSet("velocity");
    vel_dataset.read(vel_interleaved.data(), H5::PredType::NATIVE_FLOAT);
    for (size_t i = 0; i < state.particle_count; i++) {
      state.vel_x[i] = vel_interleaved[i * 3 + 0];
      state.vel_y[i] = vel_interleaved[i * 3 + 1];
      state.vel_z[i] = vel_interleaved[i * 3 + 2];
    }
  }

  // Read mass data
  {
    H5::DataSet mass_dataset = particles_group.openDataSet("mass");
    mass_dataset.read(state.mass.data(), H5::PredType::NATIVE_FLOAT);
  }

  file.close();
  return state;
}

bool HDF5IO::validateFile(const std::string& filename) {
  try {
    H5::H5File file(filename, H5F_ACC_RDONLY);

    // Check required groups exist
    if (!file.exists("particles") || !file.exists("metadata")) {
      return false;
    }

    H5::Group particles = file.openGroup("particles");
    if (!particles.exists("position") || !particles.exists("velocity") ||
        !particles.exists("mass")) {
      return false;
    }

    return true;
  } catch (...) {
    return false;
  }
}

}  // namespace nbody

#endif  // NBODY_WITH_HDF5
