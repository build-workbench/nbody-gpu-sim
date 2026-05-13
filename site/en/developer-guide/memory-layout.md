# Memory Layout

GPU memory organization.

## Structure of Arrays (SoA)

```cpp
struct ParticleData {
    float* position_x;  // N floats
    float* position_y;
    float* position_z;
    float* velocity_x;
    float* velocity_y;
    float* velocity_z;
    float* force_x;
    float* force_y;
    float* force_z;
    float* mass;
    size_t count;
};
```

## Memory Coalescing

SoA layout ensures consecutive threads access consecutive memory:

```
Thread 0: position_x[0]
Thread 1: position_x[1]
Thread 2: position_x[2]
...
```

## Allocation

```cpp
void ParticleData::allocate(size_t n) {
    cudaMalloc(&position_x, n * sizeof(float));
    cudaMalloc(&position_y, n * sizeof(float));
    // ... etc
}
```

## Zero-Copy with OpenGL

```cpp
// Create CUDA-OpenGL buffer
GLuint vbo;
cudaGraphicsGLRegisterBuffer(&resource, vbo, cudaGraphicsMapFlagsNone);

// Map for CUDA access
float* positions;
cudaGraphicsMapResources(1, &resource);
cudaGraphicsResourceGetMappedPointer((void**)&positions, &size, resource);
```
