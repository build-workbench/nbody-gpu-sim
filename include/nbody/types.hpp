#pragma once

#include <cuda_runtime.h>
#include <cstddef>

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

// Enumerations
enum class ForceMethod {
    DIRECT_N2,      // O(N²) direct calculation
    BARNES_HUT,     // O(N log N) tree algorithm
    SPATIAL_HASH    // O(N) spatial hashing
};

enum class InitDistribution {
    UNIFORM,        // Uniform distribution in bounding box
    SPHERICAL,      // Spherical distribution
    DISK            // Disk/galaxy distribution
};

enum class ColorMode {
    DEPTH,          // Color by depth
    VELOCITY,       // Color by velocity magnitude
    DENSITY         // Color by local density
};

// Vector types (using CUDA float3/float4 when available)
struct Vec3 {
    float x, y, z;
    
    __host__ __device__ Vec3() : x(0), y(0), z(0) {}
    __host__ __device__ Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    
    __host__ __device__ Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    __host__ __device__ Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    __host__ __device__ Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    __host__ __device__ Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
    __host__ __device__ Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    __host__ __device__ Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    
    __host__ __device__ float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    __host__ __device__ float length2() const { return dot(*this); }
    __host__ __device__ float length() const { return sqrtf(length2()); }
    __host__ __device__ Vec3 normalized() const { float l = length(); return l > 0 ? *this / l : Vec3(); }
};

__host__ __device__ inline Vec3 operator*(float s, const Vec3& v) { return v * s; }

// Particle data in Structure of Arrays (SoA) layout
struct ParticleData {
    // Position arrays
    float* pos_x;
    float* pos_y;
    float* pos_z;
    
    // Velocity arrays
    float* vel_x;
    float* vel_y;
    float* vel_z;
    
    // Acceleration arrays (for Verlet integration)
    float* acc_x;
    float* acc_y;
    float* acc_z;
    
    // Previous acceleration (for Velocity Verlet)
    float* acc_old_x;
    float* acc_old_y;
    float* acc_old_z;
    
    // Mass array
    float* mass;
    
    // Particle count
    size_t count;
    
    ParticleData() : pos_x(nullptr), pos_y(nullptr), pos_z(nullptr),
                     vel_x(nullptr), vel_y(nullptr), vel_z(nullptr),
                     acc_x(nullptr), acc_y(nullptr), acc_z(nullptr),
                     acc_old_x(nullptr), acc_old_y(nullptr), acc_old_z(nullptr),
                     mass(nullptr), count(0) {}
};

// Simulation configuration
struct SimulationConfig {
    size_t particle_count = 10000;
    InitDistribution init_distribution = InitDistribution::SPHERICAL;
    ForceMethod force_method = ForceMethod::DIRECT_N2;
    float dt = 0.001f;
    float G = 1.0f;
    float softening = 0.01f;
    float barnes_hut_theta = 0.5f;
    float spatial_hash_cell_size = 1.0f;
    float spatial_hash_cutoff = 2.0f;
    int cuda_block_size = 256;
};

// Render configuration
struct RenderConfig {
    int window_width = 1280;
    int window_height = 720;
    float point_size = 2.0f;
    ColorMode color_mode = ColorMode::DEPTH;
    bool show_stats = true;
};

// Distribution parameters
struct UniformDistParams {
    Vec3 min_bounds;
    Vec3 max_bounds;
    float min_mass = 1.0f;
    float max_mass = 1.0f;
};

struct SphericalDistParams {
    Vec3 center;
    float radius = 10.0f;
    float min_mass = 1.0f;
    float max_mass = 1.0f;
};

struct DiskDistParams {
    Vec3 center;
    float radius = 10.0f;
    float thickness = 1.0f;
    float min_mass = 1.0f;
    float max_mass = 1.0f;
    float rotation_speed = 1.0f;  // For initial orbital velocity
};

} // namespace nbody
