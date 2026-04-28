/**
 * @file particle_system.hpp
 * @brief Main particle system class for N-Body simulation
 *
 * This header defines the ParticleSystem class, which is the primary
 * interface for running N-Body simulations. It orchestrates all components:
 * - Particle data management
 * - Force calculation (via ForceCalculator)
 * - Time integration (via Integrator)
 * - CUDA-OpenGL interoperability
 * - State serialization
 *
 * @author N-Body Simulation Team
 * @version 2.0.0
 */

#pragma once

#include "nbody/force_calculator.hpp"
#include "nbody/integrator.hpp"
#include "nbody/particle_data.hpp"
#include "nbody/simulation_state.hpp"
#include "nbody/types.hpp"
#include <memory>
#include <string>

namespace nbody {

/**
 * @class ParticleSystem
 * @brief Main class managing the N-Body particle simulation
 *
 * ParticleSystem is the central orchestrator for the simulation,
 * coordinating particle data, force calculation, time integration,
 * and rendering interoperability.
 *
 * @section thread_safety Thread Safety
 *
 * **ParticleSystem is NOT thread-safe.** The following operations must be
 * synchronized by the caller if accessed from multiple threads:
 * - All non-const methods (initialize, update, setForceMethod, etc.)
 * - All const methods that access GPU resources (getState, computeEnergy, etc.)
 *
 * Const getters (getParticleCount, getTimeStep, etc.) are thread-safe
 * if no concurrent modifications are occurring.
 *
 * CUDA operations are inherently asynchronous. The class internally
 * synchronizes when necessary, but callers should not assume any
 * particular synchronization behavior.
 *
 * OpenGL interoperability requires special care: the interop buffer
 * must be unmapped before any OpenGL rendering, and mapped before
 * CUDA operations. This is handled automatically by update() and render().
 *
 * ## Basic Usage
 * @code{.cpp}
 * // Configure simulation
 * SimulationConfig config;
 * config.particle_count = 100000;
 * config.force_method = ForceMethod::BARNES_HUT;
 * config.dt = 0.001f;
 *
 * // Initialize and run
 * ParticleSystem system;
 * system.initialize(config);
 *
 * for (int i = 0; i < 1000; i++) {
 *     system.update(system.getTimeStep());
 * }
 * @endcode
 *
 * ## Algorithm Switching
 * @code{.cpp}
 * // Switch algorithms at runtime
 * system.setForceMethod(ForceMethod::SPATIAL_HASH);
 * system.setSpatialHashCutoff(2.0f);
 * @endcode
 *
 * ## State Persistence
 * @code{.cpp}
 * // Save simulation state
 * system.saveState("checkpoint.nbody");
 *
 * // Load simulation state
 * ParticleSystem loaded_system;
 * loaded_system.loadState("checkpoint.nbody");
 * @endcode
 *
 * @see SimulationConfig for configuration options
 * @see ForceCalculator for algorithm details
 * @see Integrator for integration methods
 */
class ParticleSystem {
public:
  /**
   * @brief Construct an uninitialized particle system
   *
   * The system must be initialized with initialize() before use.
   */
  ParticleSystem();

  /**
   * @brief Destructor - releases all GPU resources
   */
  ~ParticleSystem();

  // Non-copyable (owns unique GPU resources)
  ParticleSystem(const ParticleSystem&) = delete;
  ParticleSystem& operator=(const ParticleSystem&) = delete;

  // Movable
  ParticleSystem(ParticleSystem&&) noexcept = default;
  ParticleSystem& operator=(ParticleSystem&&) noexcept = default;

  // =========================================================================
  // Initialization
  // =========================================================================

  /**
   * @brief Initialize the particle system with given configuration
   *
   * Allocates GPU memory, initializes particle positions according to
   * the specified distribution, and sets up force calculator and integrator.
   *
   * @param config Simulation configuration parameters
   * @throws ValidationException if config parameters are invalid
   * @throws ResourceException if insufficient GPU memory
   * @throws CudaException on CUDA errors
   *
   * @code{.cpp}
   * SimulationConfig config;
   * config.particle_count = 50000;
   * config.force_method = ForceMethod::BARNES_HUT;
   *
   * ParticleSystem system;
   * system.initialize(config);
   * @endcode
   */
  void initialize(const SimulationConfig& config);

