---
layout: default
title: API Reference
parent: Documentation
nav_order: 3
---

# API Reference

Complete API reference for the N-Body Particle Simulation library.

---

## 📑 Table of Contents

- [Core Classes](#core-classes)
  - [ParticleSystem](#particlesystem)
  - [ForceCalculator](#forcecalculator)
  - [Integrator](#integrator)
  - [Camera](#camera)
  - [Renderer](#renderer)
  - [CudaGLInterop](#cudaglintinterop)
- [Data Structures](#data-structures)
  - [ParticleData](#particledata)
  - [SimulationConfig](#simulationconfig)
  - [SimulationState](#simulationstate)
- [Enumerations](#enumerations)
- [Utility Functions](#utility-functions)

---

## Core Classes

### ParticleSystem

Main orchestrator class for managing the entire simulation.

```cpp
namespace nbody {

class ParticleSystem {
public:
    // Lifecycle
    ParticleSystem();
    ~ParticleSystem();
    
    // Initialization
    void initialize(const SimulationConfig& config);
    void initializeWithDistribution(size_t particle_count, InitDistribution dist);
    
    // Simulation Control
    void update(float dt);
    void pause();
    void resume();
    void reset();
    bool isPaused() const;
    
    // Parameter Configuration
    void setForceMethod(ForceMethod method);
    void setGravitationalConstant(float G);
    void setSofteningParameter(float eps);
    void setTimeStep(float dt);
    void setBarnesHutTheta(float theta);
    void setSpatialHashCellSize(float size);
    void setSpatialHashCutoff(float cutoff);
    
    // Parameter Accessors
    ForceMethod getForceMethod() const;
    float getGravitationalConstant() const;
    float getSofteningParameter() const;
    float getTimeStep() const;
    float getSimulationTime() const;
    size_t getParticleCount() const;
    
    // Data Access
    ParticleData* getDeviceData();
    void copyToHost(ParticleData& h_particles) const;
    
    // State Management
    void saveState(const std::string& filename) const;
    void loadState(const std::string& filename);
    SimulationState getState() const;
    void setState(const SimulationState& state);
    
    // Energy Computation
    float computeKineticEnergy() const;
    float computePotentialEnergy() const;
    float computeTotalEnergy() const;
    
    // CUDA-GL Interop
    CudaGLInterop* getInterop();
    void initializeInterop();
    void updateInteropBuffer();
};

} // namespace nbody
```

#### Usage Example

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    // Configure simulation
    SimulationConfig config;
    config.particle_count = 100000;
    config.force_method = ForceMethod::BARNES_HUT;
    config.dt = 0.001f;
    config.G = 1.0f;
    
    // Initialize system
    ParticleSystem system;
    system.initialize(config);
    
    // Run simulation
    for (int i = 0; i < 1000; ++i) {
        system.update(system.getTimeStep());
        
        // Monitor energy every 100 steps
        if (i % 100 == 0) {
            float E = system.computeTotalEnergy();
            printf("Step %d: Total Energy = %.6f\n", i, E);
        }
    }
    
    // Save checkpoint
    system.saveState("checkpoint.nbody");
    
    return 0;
}
```

---

### ForceCalculator

Abstract base class for force calculation algorithms.

```cpp
namespace nbody {

class ForceCalculator {
public:
    virtual ~ForceCalculator() = default;
    
    // Compute forces for all particles
    virtual void computeForces(ParticleData* d_particles) = 0;
    
    // Get algorithm type
    virtual ForceMethod getMethod() const = 0;
    
    // Parameters
    void setGravitationalConstant(float G);
    void setSofteningParameter(float eps);
    
    float getGravitationalConstant() const;
    float getSofteningParameter() const;
    
protected:
    float G_ = 1.0f;           // Gravitational constant
    float softening_eps_ = 0.1f;  // Softening length
    float softening_eps2_ = 0.01f; // Squared softening
};

} // namespace nbody
```

#### DirectForceCalculator

```cpp
class DirectForceCalculator : public ForceCalculator {
public:
    explicit DirectForceCalculator(int block_size = 256);
    
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override;
    
    void setBlockSize(int size);
    int getBlockSize() const;
};
```

#### BarnesHutCalculator

```cpp
class BarnesHutCalculator : public ForceCalculator {
public:
    explicit BarnesHutCalculator(float theta = 0.5f);
    
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override;
    
    // Opening angle parameter (0 = exact, 1 = fastest)
    void setTheta(float theta);
    float getTheta() const;
    
    BarnesHutTree* getTree() const;
};
```

#### SpatialHashCalculator

```cpp
class SpatialHashCalculator : public ForceCalculator {
public:
    SpatialHashCalculator(float cell_size = 1.0f, float cutoff_radius = 2.0f);
    
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override;
    
    void setCellSize(float size);
    void setCutoffRadius(float radius);
    float getCellSize() const;
    float getCutoffRadius() const;
    
    SpatialHashGrid* getGrid() const;
};
```

---

### Integrator

Velocity Verlet symplectic integrator.

```cpp
namespace nbody {

class Integrator {
public:
    explicit Integrator(int block_size = 256);
    
    // Full integration step
    void integrate(ParticleData* d_particles, 
                   ForceCalculator* force_calc, 
                   float dt);
    
    // Step components (for custom schemes)
    void updatePositions(ParticleData* d_particles, float dt);
    void updateVelocities(ParticleData* d_particles, float dt);
    void storeOldAccelerations(ParticleData* d_particles);
    
    // Energy computation
    float computeKineticEnergy(const ParticleData* d_particles);
    float computePotentialEnergy(const ParticleData* d_particles, float G, float eps);
    float computeTotalEnergy(const ParticleData* d_particles, float G, float eps);
    
    // Configuration
    void setBlockSize(int size);
    int getBlockSize() const;
};

} // namespace nbody
```

---

### Camera

3D orbit camera controller.

```cpp
namespace nbody {

class Camera {
public:
    Camera(float fov = 45.0f, float aspect = 16.0f/9.0f, 
           float near_plane = 0.1f, float far_plane = 1000.0f);
    
    // Transformation matrices
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewProjectionMatrix() const;
    
    // Positioning
    void setPosition(const glm::vec3& pos);
    void setTarget(const glm::vec3& target);
    void setUp(const glm::vec3& up);
    
    glm::vec3 getPosition() const;
    glm::vec3 getTarget() const;
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    
    // Projection
    void setFOV(float fov);
    void setAspectRatio(float aspect);
    void setNearFar(float near_plane, float far_plane);
    
    // Controls
    void rotate(float yaw, float pitch);
    void pan(float dx, float dy);
    void zoom(float delta);
    void orbit(float yaw, float pitch);
    void reset();
    
    void setOrbitDistance(float distance);
    float getOrbitDistance() const;
};

} // namespace nbody
```

---

### Renderer

OpenGL-based particle renderer.

```cpp
namespace nbody {

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    void initialize(int width, int height);
    void cleanup();
    
    // Rendering
    void render(GLuint position_vbo, size_t particle_count);
    void render(const ParticleData* d_particles);
    
    // Camera
    void setCamera(const Camera& camera);
    Camera& getCamera();
    
    // Settings
    void setColorMode(ColorMode mode);
    void setPointSize(float size);
    void setMaxDepth(float depth);
    void setMaxVelocity(float velocity);
    
    // Window events
    void onResize(int width, int height);
    
    // Info
    int getWidth() const;
    int getHeight() const;
};

} // namespace nbody
```

---

### CudaGLInterop

CUDA-OpenGL zero-copy interoperation.

```cpp
namespace nbody {

class CudaGLInterop {
public:
    CudaGLInterop();
    ~CudaGLInterop();
    
    void initialize(size_t particle_count);
    void cleanup();
    
    // Map/unmap
    float* mapPositionBuffer();     // Returns CUDA device pointer
    void unmapPositionBuffer();     // Release for OpenGL
    
    bool isMapped() const;
    GLuint getPositionVBO() const;
    size_t getParticleCount() const;
    
    // Update VBO from ParticleData
    void updatePositions(const ParticleData* d_particles);
};

} // namespace nbody
```

---

## Data Structures

### ParticleData

Structure of Arrays (SoA) layout for GPU efficiency.

```cpp
struct ParticleData {
    // Positions
    float* pos_x;
    float* pos_y;
    float* pos_z;
    
    // Velocities
    float* vel_x;
    float* vel_y;
    float* vel_z;
    
    // Current accelerations
    float* acc_x;
    float* acc_y;
    float* acc_z;
    
    // Old accelerations (for Verlet)
    float* acc_old_x;
    float* acc_old_y;
    float* acc_old_z;
    
    // Masses
    float* mass;
    
    // Count
    size_t count;
};
```

**Memory usage:** 52 bytes per particle (13 floats × 4 bytes)

---

### SimulationConfig

Configuration for simulation setup.

```cpp
struct SimulationConfig {
    // Particle settings
    size_t particle_count = 10000;
    InitDistribution init_distribution = InitDistribution::SPHERICAL;
    
    // Physics settings
    ForceMethod force_method = ForceMethod::DIRECT_N2;
    float dt = 0.001f;
    float G = 1.0f;
    float softening = 0.01f;
    
    // Algorithm-specific settings
    float barnes_hut_theta = 0.5f;
    float spatial_hash_cell_size = 1.0f;
    float spatial_hash_cutoff = 2.0f;
    
    // Performance settings
    int cuda_block_size = 256;
};
```

---

### SimulationState

Serializable simulation state.

```cpp
struct SimulationState {
    std::vector<float> pos_x, pos_y, pos_z;
    std::vector<float> vel_x, vel_y, vel_z;
    std::vector<float> mass;
    
    size_t particle_count;
    float simulation_time;
    float dt;
    float G;
    float softening;
    ForceMethod force_method;
    
    // Serialization
    void serialize(std::ostream& out) const;
    static SimulationState deserialize(std::istream& in);
    
    bool operator==(const SimulationState& other) const;
};
```

---

## Enumerations

### ForceMethod

Available force calculation algorithms.

```cpp
enum class ForceMethod {
    DIRECT_N2,      // O(N²) exact calculation
    BARNES_HUT,     // O(N log N) tree approximation
    SPATIAL_HASH    // O(N) grid-based (short-range)
};
```

### InitDistribution

Particle initial distribution types.

```cpp
enum class InitDistribution {
    UNIFORM,        // Uniform box
    SPHERICAL,      // Uniform sphere
    DISK            // Flat disk with rotation
};
```

### ColorMode

Particle coloring modes.

```cpp
enum class ColorMode {
    DEPTH,          // Color by depth (z-coordinate)
    VELOCITY,       // Color by velocity magnitude
    DENSITY         // Color by local density
};
```

---

## Utility Functions

### ParticleDataManager

Memory management utilities.

```cpp
namespace nbody {

class ParticleDataManager {
public:
    // Device memory
    static void allocateDevice(ParticleData& data, size_t count);
    static void freeDevice(ParticleData& data);
    
    // Host memory
    static void allocateHost(ParticleData& data, size_t count);
    static void freeHost(ParticleData& data);
    
    // Data transfer
    static void copyToDevice(ParticleData& d_data, const ParticleData& h_data);
    static void copyToHost(ParticleData& h_data, const ParticleData& d_data);
};

} // namespace nbody
```

### ParticleInitializer

Particle distribution initialization.

```cpp
namespace nbody {

struct UniformDistParams {
    glm::vec3 min_bounds;
    glm::vec3 max_bounds;
    float min_mass = 1.0f;
    float max_mass = 1.0f;
};

struct SphericalDistParams {
    glm::vec3 center;
    float radius;
    float mass = 1.0f;
};

struct DiskDistParams {
    glm::vec3 center;
    float radius;
    float thickness;
    float rotation_speed;
    float mass = 1.0f;
};

class ParticleInitializer {
public:
    static void initUniform(ParticleData& h_data, const UniformDistParams& params,
                           unsigned int seed = 42);
    static void initSpherical(ParticleData& h_data, const SphericalDistParams& params,
                             unsigned int seed = 42);
    static void initDisk(ParticleData& h_data, const DiskDistParams& params,
                        unsigned int seed = 42);
    
    static void zeroVelocities(ParticleData& h_data);
    static void zeroAccelerations(ParticleData& h_data);
};

} // namespace nbody
```

### Serializer

State persistence utilities.

```cpp
namespace nbody {

class Serializer {
public:
    static void save(const std::string& filename, const SimulationState& state);
    static SimulationState load(const std::string& filename);
    
    static void save(std::ostream& out, const SimulationState& state);
    static SimulationState load(std::istream& in);
    
    static bool validateFile(const std::string& filename);
};

} // namespace nbody
```

### Error Handling

```cpp
namespace nbody {

// Exception types
class CudaException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class OpenGLException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class ValidationException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

// CUDA error checking
#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            throw CudaException(cudaGetErrorString(err)); \
        } \
    } while(0)

#define CUDA_CHECK_KERNEL() \
    do { \
        cudaError_t err = cudaGetLastError(); \
        if (err != cudaSuccess) { \
            throw CudaException(cudaGetErrorString(err)); \
        } \
    } while(0)

// Validation functions
void validateSimulationConfig(const SimulationConfig& config);
void validateParticleCount(size_t count);
void validateTimeStep(float dt);
void validateSoftening(float eps);

} // namespace nbody
```

---

## 📚 Related Documentation

- [Getting Started](../setup/getting-started.md) - Setup and usage guide
- [Architecture](./architecture.md) - System design overview
- [Algorithms](./algorithms.md) - Algorithm explanations
- [Performance Guide](./performance.md) - Optimization strategies
