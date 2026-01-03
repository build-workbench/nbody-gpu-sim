#include "nbody/integrator.hpp"
#include "nbody/error_handling.hpp"
#include <cuda_runtime.h>
#include <cub/cub.cuh>

namespace nbody {

// Position update kernel: x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt^2
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

// Velocity update kernel: v(t+dt) = v(t) + 0.5*(a(t) + a(t+dt))*dt
__global__ void updateVelocitiesKernel(
    float* vel_x, float* vel_y, float* vel_z,
    const float* acc_old_x, const float* acc_old_y, const float* acc_old_z,
    const float* acc_new_x, const float* acc_new_y, const float* acc_new_z,
    int N, float dt
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
        float dt_half = 0.5f * dt;
        vel_x[i] += (acc_old_x[i] + acc_new_x[i]) * dt_half;
        vel_y[i] += (acc_old_y[i] + acc_new_y[i]) * dt_half;
        vel_z[i] += (acc_old_z[i] + acc_new_z[i]) * dt_half;
    }
}

// Store current accelerations as old accelerations
__global__ void storeAccelerationsKernel(
    const float* acc_x, const float* acc_y, const float* acc_z,
    float* acc_old_x, float* acc_old_y, float* acc_old_z,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
        acc_old_x[i] = acc_x[i];
        acc_old_y[i] = acc_y[i];
        acc_old_z[i] = acc_z[i];
    }
}

// Kinetic energy kernel: KE = 0.5 * sum(m * v^2)
__global__ void computeKineticEnergyKernel(
    const float* vel_x, const float* vel_y, const float* vel_z,
    const float* mass,
    float* partial_ke,
    int N
) {
    extern __shared__ float shared_ke[];
    
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int tid = threadIdx.x;
    
    float ke = 0.0f;
    if (i < N) {
        float v2 = vel_x[i]*vel_x[i] + vel_y[i]*vel_y[i] + vel_z[i]*vel_z[i];
        ke = 0.5f * mass[i] * v2;
    }
    shared_ke[tid] = ke;
    __syncthreads();
    
    // Block reduction
    for (int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            shared_ke[tid] += shared_ke[tid + s];
        }
        __syncthreads();
    }
    
    if (tid == 0) {
        partial_ke[blockIdx.x] = shared_ke[0];
    }
}

// Potential energy kernel: PE = -G * sum(m_i * m_j / r_ij) for i < j
__global__ void computePotentialEnergyKernel(
    const float* pos_x, const float* pos_y, const float* pos_z,
    const float* mass,
    float* partial_pe,
    int N, float G, float eps
) {
    extern __shared__ float shared_pe[];
    
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int tid = threadIdx.x;
    
    float pe = 0.0f;
    if (i < N) {
        float xi = pos_x[i];
        float yi = pos_y[i];
        float zi = pos_z[i];
        float mi = mass[i];
        
        // Only count pairs where j > i to avoid double counting
        for (int j = i + 1; j < N; j++) {
            float dx = pos_x[j] - xi;
            float dy = pos_y[j] - yi;
            float dz = pos_z[j] - zi;
            float r = sqrtf(dx*dx + dy*dy + dz*dz + eps*eps);
            pe -= G * mi * mass[j] / r;
        }
    }
    shared_pe[tid] = pe;
    __syncthreads();
    
    // Block reduction
    for (int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            shared_pe[tid] += shared_pe[tid + s];
        }
        __syncthreads();
    }
    
    if (tid == 0) {
        partial_pe[blockIdx.x] = shared_pe[0];
    }
}

// Launch wrappers
void launchUpdatePositionsKernel(ParticleData* d_particles, float dt, int block_size) {
    int N = static_cast<int>(d_particles->count);
    int num_blocks = (N + block_size - 1) / block_size;
    
    updatePositionsKernel<<<num_blocks, block_size>>>(
        d_particles->pos_x, d_particles->pos_y, d_particles->pos_z,
        d_particles->vel_x, d_particles->vel_y, d_particles->vel_z,
        d_particles->acc_x, d_particles->acc_y, d_particles->acc_z,
        N, dt
    );
    CUDA_CHECK_KERNEL();
}

void launchUpdateVelocitiesKernel(ParticleData* d_particles, float dt, int block_size) {
    int N = static_cast<int>(d_particles->count);
    int num_blocks = (N + block_size - 1) / block_size;
    
    updateVelocitiesKernel<<<num_blocks, block_size>>>(
        d_particles->vel_x, d_particles->vel_y, d_particles->vel_z,
        d_particles->acc_old_x, d_particles->acc_old_y, d_particles->acc_old_z,
        d_particles->acc_x, d_particles->acc_y, d_particles->acc_z,
        N, dt
    );
    CUDA_CHECK_KERNEL();
}

