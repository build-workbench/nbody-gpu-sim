#include "nbody/types.hpp"
#include "nbody/force_calculator.hpp"
#include "nbody/error_handling.hpp"
#include <cuda_runtime.h>

namespace nbody {

// Direct N² force calculation kernel with shared memory tiling
__global__ void computeForcesDirectKernel(
    const float* __restrict__ pos_x,
    const float* __restrict__ pos_y,
    const float* __restrict__ pos_z,
    const float* __restrict__ mass,
    float* acc_x,
    float* acc_y,
    float* acc_z,
    int N,
    float G,
    float eps2  // softening^2
) {
    extern __shared__ float shared_data[];
    
    float* s_pos_x = shared_data;
    float* s_pos_y = s_pos_x + blockDim.x;
    float* s_pos_z = s_pos_y + blockDim.x;
    float* s_mass = s_pos_z + blockDim.x;
    
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    
    float ax = 0.0f, ay = 0.0f, az = 0.0f;
    float xi = 0.0f, yi = 0.0f, zi = 0.0f;
    
    if (i < N) {
        xi = pos_x[i];
        yi = pos_y[i];
        zi = pos_z[i];
    }
    
    // Process particles in tiles
    int num_tiles = (N + blockDim.x - 1) / blockDim.x;
    
    for (int tile = 0; tile < num_tiles; tile++) {
        int j = tile * blockDim.x + threadIdx.x;
        
        // Cooperatively load tile data into shared memory
        if (j < N) {
            s_pos_x[threadIdx.x] = pos_x[j];
            s_pos_y[threadIdx.x] = pos_y[j];
            s_pos_z[threadIdx.x] = pos_z[j];
            s_mass[threadIdx.x] = mass[j];
        } else {
            s_pos_x[threadIdx.x] = 0.0f;
            s_pos_y[threadIdx.x] = 0.0f;
            s_pos_z[threadIdx.x] = 0.0f;
            s_mass[threadIdx.x] = 0.0f;
        }
        __syncthreads();
        
        // Compute interactions with particles in this tile
        if (i < N) {
            #pragma unroll 8
            for (int k = 0; k < blockDim.x; k++) {
                int global_j = tile * blockDim.x + k;
                if (global_j < N && global_j != i) {
                    float dx = s_pos_x[k] - xi;
                    float dy = s_pos_y[k] - yi;
                    float dz = s_pos_z[k] - zi;
                    
                    float dist2 = dx*dx + dy*dy + dz*dz + eps2;
                    float inv_dist = rsqrtf(dist2);
                    float inv_dist3 = inv_dist * inv_dist * inv_dist;
                    
                    float f = G * s_mass[k] * inv_dist3;
                    
                    ax += f * dx;
                    ay += f * dy;
                    az += f * dz;
                }
            }
        }
        __syncthreads();
    }
    
    if (i < N) {
        acc_x[i] = ax;
        acc_y[i] = ay;
        acc_z[i] = az;
    }
}

// Launch wrapper
void launchDirectForceKernel(ParticleData* d_particles, float G, float eps2, int block_size) {
    int N = static_cast<int>(d_particles->count);
    int num_blocks = (N + block_size - 1) / block_size;
    size_t shared_mem_size = 4 * block_size * sizeof(float);  // pos_x, pos_y, pos_z, mass
    
    computeForcesDirectKernel<<<num_blocks, block_size, shared_mem_size>>>(
        d_particles->pos_x, d_particles->pos_y, d_particles->pos_z,
        d_particles->mass,
        d_particles->acc_x, d_particles->acc_y, d_particles->acc_z,
        N, G, eps2
    );
    
    CUDA_CHECK_KERNEL();
}

// DirectForceCalculator implementation
DirectForceCalculator::DirectForceCalculator(int block_size)
    : block_size_(block_size) {}

void DirectForceCalculator::computeForces(ParticleData* d_particles) {
    launchDirectForceKernel(d_particles, G_, softening_eps2_, block_size_);
}

// CPU reference implementation for testing
Vec3 computeGravitationalForceCPU(const Vec3& p1, const Vec3& p2,
                                   float m1, float m2, float G, float eps) {
    Vec3 r = p2 - p1;
    float dist2 = r.length2() + eps * eps;
    float inv_dist = 1.0f / sqrtf(dist2);
    float inv_dist3 = inv_dist * inv_dist * inv_dist;
    float f = G * m2 * inv_dist3;
    return r * f;
}

} // namespace nbody
