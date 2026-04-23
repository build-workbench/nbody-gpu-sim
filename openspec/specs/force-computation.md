# Capability: Force Computation

## Overview

Provides efficient force computation algorithms including Spatial Hash for O(N) short-range forces and Velocity Verlet numerical integration.

---

## ADDED Requirements

### Requirement: Spatial Hash Grid

The system SHALL partition 3D space into uniform grid cells for efficient neighbor search.

#### Scenario: Cell Assignment
- **WHEN** particles are distributed in space
- **THEN** each particle SHALL be assigned to exactly one grid cell

#### Scenario: Cell Bounds
- **WHEN** a particle is assigned to a cell
- **THEN** the cell bounds SHALL contain the particle position

---

### Requirement: Neighbor Search

The system SHALL only process particles in neighboring cells for short-range forces.

#### Scenario: Cutoff Radius
- **WHEN** computing short-range forces
- **THEN** only particles within cutoff radius SHALL be considered

#### Scenario: Neighbor Cells
- **WHEN** searching for neighbors
- **THEN** only the 3×3×3 cell neighborhood SHALL be examined

#### Scenario: Configurable Parameters
- **WHEN** using spatial hash method
- **THEN** the system SHALL support configurable cell size and cutoff radius

---

### Requirement: Velocity Verlet Integration

The system SHALL implement Velocity Verlet symplectic integration.

#### Scenario: Position Update
- **WHEN** a time step is computed
- **THEN** positions SHALL be updated using x(t+dt) = x(t) + v(t)·dt + ½·a(t)·dt²

#### Scenario: Velocity Update
- **WHEN** new accelerations are computed
- **THEN** velocities SHALL be updated using v(t+dt) = v(t) + ½·(a_old + a_new)·dt

#### Scenario: Single Force Evaluation
- **WHEN** integration step is performed
- **THEN** only one force evaluation per step SHALL be required

---

### Requirement: Time Step Configuration

The system SHALL support configurable time step.

#### Scenario: Custom Time Step
- **WHEN** user specifies a time step value
- **THEN** the integrator SHALL use that value for simulation

#### Scenario: Parallel Update
- **WHEN** integration is performed
- **THEN** all particles SHALL be updated in parallel on GPU

---

### Requirement: Energy Conservation

The system SHALL provide energy conservation metrics.

#### Scenario: Energy Drift Measurement
- **WHEN** simulation runs for 1000 steps
- **THEN** energy drift SHALL be less than 1%

#### Scenario: Phase Space Preservation
- **WHEN** symplectic integrator is used
- **THEN** phase space volume SHALL be preserved

---

## Traceability

| Requirement | Original ID | Test File |
|-------------|-------------|-----------|
| Spatial Hash Grid | FR-4.1 | test_spatial_hash.cpp |
| Neighbor Search | FR-4.2, FR-4.3, FR-4.4 | test_spatial_hash.cpp |
| Velocity Verlet Integration | FR-5.1 | test_integrator.cpp |
| Time Step Configuration | FR-5.2, FR-5.3 | test_integrator.cpp |
| Energy Conservation | FR-5.4 | test_integrator.cpp |