void launchStoreAccelerationsKernel(ParticleData* d_particles, int block_size) {
    int N = static_cast<int>(d_particles->count);
    int num_blocks = (N + block_size - 1) / block_size;
    
    storeAccelerationsKernel<<<num_blocks, block_size>>>(
        d_particles->acc_x, d_particles->acc_y, d_particles->acc_z,
        d_particles->acc_old_x, d_particles->acc_old_y, d_particles->acc_old_z,
        N
    );
    CUDA_CHECK_KERNEL();
}

float launchComputeKineticEnergyKernel(const ParticleData* d_particles, int block_size) {
    int N = static_cast<int>(d_particles->count);
    int num_blocks = (N + block_size - 1) / block_size;
    
    float* d_partial;
    CUDA_CHECK(cudaMalloc(&d_partial, num_blocks * sizeof(float)));
    
    computeKineticEnergyKernel<<<num_blocks, block_size, block_size * sizeof(float)>>>(
        d_particles->vel_x, d_particles->vel_y, d_particles->vel_z,
        d_particles->mass, d_partial, N
    );
    CUDA_CHECK_KERNEL();
    
    // Sum partial results on host
    std::vector<float> h_partial(num_blocks);
    CUDA_CHECK(cudaMemcpy(h_partial.data(), d_partial, num_blocks * sizeof(float), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(d_partial));
    
    float total_ke = 0.0f;
    for (int i = 0; i < num_blocks; i++) {
        total_ke += h_partial[i];
    }
    return total_ke;
}

float launchComputePotentialEnergyKernel(const ParticleData* d_particles, float G, float eps, int block_size) {
    int N = static_cast<int>(d_particles->count);
    int num_blocks = (N + block_size - 1) / block_size;
    
    float* d_partial;
    CUDA_CHECK(cudaMalloc(&d_partial, num_blocks * sizeof(float)));
    
    computePotentialEnergyKernel<<<num_blocks, block_size, block_size * sizeof(float)>>>(
        d_particles->pos_x, d_particles->pos_y, d_particles->pos_z,
        d_particles->mass, d_partial, N, G, eps
    );
    CUDA_CHECK_KERNEL();
    
    // Sum partial results on host
    std::vector<float> h_partial(num_blocks);
    CUDA_CHECK(cudaMemcpy(h_partial.data(), d_partial, num_blocks * sizeof(float), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(d_partial));
    
    float total_pe = 0.0f;
    for (int i = 0; i < num_blocks; i++) {
        total_pe += h_partial[i];
    }
    return total_pe;
}

// Integrator class implementation
Integrator::Integrator(int block_size) : block_size_(block_size) {}

void Integrator::integrate(ParticleData* d_particles, ForceCalculator* force_calc, float dt) {
    // Velocity Verlet integration:
    // 1. Store old accelerations
    storeOldAccelerations(d_particles);
    
    // 2. Update positions: x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt^2
    updatePositions(d_particles, dt);
    
    // 3. Compute new forces/accelerations at new positions
    force_calc->computeForces(d_particles);
    
    // 4. Update velocities: v(t+dt) = v(t) + 0.5*(a(t) + a(t+dt))*dt
    updateVelocities(d_particles, dt);
}

void Integrator::updatePositions(ParticleData* d_particles, float dt) {
    launchUpdatePositionsKernel(d_particles, dt, block_size_);
}

void Integrator::updateVelocities(ParticleData* d_particles, float dt) {
    launchUpdateVelocitiesKernel(d_particles, dt, block_size_);
}

void Integrator::storeOldAccelerations(ParticleData* d_particles) {
    launchStoreAccelerationsKernel(d_particles, block_size_);
}

float Integrator::computeKineticEnergy(const ParticleData* d_particles) {
    return launchComputeKineticEnergyKernel(d_particles, block_size_);
}

float Integrator::computePotentialEnergy(const ParticleData* d_particles, float G, float eps) {
    return launchComputePotentialEnergyKernel(d_particles, G, eps, block_size_);
}

float Integrator::computeTotalEnergy(const ParticleData* d_particles, float G, float eps) {
    return computeKineticEnergy(d_particles) + computePotentialEnergy(d_particles, G, eps);
}

} // namespace nbody
