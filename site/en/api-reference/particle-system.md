# ParticleSystem API

The main API for controlling the simulation.

## Class Overview

```cpp
namespace nbody {

class ParticleSystem {
public:
    // Initialization
    void initialize(const SimulationConfig& config);
    void initialize(const ParticleData& data, ForceMethod method);
    
    // Simulation control
    void update(float dt);
    void pause();
    void resume();
    void reset();
    
    // Algorithm switching
    void setForceMethod(ForceMethod method);
    ForceMethod getForceMethod() const;
    
    // State access
    size_t getParticleCount() const;
    float getTime() const;
    double getTotalEnergy() const;
    double getKineticEnergy() const;
    double getPotentialEnergy() const;
    
    // Persistence
    void saveState(const std::string& path);
    void loadState(const std::string& path);
    void exportHDF5(const std::string& path);
    void importHDF5(const std::string& path);
    
    // Data access
    const ParticleData& getParticleData() const;
};

} // namespace nbody
```

## Initialization

### From Config

```cpp
SimulationConfig config;
config.particle_count = 100'000;
config.force_method = ForceMethod::BARNES_HUT;
config.dt = 0.001f;
config.init_distribution = InitDistribution::SPHERICAL;

ParticleSystem system;
system.initialize(config);
```

### From Custom Data

```cpp
ParticleData data;
data.resize(1000);

// Set custom positions, velocities, masses
for (size_t i = 0; i < 1000; ++i) {
    data.position_x[i] = /* ... */;
    data.velocity_x[i] = /* ... */;
    data.mass[i] = 1.0f;
}

ParticleSystem system;
system.initialize(data, ForceMethod::DIRECT);
```

## Simulation Control

### Update

```cpp
// Single step
system.update(0.001f);

// Multiple steps
for (int i = 0; i < 1000; ++i) {
    system.update(0.001f);
}
```

### Pause/Resume

```cpp
system.pause();  // Stop simulation
// ... inspect or modify state ...
system.resume(); // Continue simulation
```

### Reset

```cpp
system.reset();  // Return to initial state
```

## Algorithm Switching

```cpp
// Switch at runtime
system.setForceMethod(ForceMethod::SPATIAL_HASH);

// Check current method
if (system.getForceMethod() == ForceMethod::DIRECT) {
    std::cout << "Using Direct N²" << std::endl;
}
```

## Energy Monitoring

```cpp
double ke = system.getKineticEnergy();
double pe = system.getPotentialEnergy();
double total = system.getTotalEnergy();

std::cout << "Kinetic: " << ke << std::endl;
std::cout << "Potential: " << pe << std::endl;
std::cout << "Total: " << total << std::endl;
```

## Serialization

### Binary Format

```cpp
// Save checkpoint
system.saveState("checkpoint_1000.nbody");

// Load checkpoint
system.loadState("checkpoint_1000.nbody");
```

### HDF5 Format

```cpp
// Export
system.exportHDF5("simulation_output.h5");

// Import
system.importHDF5("simulation_input.h5");
```

## Error Handling

```cpp
try {
    system.initialize(config);
} catch (const nbody::CUDAError& e) {
    std::cerr << "CUDA error: " << e.what() << std::endl;
} catch (const nbody::MemoryError& e) {
    std::cerr << "Memory error: " << e.what() << std::endl;
}
```

## Next Steps

- [ForceCalculator API](/en/api-reference/force-calculator)
- [Integrator API](/en/api-reference/integrator)