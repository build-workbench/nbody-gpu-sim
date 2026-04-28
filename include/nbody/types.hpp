/**
 * @file types.hpp
 * @brief Core type definitions for the N-Body simulation system
 *
 * This header defines the fundamental types, enumerations, and data structures
 * used throughout the N-Body particle simulation system. It provides:
 * - Vector math types compatible with both host and device code
 * - Particle data structures optimized for GPU computation (SoA layout)
 * - Simulation and rendering configuration structures
 * - Enumeration types for algorithms and distributions
 *
 * @author N-Body Simulation Team
 * @version 2.0.0
 */

#pragma once

#include <cstddef>
#include <cmath>

#if defined(__CUDACC__)
#include <cuda_runtime.h>
#else
#ifndef __host__
#define __host__
#endif
#ifndef __device__
#define __device__
#endif
struct int3 {
  int x, y, z;
};
inline constexpr int3 make_int3(int x, int y, int z) {
  return {x, y, z};
}
#endif

namespace nbody {

// Forward declarations
struct ParticleData;
class ForceCalculator;
class Integrator;
class BarnesHutTree;
class SpatialHashGrid;
class CudaGLInterop;
class Renderer;
class Camera;
class ParticleSystem;

// ============================================================================
// Enumerations
// ============================================================================

/**
 * @enum ForceMethod
 * @brief Force calculation algorithm selection
 *
 * Specifies which algorithm to use for computing gravitational forces
 * between particles. Each method has different time complexity and
 * accuracy characteristics.
 *
 * @see ForceCalculator
 * @see createForceCalculator()
 */
enum class ForceMethod {
  DIRECT_N2,    ///< O(N²) direct pairwise calculation - exact but slow for large N
  BARNES_HUT,   ///< O(N log N) tree-based approximation - good balance for large N
  SPATIAL_HASH  ///< O(N) spatial hashing - optimal for short-range forces only
};

/**
 * @enum InitDistribution
 * @brief Initial particle distribution pattern
 *
 * Defines how particles are initially positioned in the simulation space.
 * Each distribution creates different initial conditions suitable for
 * various simulation scenarios.
 *
 * @see ParticleInitializer
 */
enum class InitDistribution {
  UNIFORM,    ///< Uniform random distribution within a bounding box
  SPHERICAL,  ///< Uniform distribution within a sphere (good for collapse simulations)
  DISK        ///< Flat disk distribution with initial rotation (galaxy formation)
};

/**
 * @enum ColorMode
 * @brief Particle coloring mode for visualization
 *
 * Determines how particle colors are computed during rendering.
 * Different modes highlight different physical properties.
 *
 * @see Renderer::setColorMode()
 */
enum class ColorMode {
  DEPTH,     ///< Color based on distance from camera (warm=close, cool=far)
  VELOCITY,  ///< Color based on velocity magnitude (blue=slow, red=fast)
  DENSITY    ///< Color based on local particle density
};

// ============================================================================
// Vector Types
// ============================================================================

/**
 * @struct Vec3
 * @brief 3D vector type compatible with both host and device code
 *
 * A simple 3D vector class that works in both CPU and CUDA code.
 * Provides basic vector operations needed for physics calculations.
 *
 * @note This type is preferred over float3 for consistency and
 *       to avoid CUDA-specific dependencies in host code.
 *
 * @code{.cpp}
 * Vec3 position(1.0f, 2.0f, 3.0f);
 * Vec3 velocity(0.1f, 0.0f, 0.0f);
 * Vec3 new_pos = position + velocity * dt;
 * @endcode
 */
struct Vec3 {
  float x, y, z;  ///< Vector components

  /**
   * @brief Default constructor - initializes to zero vector
   */
  __host__ __device__ Vec3() : x(0), y(0), z(0) {}

  /**
   * @brief Component-wise constructor
   * @param x_ X component
   * @param y_ Y component
   * @param z_ Z component
   */
  __host__ __device__ Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

