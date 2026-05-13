# Testing

The N-Body project uses GoogleTest for unit tests and RapidCheck for property-based testing.

## Run Tests

```bash
# All tests
./scripts/test.sh

# Or manually
cd build
ctest --output-on-failure
```

## Test Categories

### Unit Tests

Located in `tests/` directory:

| Test File | Coverage |
|-----------|----------|
| `test_particle_data.cpp` | ParticleData structure, memory layout |
| `test_particle_system.cpp` | ParticleSystem API |
| `test_force_calculator.cpp` | Force calculation correctness |
| `test_integrator.cpp` | Velocity Verlet integration |
| `test_barnes_hut.cpp` | Octree construction and traversal |
| `test_spatial_hash.cpp` | Grid construction and neighbor search |
| `test_serialization.cpp` | State save/load |

### Property-Based Tests

Use RapidCheck to generate random inputs:

```cpp
// Example: Energy should be conserved
RC_ASSERT(std::abs(energy_after - energy_before) < tolerance);
```

## Writing Tests

### Basic Test

```cpp
#include <gtest/gtest.h>
#include "nbody/particle_system.hpp"

TEST(ParticleSystemTest, InitializeSetsCorrectCount) {
    nbody::SimulationConfig config;
    config.particle_count = 1000;
    
    nbody::ParticleSystem system;
    system.initialize(config);
    
    EXPECT_EQ(system.getParticleCount(), 1000);
}
```

### CUDA Test

```cpp
#include <gtest/gtest.h>
#include "nbody/force_calculator.hpp"

TEST(ForceCalculatorTest, DirectForceMatchesReference) {
    // Create test particles
    ParticleData data;
    data.resize(100);
    // ... initialize ...
    
    // Compute on GPU
    ForceCalculator calc(ForceMethod::DIRECT);
    calc.compute(data);
    
    // Compare with CPU reference
    for (size_t i = 0; i < 100; ++i) {
        EXPECT_NEAR(data.force_x[i], ref_force_x[i], 1e-5f);
    }
}
```

## Test Configuration

### Environment Variables

| Variable | Purpose |
|----------|---------|
| `NBODY_TEST_PARTICLES` | Max particles for tests |
| `NBODY_TEST_TOLERANCE` | Floating-point tolerance |

### Headless Testing

Tests run in headless mode by default (no OpenGL required):

```bash
cmake .. -DNBODY_ENABLE_RENDERING=OFF
```

## Benchmark Tests

```bash
# Run benchmarks
./scripts/benchmark.sh

# Specific benchmark
./build/benchmark --filter=serialization.*
```

## CI Testing

Tests run automatically on GitHub Actions:

- Push to main/master
- Pull requests affecting C++/CUDA files
- Weekly scheduled run

See `.github/workflows/ci.yml` for details.