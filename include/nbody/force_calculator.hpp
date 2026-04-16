/**
 * @file force_calculator.hpp
 * @brief Force calculation strategies for N-Body simulation
 *
 * Defines abstract interface and concrete implementations for computing
 * gravitational forces between particles.
 *
 * @section force_thread_safety Thread Safety
 *
 * **ForceCalculator and its subclasses are NOT thread-safe.**
 *
 * - computeForces() modifies GPU particle data and should not be called
 *   concurrently from multiple threads.
 * - Setters (setTheta, setCellSize, etc.) are not atomic.
 * - Const getters are thread-safe if no concurrent modifications occur.
 *
 * CUDA kernels are launched asynchronously. Synchronization with the host
 * is handled internally where necessary (e.g., for CPU tree building in
 * Barnes-Hut).
 */

#pragma once

#include "nbody/types.hpp"
#include <memory>

namespace nbody {

/**
 * @class ForceCalculator
 * @brief Abstract base class for force calculation algorithms
 *
 * Provides a common interface for different force calculation strategies.
 * All implementations compute gravitational acceleration for each particle.
 */
class ForceCalculator {
public:
  virtual ~ForceCalculator() = default;

  /**
   * @brief Compute gravitational accelerations for all particles
   *
   * Calculates the gravitational force on each particle due to all other
   * particles and stores the resulting acceleration in acc_x, acc_y, acc_z.
   *
   * @param d_particles Device particle data (accelerations are overwritten)
   */
  virtual void computeForces(ParticleData* d_particles) = 0;

  /**
   * @brief Get the force calculation method type
   * @return ForceMethod enum value identifying this algorithm
   */
  virtual ForceMethod getMethod() const = 0;

  /**
   * @brief Set softening parameter for force calculation
   *
   * Softening prevents singularities when particles get very close.
   * The effective force is computed using (r² + eps²) instead of r².
   *
   * @param eps Softening length (default: 0.01)
   */
  void setSofteningParameter(float eps) {
    softening_eps_ = eps;
    softening_eps2_ = eps * eps;
  }

  /**
   * @brief Set gravitational constant
   * @param G Gravitational constant (default: 1.0 for normalized units)
   */
  void setGravitationalConstant(float G) { G_ = G; }

  /**
   * @brief Get current softening parameter
   */
  float getSofteningParameter() const noexcept { return softening_eps_; }

  /**
   * @brief Get current gravitational constant
   */
  float getGravitationalConstant() const noexcept { return G_; }

protected:
  float softening_eps_ = 0.01f;     ///< Softening length
  float softening_eps2_ = 0.0001f;  ///< Pre-computed eps² for efficiency
  float G_ = 1.0f;                  ///< Gravitational constant
};

/**
 * @class DirectForceCalculator
 * @brief Direct O(N²) force calculation
 *
 * Computes forces by evaluating all N(N-1)/2 pairwise interactions.
 * Most accurate but scales poorly with particle count.
 * Suitable for N < 10,000 particles.
 */
class DirectForceCalculator : public ForceCalculator {
public:
  /**
   * @brief Construct direct force calculator
   * @param block_size CUDA thread block size (default: 256)
   */
  explicit DirectForceCalculator(int block_size = 256);

  void computeForces(ParticleData* d_particles) override;
  ForceMethod getMethod() const noexcept override { return ForceMethod::DIRECT_N2; }

  /**
   * @brief Set CUDA block size for kernel launches
   */
  void setBlockSize(int size) { block_size_ = size; }

