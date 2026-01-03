# Requirements Document

## Introduction

超大规模 N-Body 粒子仿真系统，用于模拟数百万个粒子在引力作用下的运动。系统采用 GPU 并行计算实现高性能仿真，并通过 CUDA-OpenGL 互操作实现零拷贝实时可视化。

## Glossary

- **Particle_System**: 管理所有粒子数据和状态的核心系统
- **Force_Calculator**: 计算粒子间相互作用力的模块
- **Barnes_Hut_Tree**: 用于加速力计算的八叉树数据结构
- **Spatial_Hash_Grid**: 用于空间划分的网格数据结构
- **Renderer**: 负责粒子可视化渲染的模块
- **CUDA_GL_Interop**: CUDA 与 OpenGL 之间的数据互操作层
- **Integrator**: 根据力更新粒子位置和速度的数值积分器

## Requirements

### Requirement 1: Particle Data Management

**User Story:** As a simulation developer, I want to efficiently manage millions of particles' data, so that the system can handle large-scale simulations.

#### Acceptance Criteria

1. THE Particle_System SHALL store particle data using Structure of Arrays (SoA) layout for optimal memory coalescing
2. THE Particle_System SHALL support at least 1 million particles
3. WHEN initializing particles, THE Particle_System SHALL allow configurable initial distributions (uniform, spherical, disk)
4. THE Particle_System SHALL maintain position (x, y, z), velocity (vx, vy, vz), and mass for each particle

### Requirement 2: Direct N-Body Force Calculation

**User Story:** As a physics simulation user, I want accurate gravitational force calculations between all particle pairs, so that the simulation produces physically correct results.

#### Acceptance Criteria

1. THE Force_Calculator SHALL compute gravitational forces using Newton's law of universal gravitation
2. THE Force_Calculator SHALL use GPU parallel computation with one thread per particle
3. THE Force_Calculator SHALL utilize CUDA Shared Memory to cache particle data for reduced global memory access
4. THE Force_Calculator SHALL use hardware rsqrt instruction for inverse square root calculations
5. WHEN computing forces, THE Force_Calculator SHALL apply softening parameter to prevent numerical singularities

### Requirement 3: Barnes-Hut Tree Acceleration

**User Story:** As a performance-conscious user, I want an O(N log N) algorithm option, so that I can simulate larger particle counts efficiently.

#### Acceptance Criteria

1. THE Barnes_Hut_Tree SHALL construct an octree structure from particle positions
2. THE Barnes_Hut_Tree SHALL compute center of mass and total mass for each tree node
3. WHEN calculating forces, THE Barnes_Hut_Tree SHALL use theta parameter to determine when to approximate distant particle groups
4. THE Barnes_Hut_Tree SHALL be rebuilt each simulation step to handle particle movement
5. THE Force_Calculator SHALL support switching between direct O(N²) and Barnes-Hut O(N log N) methods

### Requirement 4: Spatial Hash Grid Acceleration

**User Story:** As a user simulating local interactions, I want an O(N) spatial hashing option, so that I can efficiently compute short-range forces.

#### Acceptance Criteria

1. THE Spatial_Hash_Grid SHALL partition 3D space into uniform grid cells
2. THE Spatial_Hash_Grid SHALL assign each particle to its corresponding cell
3. WHEN calculating forces, THE Spatial_Hash_Grid SHALL only consider particles in neighboring cells
4. THE Spatial_Hash_Grid SHALL support configurable cell size based on interaction cutoff radius

### Requirement 5: Numerical Integration

**User Story:** As a simulation user, I want stable and accurate time integration, so that particle trajectories are physically meaningful.

#### Acceptance Criteria

1. THE Integrator SHALL implement Velocity Verlet integration scheme
2. THE Integrator SHALL support configurable time step size
3. THE Integrator SHALL update all particle positions and velocities in parallel on GPU
4. WHEN time step is too large, THE Integrator SHALL provide energy conservation metrics for stability monitoring

### Requirement 6: CUDA-OpenGL Interoperability

**User Story:** As a visualization user, I want zero-copy data transfer between compute and render, so that real-time visualization doesn't bottleneck performance.

#### Acceptance Criteria

1. THE CUDA_GL_Interop SHALL register OpenGL buffer objects with CUDA
2. THE CUDA_GL_Interop SHALL map particle position data directly to OpenGL vertex buffers
3. THE CUDA_GL_Interop SHALL avoid CPU-GPU data transfers during normal operation
4. WHEN rendering, THE Renderer SHALL use the shared buffer without additional copies

### Requirement 7: Real-time Visualization

**User Story:** As a user, I want to see the particle simulation in real-time, so that I can observe and understand the system's behavior.

#### Acceptance Criteria

1. THE Renderer SHALL display particles as points or sprites with configurable size
2. THE Renderer SHALL support camera controls (rotation, zoom, pan)
3. THE Renderer SHALL display simulation statistics (particle count, FPS, simulation time)
4. WHEN particle density varies, THE Renderer SHALL use color mapping to indicate velocity or density
5. THE Renderer SHALL maintain at least 30 FPS for 1 million particles on supported hardware

### Requirement 8: Simulation Control

**User Story:** As a user, I want to control simulation parameters at runtime, so that I can experiment with different configurations.

#### Acceptance Criteria

1. THE Particle_System SHALL support pause, resume, and reset operations
2. THE Particle_System SHALL allow runtime adjustment of gravitational constant
3. THE Particle_System SHALL allow runtime switching between force calculation methods
4. THE Particle_System SHALL support saving and loading simulation states

### Requirement 9: Performance Optimization

**User Story:** As a developer, I want optimized GPU kernels, so that the simulation achieves maximum throughput.

#### Acceptance Criteria

1. THE Force_Calculator SHALL achieve memory coalescing through SoA data layout
2. THE Force_Calculator SHALL use CUDA Shared Memory to reduce global memory bandwidth
3. THE Force_Calculator SHALL configure optimal thread block sizes for target GPU architecture
4. WHEN profiling, THE system SHALL report kernel execution times and memory bandwidth utilization

### Requirement 10: Error Handling and Robustness

**User Story:** As a user, I want the system to handle errors gracefully, so that I can diagnose and recover from problems.

#### Acceptance Criteria

1. IF CUDA initialization fails, THEN THE system SHALL report detailed error information and exit gracefully
2. IF OpenGL context creation fails, THEN THE system SHALL report the error and suggest solutions
3. IF particle count exceeds GPU memory, THEN THE system SHALL report memory requirements and reduce count
4. THE system SHALL validate all user inputs before applying them
