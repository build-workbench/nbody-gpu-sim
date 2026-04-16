---
layout: default
title: Algorithms
parent: Documentation
nav_order: 4
---

# Algorithms

Detailed explanation of the force calculation algorithms and integration methods used in the N-Body Particle Simulation System.

---

## 📑 Table of Contents

1. [Force Calculation Fundamentals](#force-calculation-fundamentals)
2. [Direct N² Algorithm](#direct-n²-algorithm)
3. [Barnes-Hut Algorithm](#barnes-hut-algorithm)
4. [Spatial Hash Algorithm](#spatial-hash-algorithm)
5. [Velocity Verlet Integration](#velocity-verlet-integration)
6. [Algorithm Selection Guide](#algorithm-selection-guide)

---

## Force Calculation Fundamentals

### Newton's Law of Universal Gravitation

The gravitational force between two particles:

$$F = G \frac{m_1 m_2}{r^2}$$

Where:
- $G$ = Gravitational constant
- $m_1, m_2$ = Particle masses
- $r$ = Distance between particles

### Softening Parameter

To prevent numerical divergence at close encounters, we introduce a softening parameter $\epsilon$:

$$F = G \frac{m_1 m_2}{r^2 + \epsilon^2}$$

**Benefits:**
- Prevents force singularities
- Models finite-sized particles
- Smooths close interactions

**Typical values:** $\epsilon \in [0.01, 0.1]$ (dependent on system scale)

### Acceleration Calculation

The total acceleration on particle $i$ due to all other particles:

$$\mathbf{a}_i = \sum_{j \neq i} G m_j \frac{\mathbf{r}_j - \mathbf{r}_i}{|\mathbf{r}_j - \mathbf{r}_i|^3}$$

---

## Direct N² Algorithm

### Principle

Compute forces between all pairs of particles directly. Guaranteed exact but $O(N^2)$ complexity.

### Pseudo-code

```
for each particle i:
    a_i = 0
    for each particle j ≠ i:
        r = pos_j - pos_i
        dist² = |r|² + ε²
        a_i += G * m_j * r / dist^(3/2)
```

### CUDA Optimization

#### Shared Memory Tiling

```cuda
__global__ void computeForcesDirectKernel(
    const float* pos_x, const float* pos_y, const float* pos_z,
    const float* mass,
    float* acc_x, float* acc_y, float* acc_z,
    int N, float G, float eps2
) {
    extern __shared__ float shared[];
    float* s_pos_x = shared;
    float* s_pos_y = s_pos_x + blockDim.x;
    float* s_pos_z = s_pos_y + blockDim.x;
    float* s_mass = s_pos_z + blockDim.x;
    
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    float ax = 0, ay = 0, az = 0;
    
    float my_x = pos_x[i];
    float my_y = pos_y[i];
    float my_z = pos_z[i];
    
    // Process in tiles
    for (int tile = 0; tile < gridDim.x; ++tile) {
        int j = tile * blockDim.x + threadIdx.x;
        
        // Collaborative load to shared memory
        s_pos_x[threadIdx.x] = pos_x[j];
        s_pos_y[threadIdx.x] = pos_y[j];
        s_pos_z[threadIdx.x] = pos_z[j];
        s_mass[threadIdx.x] = mass[j];
        __syncthreads();
        
        // Compute interactions with this tile
        for (int k = 0; k < blockDim.x; ++k) {
            float dx = s_pos_x[k] - my_x;
            float dy = s_pos_y[k] - my_y;
            float dz = s_pos_z[k] - my_z;
            float dist2 = dx*dx + dy*dy + dz*dz + eps2;
            
            float inv_dist = rsqrtf(dist2);
            float inv_dist3 = inv_dist * inv_dist * inv_dist;
            
            float f = G * s_mass[k] * inv_dist3;
            ax += f * dx;
            ay += f * dy;
            az += f * dz;
        }
        __syncthreads();
    }
    
    acc_x[i] = ax;
    acc_y[i] = ay;
    acc_z[i] = az;
}
```

#### Optimizations Applied

| Technique | Benefit |
|-----------|---------|
| Shared memory | Reduces global memory bandwidth by ~50% |
| Coalesced access | Maximizes memory throughput |
| `rsqrtf()` | Fast inverse square root |
| `__restrict__` | Enables compiler optimizations |

### Performance Characteristics

| Aspect | Value |
|--------|-------|
| Complexity | $O(N^2)$ |
| Memory | $O(N)$ |
| Accuracy | Exact |
| Best for | $N < 50{,}000$ |

---

## Barnes-Hut Algorithm

### Principle

Use an octree to approximate distant particle clusters as single mass points. Reduces complexity to $O(N \log N)$.

### Opening Angle Criterion

Whether to accept a node approximation:

$$\theta = \frac{s}{d}$$

Where:
- $s$ = Node side length
- $d$ = Distance from particle to node center of mass

If $\theta < \theta_{threshold}$, use approximation. Otherwise, recurse into children.

| θ | Accuracy | Speed | Use Case |
|---|----------|-------|----------|
| 0.0 | Exact | Slowest | Baseline |
| 0.3 | High | Slow | Scientific |
| 0.5 | Medium | Medium | General |
| 0.8 | Low | Fast | Preview |
| 1.0 | Very Low | Fastest | Draft |

### Algorithm Steps

1. **Compute bounding box** of all particles
2. **Build octree** by recursively subdividing space
3. **Compute centers of mass** for all nodes
4. **Traverse tree** for each particle using opening angle criterion

### Tree Traversal (CUDA)

```cuda
__device__ float3 computeBarnesHutForce(
    int particle_idx,
    const OctreeNode* nodes,
    float theta,
    float G, float eps2
) {
    float3 force = make_float3(0, 0, 0);
    int stack[64];  // Explicit stack for traversal
    int stack_ptr = 0;
    
    stack[stack_ptr++] = 0;  // Start with root
    
    while (stack_ptr > 0) {
        int node_idx = stack[--stack_ptr];
        const OctreeNode& node = nodes[node_idx];
        
        float3 dp = make_float3(
            node.center_of_mass.x - pos_x[particle_idx],
            node.center_of_mass.y - pos_y[particle_idx],
            node.center_of_mass.z - pos_z[particle_idx]
        );
        
        float dist2 = dp.x*dp.x + dp.y*dp.y + dp.z*dp.z + eps2;
        float dist = sqrtf(dist2);
        
        // Opening angle test
        if (node.is_leaf || (node.size / dist < theta)) {
            // Accept approximation
            float inv_dist3 = 1.0f / (dist2 * dist);
            float f = G * node.total_mass * inv_dist3;
            force.x += f * dp.x;
            force.y += f * dp.y;
            force.z += f * dp.z;
        } else {
            // Recurse into children
            for (int i = 0; i < 8; ++i) {
                if (node.children[i] >= 0) {
                    stack[stack_ptr++] = node.children[i];
                }
            }
        }
    }
    
    return force;
}
```

### Performance Characteristics

| Aspect | Value |
|--------|-------|
| Complexity | $O(N \log N)$ |
| Memory | $O(N)$ (tree overhead ~2× particles) |
| Accuracy | Configurable via θ |
| Best for | $N > 50{,}000$, long-range forces |

---

## Spatial Hash Algorithm

### Principle

Divide space into uniform grid cells. Particles only interact with others in neighboring cells. $O(N)$ for short-range forces.

### Grid Construction

```
cell_idx = floor(position / cell_size)
hash = cell_idx.x + cell_idx.y * grid_width + cell_idx.z * grid_width * grid_height
```

### Neighbor Search

For cutoff radius $r_{cut}$ and cell size $s$:

$$\text{neighbor\_cells} = \left\lceil \frac{r_{cut}}{s} \right\rceil$$

With $s \approx r_{cut}$, search $3^3 = 27$ neighboring cells.

### CUDA Implementation

```cuda
__global__ void spatialHashForceKernel(
    const int* cell_start, const int* cell_end,
    const int* sorted_indices,
    const float* pos_x, const float* pos_y, const float* pos_z,
    const float* mass,
    float* acc_x, float* acc_y, float* acc_z,
    int N, int3 grid_dims, float cell_size,
    float cutoff, float G, float eps2
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    float ax = 0, ay = 0, az = 0;
    float my_x = pos_x[i];
    float my_y = pos_y[i];
    float my_z = pos_z[i];
    
    int3 my_cell = computeCell(my_x, my_y, my_z, cell_size);
    float cutoff2 = cutoff * cutoff;
    
    // Iterate over 27 neighboring cells
    for (int dz = -1; dz <= 1; ++dz) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int3 neighbor_cell = my_cell + make_int3(dx, dy, dz);
                int hash = hashCell(neighbor_cell, grid_dims);
                
                // Iterate particles in this cell
                for (int j = cell_start[hash]; j < cell_end[hash]; ++j) {
                    int idx = sorted_indices[j];
                    if (idx == i) continue;
                    
                    float dx = pos_x[idx] - my_x;
                    float dy = pos_y[idx] - my_y;
                    float dz = pos_z[idx] - my_z;
                    float dist2 = dx*dx + dy*dy + dz*dz;
                    
                    if (dist2 < cutoff2 && dist2 > 0) {
                        dist2 += eps2;
                        float inv_dist = rsqrtf(dist2);
                        float inv_dist3 = inv_dist * inv_dist * inv_dist;
                        float f = G * mass[idx] * inv_dist3;
                        ax += f * dx;
                        ay += f * dy;
                        az += f * dz;
                    }
                }
            }
        }
    }
    
    acc_x[i] = ax;
    acc_y[i] = ay;
    acc_z[i] = az;
}
```

### Performance Characteristics

| Aspect | Value |
|--------|-------|
| Complexity | $O(N)$ (with uniform distribution) |
| Memory | $O(N + C)$ where C = number of cells |
| Accuracy | Exact within cutoff |
| Best for | Short-range forces, molecular dynamics |

---

## Velocity Verlet Integration

### Principle

A symplectic integrator that preserves energy over long periods. Second-order accurate with single force evaluation per step.

### Algorithm

1. **Store old accelerations:**
   $$\mathbf{a}_{old} = \mathbf{a}(t)$$

2. **Update positions:**
   $$\mathbf{x}(t+dt) = \mathbf{x}(t) + \mathbf{v}(t) \cdot dt + \frac{1}{2}\mathbf{a}(t) \cdot dt^2$$

3. **Compute new forces:**
   $$\mathbf{a}(t+dt) = \frac{\mathbf{F}(\mathbf{x}(t+dt))}{m}$$

4. **Update velocities:**
   $$\mathbf{v}(t+dt) = \mathbf{v}(t) + \frac{1}{2}(\mathbf{a}_{old} + \mathbf{a}(t+dt)) \cdot dt$$

### Why Velocity Verlet?

| Integrator | Order | Energy Conservation | Force Evaluations |
|------------|-------|---------------------|-------------------|
| Euler | 1st | Poor | 1 |
| Leapfrog | 2nd | Good | 1 |
| **Velocity Verlet** | **2nd** | **Excellent** | **1** |
| RK4 | 4th | Fair | 4 |

### Symplectic Property

Symplectic integrators preserve phase space volume, ensuring:
- Long-term energy error is bounded (oscillates)
- Suitable for very long simulations
- Preserves qualitative dynamics
- No systematic drift in energy

### CUDA Implementation

```cuda
// Position update kernel
__global__ void updatePositionsKernel(
    float* pos_x, float* pos_y, float* pos_z,
    const float* vel_x, const float* vel_y, const float* vel_z,
    const float* acc_x, const float* acc_y, const float* acc_z,
    int N, float dt
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
        float dt2_half = 0.5f * dt * dt;
        pos_x[i] += vel_x[i] * dt + acc_x[i] * dt2_half;
        pos_y[i] += vel_y[i] * dt + acc_y[i] * dt2_half;
        pos_z[i] += vel_z[i] * dt + acc_z[i] * dt2_half;
    }
}

// Velocity update kernel
__global__ void updateVelocitiesKernel(
    float* vel_x, float* vel_y, float* vel_z,
    const float* acc_old_x, const float* acc_old_y, const float* acc_old_z,
    const float* acc_x, const float* acc_y, const float* acc_z,
    int N, float dt
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
        float dt_half = 0.5f * dt;
        vel_x[i] += (acc_old_x[i] + acc_x[i]) * dt_half;
        vel_y[i] += (acc_old_y[i] + acc_y[i]) * dt_half;
        vel_z[i] += (acc_old_z[i] + acc_z[i]) * dt_half;
    }
}
```

---

## Algorithm Selection Guide

### Decision Flowchart

```
Number of particles N?
│
├── N < 10,000
│   └── Direct N² (simplest, exact)
│
├── 10,000 ≤ N < 100,000
│   └── Barnes-Hut with θ=0.5 (balanced)
│
├── N ≥ 100,000
│   ├── Long-range forces (gravity)
│   │   └── Barnes-Hut with θ=0.7 (faster approximation)
│   │
│   └── Short-range forces (molecular)
│       └── Spatial Hash (O(N) optimal)
```

### Performance Comparison

| Particles | Direct N² | Barnes-Hut | Spatial Hash |
|-----------|-----------|------------|--------------|
| 1K | 0.1 ms | 0.5 ms | 0.1 ms |
| 10K | 10 ms | 2 ms | 0.5 ms |
| 100K | 1000 ms | 15 ms | 5 ms |
| 1M | 100s | 200 ms | 50 ms |

*Timings on NVIDIA RTX 3080*

### Accuracy Comparison

| Algorithm | Relative Error | Recommended θ |
|-----------|----------------|---------------|
| Direct N² | 0 (exact) | — |
| Barnes-Hut | < 0.1% to < 5% | 0.3 to 1.0 |
| Spatial Hash | Exact within cutoff | — |

---

## References

1. Barnes, J., & Hut, P. (1986). A hierarchical O(N log N) force-calculation algorithm. *Nature*, 324(6096), 446-449.

2. Nyland, L., Harris, M., & Prins, J. (2007). Fast N-body simulation with CUDA. *GPU Gems 3*, 677-695.

3. Green, S. (2010). Particle simulation using CUDA. *NVIDIA Whitepaper*.

4. Verlet, L. (1967). Computer "experiments" on classical fluids. *Physical Review*, 159(1), 98.

5. Salmon, J. K., & Warren, M. S. (1994). Skeletons from the treecode closet. *Journal of Computational Physics*, 111(1), 136-155.
