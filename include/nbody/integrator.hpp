#pragma once

#include "nbody/types.hpp"

namespace nbody {

// Velocity Verlet integrator
class Integrator {
public:
  Integrator(int block_size = 256);
  ~Integrator();

  // Full integration step (position update, force calculation, velocity update)
  // Note: Force calculation is done externally between position and velocity
  // updates
  void integrate(ParticleData *d_particles, ForceCalculator *force_calc,
                 float dt);

  // Individual steps for more control
  void updatePositions(ParticleData *d_particles, float dt);
  void updateVelocities(ParticleData *d_particles, float dt);

  // Store current accelerations as old accelerations (for Velocity Verlet)
  void storeOldAccelerations(ParticleData *d_particles);

  // Compute total kinetic energy
  float computeKineticEnergy(const ParticleData *d_particles);

  // Compute total potential energy (requires force calculator)
  float computePotentialEnergy(const ParticleData *d_particles, float G,
                               float eps);

  // Compute total energy
  float computeTotalEnergy(const ParticleData *d_particles, float G, float eps);

  // Pre-allocate scratch buffer for reduction (call after particle count is
  // known)
  void ensureScratchBuffer(size_t particle_count);

  void setBlockSize(int size) { block_size_ = size; }
  int getBlockSize() const { return block_size_; }

private:
  int block_size_;
  float *d_scratch_ = nullptr; // Pre-allocated scratch for reductions
  int scratch_blocks_ = 0;     // Number of blocks in scratch buffer
};

// GPU kernel declarations (implemented in .cu file)
void launchUpdatePositionsKernel(ParticleData *d_particles, float dt,
                                 int block_size);
void launchUpdateVelocitiesKernel(ParticleData *d_particles, float dt,
                                  int block_size);
void launchStoreAccelerationsKernel(ParticleData *d_particles, int block_size);
float launchComputeKineticEnergyKernel(const ParticleData *d_particles,
                                       int block_size);
float launchComputePotentialEnergyKernel(const ParticleData *d_particles,
                                         float G, float eps, int block_size);

} // namespace nbody
