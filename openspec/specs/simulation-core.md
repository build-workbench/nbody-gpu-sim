# Capability: Simulation Core

## Overview

Manages N-body system state, particle data, and force calculation algorithms including Barnes-Hut for O(N log N) performance.

---

## ADDED Requirements

### Requirement: Particle Data Layout

The system SHALL use Structure of Arrays (SoA) layout for GPU memory coalescing.

#### Scenario: Memory Coalescing
- **WHEN** particle data is accessed on GPU
- **THEN** the system SHALL arrange data in contiguous arrays per attribute (pos_x, pos_y, pos_z, vel_x, vel_y, vel_z, etc.)

#### Scenario: Large Scale Support
- **WHEN** a simulation is initialized
- **THEN** the system SHALL support at least 10 million particles

---

### Requirement: Particle Attributes

The system SHALL store position, velocity, acceleration, and mass for each particle.

#### Scenario: Complete State Storage
- **WHEN** particles are created
- **THEN** the system SHALL allocate memory for position (3N), velocity (3N), acceleration (6N - current + old), and mass (N)

#### Scenario: Memory Efficiency
- **WHEN** 1 million particles are simulated
- **THEN** total memory usage SHALL be less than 200 MB

---

### Requirement: Initial Distributions

The system SHALL provide configurable initial distributions (uniform, spherical, disk).

#### Scenario: Uniform Distribution
- **WHEN** user specifies uniform distribution
- **THEN** particles SHALL be evenly distributed within specified bounds

#### Scenario: Spherical Distribution
- **WHEN** user specifies spherical distribution
- **THEN** particles SHALL be distributed within a sphere with configurable radius

#### Scenario: Disk Distribution
- **WHEN** user specifies disk distribution
- **THEN** particles SHALL be distributed in a rotating disk pattern

---

### Requirement: Gravitational Force Calculation

The system SHALL compute gravitational forces using Newton's law.

#### Scenario: Pairwise Force
- **WHEN** computing forces between two particles
- **THEN** the force magnitude SHALL approximate G·m₁·m₂/(r² + ε²) where ε is the softening parameter

#### Scenario: Softening Parameter
- **WHEN** particles are very close together
- **THEN** the system SHALL apply softening to prevent numerical singularities

---

### Requirement: GPU Parallel Computation

The system SHALL use GPU parallel computation with one thread per particle.

#### Scenario: Thread Assignment
- **WHEN** force calculation is performed
- **THEN** each particle SHALL be processed by a dedicated GPU thread

#### Scenario: Shared Memory Caching
- **WHEN** computing forces
- **THEN** the system SHALL use CUDA shared memory for data caching

#### Scenario: Hardware Optimization
- **WHEN** computing inverse square root
- **THEN** the system SHALL use hardware rsqrt for performance

---

### Requirement: Barnes-Hut Algorithm

The system SHALL implement Barnes-Hut algorithm for O(N log N) force calculation.

#### Scenario: Octree Construction
- **WHEN** Barnes-Hut method is selected
- **THEN** the system SHALL construct an octree from particle positions

#### Scenario: Center of Mass
- **WHEN** tree nodes are built
- **THEN** each node SHALL store center of mass and total mass of contained particles

#### Scenario: Theta Parameter
- **WHEN** traversing the tree for force calculation
- **THEN** the system SHALL use configurable θ parameter for approximation accuracy

#### Scenario: Tree Rebuild
- **WHEN** simulation advances to next step
- **THEN** the system SHALL rebuild the octree

#### Scenario: Algorithm Switching
- **WHEN** user requests different force method
- **THEN** the system SHALL support runtime switching between Direct N², Barnes-Hut, and Spatial Hash

---

## Traceability

| Requirement | Original ID | Test File |
|-------------|-------------|-----------|
| Particle Data Layout | FR-1.1 | test_particle_data.cpp |
| Particle Attributes | FR-1.4 | test_particle_data.cpp |
| Initial Distributions | FR-1.3 | test_particle_data.cpp |
| Large Scale Support | FR-1.2 | test_particle_data.cpp |
| Gravitational Force Calculation | FR-2.1, FR-2.5 | test_force_calculation.cpp |
| GPU Parallel Computation | FR-2.2, FR-2.3, FR-2.4 | test_force_calculation.cpp |
| Barnes-Hut Algorithm | FR-3.1, FR-3.2, FR-3.3, FR-3.4, FR-3.5 | test_barnes_hut.cpp |