  /**
   * @brief Get current CUDA block size
   */
  int getBlockSize() const noexcept { return block_size_; }

private:
  int block_size_;
};

/**
 * @class BarnesHutCalculator
 * @brief Barnes-Hut O(N log N) force calculation
 *
 * Uses an octree to approximate distant particle groups as single masses.
 * Accuracy controlled by theta parameter (smaller = more accurate, slower).
 * Suitable for N = 10,000 to 10,000,000 particles.
 */
class BarnesHutCalculator : public ForceCalculator {
public:
  /**
   * @brief Construct Barnes-Hut force calculator
   * @param theta Opening angle threshold (default: 0.5)
   *               - theta < 0.5: high accuracy, slower
   *               - theta = 0.5-1.0: balanced
   *               - theta > 1.0: fast, lower accuracy
   */
  explicit BarnesHutCalculator(float theta = 0.5f);
  ~BarnesHutCalculator();

  void computeForces(ParticleData* d_particles) override;
  ForceMethod getMethod() const noexcept override { return ForceMethod::BARNES_HUT; }

  /**
   * @brief Set opening angle threshold
   * @param theta New threshold value
   */
  void setTheta(float theta) { theta_ = theta; }

  /**
   * @brief Get current opening angle threshold
   */
  float getTheta() const noexcept { return theta_; }

  /**
   * @brief Get the Barnes-Hut octree (for testing/debugging)
   * @return Pointer to internal tree structure
   */
  BarnesHutTree* getTree() noexcept { return tree_.get(); }

private:
  std::unique_ptr<BarnesHutTree> tree_;
  float theta_;
};

/**
 * @class SpatialHashCalculator
 * @brief Spatial Hash O(N) force calculation for short-range forces
 *
 * Uses a uniform grid to find nearby particles efficiently.
 * Only computes interactions within the cutoff radius.
 * Suitable for systems with natural cutoff (e.g., SPH, short-range gravity).
 */
class SpatialHashCalculator : public ForceCalculator {
public:
  /**
   * @brief Construct spatial hash force calculator
   * @param cell_size Size of grid cells (default: 1.0)
   * @param cutoff_radius Maximum interaction distance (default: 2.0)
   */
  SpatialHashCalculator(float cell_size = 1.0f, float cutoff_radius = 2.0f);
  ~SpatialHashCalculator();

  void computeForces(ParticleData* d_particles) override;
  ForceMethod getMethod() const noexcept override { return ForceMethod::SPATIAL_HASH; }

  /**
   * @brief Set grid cell size
   * @param size Cell size (typically similar to cutoff radius)
   */
  void setCellSize(float size) { cell_size_ = size; }

  /**
   * @brief Set cutoff radius for force calculation
   * @param radius Maximum interaction distance
   */
  void setCutoffRadius(float radius) { cutoff_radius_ = radius; }

  /**
   * @brief Get current cell size
   */
  float getCellSize() const noexcept { return cell_size_; }

  /**
   * @brief Get current cutoff radius
   */
  float getCutoffRadius() const noexcept { return cutoff_radius_; }

  /**
   * @brief Get the spatial hash grid (for testing/debugging)
   * @return Pointer to internal grid structure
   */
  SpatialHashGrid* getGrid() noexcept { return grid_.get(); }

private:
  std::unique_ptr<SpatialHashGrid> grid_;
  float cell_size_;
  float cutoff_radius_;
};

/**
 * @brief Factory function to create force calculator
 * @param method Force calculation algorithm to use
 * @param config Simulation configuration with algorithm parameters
 * @return Unique pointer to the created force calculator
 */
std::unique_ptr<ForceCalculator> createForceCalculator(ForceMethod method,
                                                       const SimulationConfig& config);

/**
 * @brief CPU reference implementation of gravitational force
 *
 * Used for testing and validation of GPU implementations.
 *
 * @param p1 Position of first particle
 * @param p2 Position of second particle
 * @param m1 Mass of first particle
 * @param m2 Mass of second particle
 * @param G Gravitational constant
 * @param eps Softening parameter
 * @return Force vector on particle 1 due to particle 2
 */
Vec3 computeGravitationalForceCPU(const Vec3& p1, const Vec3& p2, float m1, float m2, float G,
                                  float eps);

}  // namespace nbody
