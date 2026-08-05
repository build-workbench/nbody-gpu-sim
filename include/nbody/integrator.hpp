#pragma once

#include "nbody/types.hpp"

namespace nbody {

// Velocity Verlet (symplectic) integrator — good energy conservation.
class Integrator {
public:
  explicit Integrator(int block_size = 256);
  ~Integrator();

  // Non-copyable, non-movable: owns a raw device scratch buffer.
  Integrator(const Integrator&) = delete;
  Integrator& operator=(const Integrator&) = delete;
  Integrator(Integrator&&) = delete;
  Integrator& operator=(Integrator&&) = delete;

  void integrate(ParticleData* d_particles, ForceCalculator* force_calc, float dt);
  void updatePositions(ParticleData* d_particles, float dt);
  void updateVelocities(ParticleData* d_particles, float dt);
  void storeOldAccelerations(ParticleData* d_particles);

  float computeKineticEnergy(const ParticleData* d_particles);
  float computePotentialEnergy(const ParticleData* d_particles, float G, float eps);
  float computeTotalEnergy(const ParticleData* d_particles, float G, float eps);

  void ensureScratchBuffer(size_t particle_count);
  void setBlockSize(int size) { block_size_ = size; }
  int getBlockSize() const noexcept { return block_size_; }

private:
  int block_size_;
  float* d_scratch_ = nullptr;
  int scratch_blocks_ = 0;
};

void launchUpdatePositionsKernel(ParticleData* d_particles, float dt, int block_size);
void launchUpdateVelocitiesKernel(ParticleData* d_particles, float dt, int block_size);
void launchStoreAccelerationsKernel(ParticleData* d_particles, int block_size);

}  // namespace nbody