  /// Vector addition
  __host__ __device__ Vec3 operator+(const Vec3& v) const {
    return Vec3(x + v.x, y + v.y, z + v.z);
  }

  /// Vector subtraction
  __host__ __device__ Vec3 operator-(const Vec3& v) const {
    return Vec3(x - v.x, y - v.y, z - v.z);
  }

  /// Scalar multiplication
  __host__ __device__ Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }

  /// Scalar division
  __host__ __device__ Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }

  /// In-place vector addition
  __host__ __device__ Vec3& operator+=(const Vec3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }

  /// In-place vector subtraction
  __host__ __device__ Vec3& operator-=(const Vec3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }

  /**
   * @brief Compute dot product with another vector
   * @param v Other vector
   * @return Dot product (scalar)
   */
  __host__ __device__ float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }

  /**
   * @brief Compute squared length (avoids sqrt)
   * @return Length squared
   */
  __host__ __device__ float length2() const { return dot(*this); }

  /**
   * @brief Compute vector length
   * @return Euclidean length
   */
  __host__ __device__ float length() const { return sqrtf(length2()); }

  /**
   * @brief Compute normalized (unit) vector
   * @return Unit vector in same direction, or zero if length is zero
   */
  __host__ __device__ Vec3 normalized() const {
    float l = length();
    return l > 0 ? *this / l : Vec3();
  }
};

/**
 * @brief Scalar * Vec3 multiplication (commutative)
 * @param s Scalar value
 * @param v Vector
 * @return Scaled vector
 */
__host__ __device__ inline Vec3 operator*(float s, const Vec3& v) {
  return v * s;
}

// ============================================================================
// Particle Data Structures
// ============================================================================

/**
 * @struct ParticleData
 * @brief Particle data in Structure of Arrays (SoA) layout
 *
 * Stores all particle data in separate arrays for each component,
 * which is optimal for GPU memory access patterns. This layout
 * enables coalesced memory access in CUDA kernels.
 *
 * @note Arrays are typically allocated on the device (GPU) for simulation,
 *       but host allocations are also supported for initialization and I/O.
 *
 * Memory per particle: 13 floats = 52 bytes
 * - Position: 3 floats
 * - Velocity: 3 floats
 * - Acceleration: 6 floats (current + old for Verlet)
 * - Mass: 1 float
 *
 * @see ParticleDataManager for memory allocation and transfer
 * @see ParticleInitializer for population methods
 */
struct ParticleData {
  // Position arrays (size: count)
  float* pos_x;  ///< X position components
  float* pos_y;  ///< Y position components
  float* pos_z;  ///< Z position components

  // Velocity arrays (size: count)
  float* vel_x;  ///< X velocity components
  float* vel_y;  ///< Y velocity components
  float* vel_z;  ///< Z velocity components

  // Acceleration arrays (size: count)
  float* acc_x;      ///< Current X acceleration
  float* acc_y;      ///< Current Y acceleration
  float* acc_z;      ///< Current Z acceleration
  float* acc_old_x;  ///< Previous X acceleration (for Verlet)
  float* acc_old_y;  ///< Previous Y acceleration (for Verlet)
  float* acc_old_z;  ///< Previous Z acceleration (for Verlet)

  // Mass array (size: count)
  float* mass;  ///< Particle masses

  size_t count;  ///< Number of particles

  /**
   * @brief Default constructor - initializes all pointers to null
   */
  ParticleData()
      : pos_x(nullptr),
        pos_y(nullptr),
        pos_z(nullptr),
        vel_x(nullptr),
        vel_y(nullptr),
        vel_z(nullptr),
        acc_x(nullptr),
        acc_y(nullptr),
        acc_z(nullptr),
        acc_old_x(nullptr),
        acc_old_y(nullptr),
        acc_old_z(nullptr),
        mass(nullptr),
        count(0) {}
};

