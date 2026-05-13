# Testing Guide

How to write and run tests.

## Test Structure

```
tests/
├── test_particle_data.cpp
├── test_particle_system.cpp
├── test_force_calculator.cpp
├── test_integrator.cpp
├── test_barnes_hut.cpp
├── test_spatial_hash.cpp
└── test_serialization.cpp
```

## Writing Unit Tests

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

## Property-Based Tests

```cpp
#include <rapidcheck.h>
#include <gtest/gtest.h>

RC_GTEST_PROP(EnergyConservation, TotalEnergyIsConserved, ()) {
    auto config = *rc::gen::arbitrary<SimulationConfig>();
    // ... test that energy is conserved
}
```

## Running Tests

```bash
# All tests
./scripts/test.sh

# Specific test
./build/nbody_tests --gtest_filter=ParticleSystemTest.*
```