  /**
   * @brief Initialize with a specific distribution
   *
   * Convenience method that creates a default config and sets
   * particle count and distribution type.
   *
   * @param particle_count Number of particles
   * @param dist Distribution pattern for initial positions
   */
  void initializeWithDistribution(size_t particle_count, InitDistribution dist);

  // =========================================================================
  // Simulation Control
  // =========================================================================

  /**
   * @brief Advance simulation by one time step
   *
   * Performs one complete Velocity Verlet integration step:
   * 1. Store old accelerations
   * 2. Update positions
   * 3. Compute new forces
   * 4. Update velocities
   *
   * @param dt Time step size (typically from getTimeStep())
   *
   * @note Does nothing if system is paused or not initialized
   *
   * @see Integrator::integrate()
   */
  void update(float dt);

  /**
   * @brief Pause the simulation
   *
   * Paused simulations do not advance during update() calls.
   * Energy calculations remain available while paused.
   */
  void pause() { is_paused_ = true; }

  /**
   * @brief Resume a paused simulation
   */
  void resume() { is_paused_ = false; }

  /**
   * @brief Reset simulation to initial state
   *
   * Re-initializes particles using the original configuration.
   * Simulation time is reset to zero.
   */
  void reset();

  /**
   * @brief Check if simulation is paused
   * @return true if paused, false otherwise
   */
  bool isPaused() const noexcept { return is_paused_; }

  // =========================================================================
  // Parameter Adjustment
  // =========================================================================

  /**
   * @brief Switch force calculation algorithm
   *
   * Can be called at any time during simulation. The new algorithm
   * will be used for subsequent force calculations.
   *
   * @param method Force calculation algorithm to use
   */
  void setForceMethod(ForceMethod method);

  /**
   * @brief Set gravitational constant G
   * @param G New gravitational constant (must be positive)
   * @throws ValidationException if G is not positive and finite
   */
  void setGravitationalConstant(float G);

  /**
   * @brief Set softening parameter
   * @param eps Softening length (prevents singularities at close range)
   * @throws ValidationException if eps is negative
   */
  void setSofteningParameter(float eps);

  /**
   * @brief Set time step
   * @param dt Time step for integration
   * @throws ValidationException if dt is not positive or > 1.0
   */
  void setTimeStep(float dt);

  /**
   * @brief Set Barnes-Hut accuracy parameter
   * @param theta Opening angle threshold (0.3 = accurate, 1.0 = fast)
   * @throws ValidationException if theta < 0 or > 2
   */
  void setBarnesHutTheta(float theta);

  /**
   * @brief Set spatial hash cell size
   * @param size Cell size for spatial hashing
   * @throws ValidationException if size is not positive
   */
  void setSpatialHashCellSize(float size);

  /**
   * @brief Set spatial hash cutoff radius
   * @param cutoff Maximum interaction distance
   * @throws ValidationException if cutoff is not positive
   */
  void setSpatialHashCutoff(float cutoff);

  // =========================================================================
  // Getters
  // =========================================================================

  /** @brief Get current force calculation method */
  ForceMethod getForceMethod() const noexcept { return force_method_; }

  /** @brief Get gravitational constant */
  float getGravitationalConstant() const noexcept { return G_; }

  /** @brief Get softening parameter */
  float getSofteningParameter() const noexcept { return softening_; }

  /** @brief Get current time step */
  float getTimeStep() const noexcept { return dt_; }

  /** @brief Get elapsed simulation time */
  float getSimulationTime() const noexcept { return simulation_time_; }

  /** @brief Get number of particles */
  size_t getParticleCount() const noexcept { return particle_count_; }

  // =========================================================================
  // Data Access
  // =========================================================================