// ============================================================================
// Configuration Structures
// ============================================================================

/**
 * @struct SimulationConfig
 * @brief Complete configuration for a simulation run
 *
 * Contains all parameters needed to initialize and run an N-Body simulation.
 * Default values are chosen for reasonable performance and accuracy balance.
 *
 * @code{.cpp}
 * SimulationConfig config;
 * config.particle_count = 100000;
 * config.force_method = ForceMethod::BARNES_HUT;
 * config.dt = 0.001f;
 *
 * ParticleSystem system;
 * system.initialize(config);
 * @endcode
 *
 * @see ParticleSystem::initialize()
 */
struct SimulationConfig {
  size_t particle_count = 10000;  ///< Number of particles
  InitDistribution init_distribution =
      InitDistribution::SPHERICAL;                    ///< Initial distribution pattern
  ForceMethod force_method = ForceMethod::DIRECT_N2;  ///< Force calculation algorithm
  float dt = 0.001f;                                  ///< Time step size
  float G = 1.0f;                                     ///< Gravitational constant
  float softening = 0.1f;               ///< Softening parameter (prevent singularities)
  float barnes_hut_theta = 0.5f;        ///< Barnes-Hut accuracy (0.3-0.8 typical)
  float spatial_hash_cell_size = 1.0f;  ///< Spatial hash cell size
  float spatial_hash_cutoff = 2.0f;     ///< Spatial hash force cutoff radius
  int cuda_block_size = 256;            ///< CUDA thread block size
};

/**
 * @struct RenderConfig
 * @brief Configuration for rendering settings
 *
 * Controls visual aspects of the particle rendering.
 *
 * @see Renderer
 */
struct RenderConfig {
  int window_width = 1280;                  ///< Window width in pixels
  int window_height = 720;                  ///< Window height in pixels
  float point_size = 2.0f;                  ///< Base particle point size
  ColorMode color_mode = ColorMode::DEPTH;  ///< Particle coloring mode
  bool show_stats = true;                   ///< Display statistics overlay
};

// ============================================================================
// Distribution Parameter Structures
// ============================================================================

/**
 * @struct UniformDistParams
 * @brief Parameters for uniform distribution initialization
 *
 * Creates particles uniformly distributed within a rectangular box.
 *
 * @see ParticleInitializer::initUniform()
 */
struct UniformDistParams {
  Vec3 min_bounds;        ///< Lower corner of bounding box
  Vec3 max_bounds;        ///< Upper corner of bounding box
  float min_mass = 1.0f;  ///< Minimum particle mass
  float max_mass = 1.0f;  ///< Maximum particle mass
};

/**
 * @struct SphericalDistParams
 * @brief Parameters for spherical distribution initialization
 *
 * Creates particles uniformly distributed within a sphere.
 * Good for simulating gravitational collapse.
 *
 * @see ParticleInitializer::initSpherical()
 */
struct SphericalDistParams {
  Vec3 center;            ///< Center of sphere
  float radius = 10.0f;   ///< Sphere radius
  float min_mass = 1.0f;  ///< Minimum particle mass
  float max_mass = 1.0f;  ///< Maximum particle mass
};

/**
 * @struct DiskDistParams
 * @brief Parameters for disk distribution initialization
 *
 * Creates particles in a flat disk with initial rotational velocity.
 * Suitable for galaxy formation simulations.
 *
 * @see ParticleInitializer::initDisk()
 */
struct DiskDistParams {
  Vec3 center;                  ///< Center of disk
  float radius = 10.0f;         ///< Disk radius
  float thickness = 1.0f;       ///< Disk thickness (z-extent)
  float min_mass = 1.0f;        ///< Minimum particle mass
  float max_mass = 1.0f;        ///< Maximum particle mass
  float rotation_speed = 1.0f;  ///< Initial rotation speed factor
};

}  // namespace nbody
