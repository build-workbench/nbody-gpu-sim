/**
 * @file integrator.hpp
 * @brief Velocity Verlet integrator for N-Body simulation
 *
 * Implements the symplectic Velocity Verlet integration scheme,
 * which provides good energy conservation for gravitational systems.
 *
 * @section integrator_thread_safety Thread Safety
 *
 * **Integrator is NOT thread-safe.** All methods modify GPU state and
 * should not be called concurrently from multiple threads.
 *
 * The class maintains internal scratch buffers on the GPU that are
 * reused across calls for efficiency. These buffers are not protected
 * against concurrent access.
 */

#pragma once

#include "nbody/types.hpp"

namespace nbody {

/**
 * @class Integrator
 * @brief Velocity Verlet integrator for particle dynamics
 *
 * Implements a symplectic integrator that conserves energy well for
 * gravitational N-body simulations. The Velocity Verlet scheme is:
 *
 * 1. x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt²
 * 2. Compute a(t+dt) from new positions
 * 3. v(t+dt) = v(t) + 0.5*(a(t) + a(t+dt))*dt
 */
class Integrator {
public:
  /**
   * @brief Construct an integrator with specified CUDA block size
   * @param block_size CUDA thread block size for kernel launches (default: 256)
   */
  explicit Integrator(int block_size = 256);

  /**
   * @brief Destructor - releases GPU scratch buffer
   */
  ~Integrator();

  /**
   * @brief Perform a complete Velocity Verlet integration step
   *
   * Executes the full integration cycle:
   * 1. Store old accelerations
   * 2. Update positions using current velocities and accelerations
   * 3. Compute new forces via the force calculator
   * 4. Update velocities using average of old and new accelerations
   *
   * @param d_particles Device particle data (modified in place)
   * @param force_calc Force calculator to use for acceleration computation
   * @param dt Time step size
   *
   * @note Force calculation is performed internally as part of the step
   */
  void integrate(ParticleData* d_particles, ForceCalculator* force_calc, float dt);

  /**
   * @brief Update particle positions only (half-step)
   *
   * Implements: x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt²
   *
   * @param d_particles Device particle data (modified in place)
   * @param dt Time step size
   */
  void updatePositions(ParticleData* d_particles, float dt);

  /**
   * @brief Update particle velocities only (half-step)
   *
   * Implements: v(t+dt) = v(t) + 0.5*(a_old + a_new)*dt
   * Requires storeOldAccelerations() to have been called before force update.
   *
   * @param d_particles Device particle data (modified in place)
   * @param dt Time step size
   */
  void updateVelocities(ParticleData* d_particles, float dt);

  /**
   * @brief Store current accelerations as old accelerations
   *
   * Copies current accelerations to acc_old_* arrays for use in
   * the velocity update step of Velocity Verlet integration.
   *
   * @param d_particles Device particle data
   */
  void storeOldAccelerations(ParticleData* d_particles);

  /**
   * @brief Compute total kinetic energy of the system
   *
   * @param d_particles Device particle data
   * @return Sum of 0.5 * m * v² for all particles
   */
  float computeKineticEnergy(const ParticleData* d_particles);

  /**
   * @brief Compute total gravitational potential energy
   *
   * @param d_particles Device particle data
   * @param G Gravitational constant
   * @param eps Softening parameter
   * @return Gravitational potential energy (negative for bound systems)
   */
  float computePotentialEnergy(const ParticleData* d_particles, float G, float eps);

  /**
   * @brief Compute total mechanical energy
   *
   * @param d_particles Device particle data
   * @param G Gravitational constant
   * @param eps Softening parameter
   * @return KE + PE (conserved quantity for isolated system)
   */
  float computeTotalEnergy(const ParticleData* d_particles, float G, float eps);

  /**
   * @brief Ensure scratch buffer is allocated for given particle count
   *
   * Pre-allocates GPU memory for parallel reduction operations.
   * Called automatically when needed.
   *
   * @param particle_count Number of particles in the system
   */
  void ensureScratchBuffer(size_t particle_count);

  /**
   * @brief Set CUDA block size for kernel launches
   * @param size Block size (typically 128-512)
   */
  void setBlockSize(int size) { block_size_ = size; }

  /**
   * @brief Get current CUDA block size
   * @return Block size used for kernel launches
   */
  int getBlockSize() const noexcept { return block_size_; }

private:
  int block_size_;
  float* d_scratch_ = nullptr;  ///< Pre-allocated scratch for reductions
  int scratch_blocks_ = 0;      ///< Number of blocks in scratch buffer
};

// GPU kernel declarations (implemented in .cu file)
void launchUpdatePositionsKernel(ParticleData* d_particles, float dt, int block_size);
void launchUpdateVelocitiesKernel(ParticleData* d_particles, float dt, int block_size);
void launchStoreAccelerationsKernel(ParticleData* d_particles, int block_size);
float launchComputeKineticEnergyKernel(const ParticleData* d_particles, int block_size);
float launchComputePotentialEnergyKernel(const ParticleData* d_particles, float G, float eps,
                                         int block_size);

}  // namespace nbody