  /**
   * @brief Get device (GPU) particle data
   * @return Pointer to device particle data structure
   * @warning Data is on GPU - do not access directly from CPU
   */
  ParticleData* getDeviceData() { return &d_particles_; }

  /** @brief Get const device particle data */
  const ParticleData* getDeviceData() const { return &d_particles_; }

  /**
   * @brief Copy particle data to host memory
   * @param h_particles Host particle data structure to receive data
   *
   * @code{.cpp}
   * ParticleData host_data;
   * ParticleDataManager::allocateHost(host_data, system.getParticleCount());
   * system.copyToHost(host_data);
   * // Access host_data.pos_x[i], etc.
   * ParticleDataManager::freeHost(host_data);
   * @endcode
   */
  void copyToHost(ParticleData& h_particles) const;

  // =========================================================================
  // State Management
  // =========================================================================

  /**
   * @brief Save simulation state to file
   * @param filename Output file path
   *
   * Saves all particle data and simulation parameters to a binary file.
   * Can be loaded later to resume simulation.
   */
  void saveState(const std::string& filename) const;

  /**
   * @brief Load simulation state from file
   * @param filename Input file path
   * @throws std::runtime_error if file cannot be read or is invalid
   */
  void loadState(const std::string& filename);

  /**
   * @brief Get current simulation state
   * @return SimulationState containing all particle data and parameters
   */
  SimulationState getState() const;

  /**
   * @brief Set simulation state
   * @param state State to restore
   *
   * Reinitializes the system with the given state, including all
   * particle positions, velocities, and parameters.
   */
  void setState(const SimulationState& state);

  // =========================================================================
  // Energy Calculations
  // =========================================================================

  /**
   * @brief Compute total kinetic energy
   * @return Sum of 0.5 * m * v² for all particles
   */
  float computeKineticEnergy() const;

  /**
   * @brief Compute total potential energy
   * @return Gravitational potential energy (negative)
   */
  float computePotentialEnergy() const;

  /**
   * @brief Compute total mechanical energy
   * @return KE + PE (conserved quantity for isolated system)
   */
  float computeTotalEnergy() const;

  // =========================================================================
  // CUDA-GL Interop
  // =========================================================================

  /**
   * @brief Get CUDA-OpenGL interoperability object
   * @return Pointer to interop object (null if not initialized)
   */
  CudaGLInterop* getInterop() noexcept { return interop_.get(); }

  /**
   * @brief Get const CUDA-OpenGL interoperability object
   */
  const CudaGLInterop* getInterop() const noexcept { return interop_.get(); }

  /**
   * @brief Initialize CUDA-OpenGL interoperability
   *
   * Creates shared buffer for particle positions that can be
   * directly rendered by OpenGL without CPU transfer.
   * Must be called after initialize() and before rendering.
   */
  void initializeInterop();

  /**
   * @brief Update interop buffer with current positions
   *
   * Copies particle positions to the shared VBO for rendering.
   * Called automatically during update() if interop is initialized.
   */
  void updateInteropBuffer();

private:
  // Particle data
  ParticleData d_particles_;  ///< Device (GPU) particle memory
  ParticleData h_particles_;  ///< Host (CPU) particle memory
  size_t particle_count_;     ///< Number of particles

  // Components
  std::unique_ptr<ForceCalculator> force_calculator_;
  std::unique_ptr<Integrator> integrator_;
  std::unique_ptr<CudaGLInterop> interop_;

  // Simulation parameters
  float dt_;                  ///< Time step
  float G_;                   ///< Gravitational constant
  float softening_;           ///< Softening length
  float simulation_time_;     ///< Elapsed simulation time
  ForceMethod force_method_;  ///< Current force algorithm
  bool is_paused_;            ///< Pause state
  bool is_initialized_;       ///< Initialization state

  // Configuration
  SimulationConfig config_;  ///< Original configuration

  // Internal methods
  void allocateMemory(size_t count);
  void freeMemory();
  void createForceCalculator();
};

}  // namespace nbody
