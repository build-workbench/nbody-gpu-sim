# Examples

Explore the example programs in the `examples/` directory.

## Basic Example

**File**: `examples/example_basic.cpp`

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    SimulationConfig config;
    config.particle_count = 50'000;
    config.force_method = ForceMethod::BARNES_HUT;

    ParticleSystem system;
    system.initialize(config);

    for (int i = 0; i < 1000; ++i) {
        system.update(0.001f);
    }

    return 0;
}
```

## Force Methods Comparison

**File**: `examples/example_force_methods.cpp`

Compare all three algorithms on the same initial conditions:

```cpp
#include "nbody/particle_system.hpp"
#include <iostream>
#include <chrono>

using namespace nbody;
using namespace std::chrono;

void benchmark(ForceMethod method, const std::string& name) {
    SimulationConfig config;
    config.particle_count = 100'000;
    config.force_method = method;

    ParticleSystem system;
    system.initialize(config);

    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 100; ++i) {
        system.update(0.001f);
    }
    
    auto end = high_resolution_clock::now();
    auto ms = duration_cast<milliseconds>(end - start).count();
    
    std::cout << name << ": " << ms << " ms" << std::endl;
}

int main() {
    benchmark(ForceMethod::DIRECT, "Direct N²");
    benchmark(ForceMethod::BARNES_HUT, "Barnes-Hut");
    benchmark(ForceMethod::SPATIAL_HASH, "Spatial Hash");
    
    return 0;
}
```

## Custom Distribution

**File**: `examples/example_custom_distribution.cpp`

Create particles with custom initial positions and velocities:

```cpp
#include "nbody/particle_system.hpp"
#include "nbody/particle_data.hpp"
#include <cmath>

using namespace nbody;

int main() {
    // Create particle data directly
    ParticleData data;
    data.resize(1000);
    
    // Initialize particles in a ring
    for (size_t i = 0; i < 1000; ++i) {
        float angle = 2.0f * M_PI * i / 1000.0f;
        float radius = 10.0f;
        
        data.position_x[i] = radius * std::cos(angle);
        data.position_y[i] = radius * std::sin(angle);
        data.position_z[i] = 0.0f;
        
        // Circular orbital velocity
        float v = std::sqrt(1.0f / radius);
        data.velocity_x[i] = -v * std::sin(angle);
        data.velocity_y[i] = v * std::cos(angle);
        data.velocity_z[i] = 0.0f;
        
        data.mass[i] = 1.0f;
    }
    
    // Create system from data
    ParticleSystem system;
    system.initialize(data, ForceMethod::BARNES_HUT);
    
    // Run simulation
    for (int i = 0; i < 10'000; ++i) {
        system.update(0.001f);
    }
    
    return 0;
}
```

## Energy Conservation

**File**: `examples/example_energy_conservation.cpp`

Monitor energy conservation over time:

```cpp
#include "nbody/particle_system.hpp"
#include <iostream>
#include <fstream>

using namespace nbody;

int main() {
    SimulationConfig config;
    config.particle_count = 10'000;
    config.force_method = ForceMethod::BARNES_HUT;

    ParticleSystem system;
    system.initialize(config);

    double initial_energy = system.getTotalEnergy();
    std::ofstream out("energy.csv");
    out << "step,time,kinetic,potential,total,error\n";

    for (int step = 0; step < 10'000; ++step) {
        system.update(0.001f);
        
        if (step % 100 == 0) {
            double ke = system.getKineticEnergy();
            double pe = system.getPotentialEnergy();
            double total = ke + pe;
            double error = std::abs((total - initial_energy) / initial_energy);
            
            out << step << ","
                << system.getTime() << ","
                << ke << ","
                << pe << ","
                << total << ","
                << error << "\n";
        }
    }

    std::cout << "Energy data written to energy.csv" << std::endl;
    return 0;
}
```

## Running Examples

```bash
# Build examples
cmake --build build --target examples

# Run specific example
./build/example_basic
./build/example_force_methods
./build/example_energy_conservation
```

## Next Steps

- [API Reference](/en/api-reference/particle-system) - Full API documentation
- [Algorithms](/en/user-guide/algorithms) - Understanding the algorithms