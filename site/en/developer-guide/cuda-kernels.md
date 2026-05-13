# CUDA Kernels

GPU kernel implementation details.

## Force Kernels

### Direct N²

```cpp
__global__ void direct_force_kernel(
    const float* pos_x, const float* pos_y, const float* pos_z,
    const float* mass,
    float* force_x, float* force_y, float* force_z,
    int n, float softening
);
```

### Barnes-Hut

```cpp
__global__ void barnes_hut_force_kernel(
    const OctreeNode* nodes,
    const float* pos_x, const float* pos_y, const float* pos_z,
    float* force_x, float* force_y, float* force_z,
    int n, float theta
);
```

### Spatial Hash

```cpp
__global__ void spatial_hash_force_kernel(
    const float* pos_x, const float* pos_y, const float* pos_z,
    const float* mass,
    const int* cell_start, const int* cell_end,
    float* force_x, float* force_y, float* force_z,
    int n, float cutoff, float cell_size
);
```

## Integration Kernel

```cpp
__global__ void velocity_verlet_kernel(
    float* pos_x, float* pos_y, float* pos_z,
    float* vel_x, float* vel_y, float* vel_z,
    const float* force_x, const float* force_y, const float* force_z,
    const float* mass,
    int n, float dt
);
```

## Optimization

- **Memory coalescing**: Use SoA layout
- **Shared memory**: Cache frequently accessed data
- **Occupancy**: Balance threads and registers
