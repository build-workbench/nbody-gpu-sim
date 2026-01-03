#include "nbody/particle_data.hpp"
#include "nbody/error_handling.hpp"
#include <cuda_runtime.h>
#include <curand_kernel.h>

namespace nbody {

// CUDA kernel for uniform distribution initialization
__global__ void initUniformKernel(
    float* pos_x, float* pos_y, float* pos_z,
    float* vel_x, float* vel_y, float* vel_z,
    float* mass,
    int N,
    float min_x, float min_y, float min_z,
    float max_x, float max_y, float max_z,
    float min_mass, float max_mass,
    unsigned int seed
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    curandState state;
    curand_init(seed, i, 0, &state);
    
    pos_x[i] = min_x + curand_uniform(&state) * (max_x - min_x);
    pos_y[i] = min_y + curand_uniform(&state) * (max_y - min_y);
    pos_z[i] = min_z + curand_uniform(&state) * (max_z - min_z);
    
    vel_x[i] = 0.0f;
    vel_y[i] = 0.0f;
    vel_z[i] = 0.0f;
    
    mass[i] = min_mass + curand_uniform(&state) * (max_mass - min_mass);
}

// CUDA kernel for spherical distribution initialization
__global__ void initSphericalKernel(
    float* pos_x, float* pos_y, float* pos_z,
    float* vel_x, float* vel_y, float* vel_z,
    float* mass,
    int N,
    float center_x, float center_y, float center_z,
    float radius,
    float min_mass, float max_mass,
    unsigned int seed
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    curandState state;
    curand_init(seed, i, 0, &state);
    
    // Generate random point in unit sphere using rejection sampling
    float x, y, z;
    do {
        x = 2.0f * curand_uniform(&state) - 1.0f;
        y = 2.0f * curand_uniform(&state) - 1.0f;
        z = 2.0f * curand_uniform(&state) - 1.0f;
    } while (x*x + y*y + z*z > 1.0f);
    
    // Scale to desired radius
    float r = cbrtf(curand_uniform(&state)) * radius;  // Uniform in volume
    float len = sqrtf(x*x + y*y + z*z);
    if (len > 0.0f) {
        x = x / len * r;
        y = y / len * r;
        z = z / len * r;
    }
    
    pos_x[i] = center_x + x;
    pos_y[i] = center_y + y;
    pos_z[i] = center_z + z;
    
    vel_x[i] = 0.0f;
    vel_y[i] = 0.0f;
    vel_z[i] = 0.0f;
    
    mass[i] = min_mass + curand_uniform(&state) * (max_mass - min_mass);
}

// CUDA kernel for disk distribution initialization
__global__ void initDiskKernel(
    float* pos_x, float* pos_y, float* pos_z,
    float* vel_x, float* vel_y, float* vel_z,
    float* mass,
    int N,
    float center_x, float center_y, float center_z,
    float radius, float thickness,
    float min_mass, float max_mass,
    float rotation_speed,
    unsigned int seed
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    curandState state;
    curand_init(seed, i, 0, &state);
    
    // Generate random point in disk
    float r = sqrtf(curand_uniform(&state)) * radius;  // Uniform in area
    float theta = curand_uniform(&state) * 2.0f * 3.14159265f;
    float z = (curand_uniform(&state) - 0.5f) * thickness;
    
    float x = r * cosf(theta);
    float y = r * sinf(theta);
    
    pos_x[i] = center_x + x;
    pos_y[i] = center_y + y;
    pos_z[i] = center_z + z;
    
    // Orbital velocity (perpendicular to radius in xy plane)
    float v = rotation_speed * sqrtf(r);  // Keplerian-like
    vel_x[i] = -v * sinf(theta);
    vel_y[i] = v * cosf(theta);
    vel_z[i] = 0.0f;
    
    mass[i] = min_mass + curand_uniform(&state) * (max_mass - min_mass);
}

// Launch wrappers
void launchInitUniformKernel(ParticleData* d_data, const UniformDistParams& params,
                             unsigned int seed, int block_size) {
    int N = static_cast<int>(d_data->count);
    int num_blocks = (N + block_size - 1) / block_size;
    
    initUniformKernel<<<num_blocks, block_size>>>(
        d_data->pos_x, d_data->pos_y, d_data->pos_z,
        d_data->vel_x, d_data->vel_y, d_data->vel_z,
        d_data->mass, N,
        params.min_bounds.x, params.min_bounds.y, params.min_bounds.z,
        params.max_bounds.x, params.max_bounds.y, params.max_bounds.z,
        params.min_mass, params.max_mass, seed
    );
    CUDA_CHECK_KERNEL();
}

void launchInitSphericalKernel(ParticleData* d_data, const SphericalDistParams& params,
                               unsigned int seed, int block_size) {
    int N = static_cast<int>(d_data->count);
    int num_blocks = (N + block_size - 1) / block_size;
    
    initSphericalKernel<<<num_blocks, block_size>>>(
        d_data->pos_x, d_data->pos_y, d_data->pos_z,
        d_data->vel_x, d_data->vel_y, d_data->vel_z,
        d_data->mass, N,
        params.center.x, params.center.y, params.center.z,
        params.radius, params.min_mass, params.max_mass, seed
    );
    CUDA_CHECK_KERNEL();
}

void launchInitDiskKernel(ParticleData* d_data, const DiskDistParams& params,
                          unsigned int seed, int block_size) {
    int N = static_cast<int>(d_data->count);
    int num_blocks = (N + block_size - 1) / block_size;
    
    initDiskKernel<<<num_blocks, block_size>>>(
        d_data->pos_x, d_data->pos_y, d_data->pos_z,
        d_data->vel_x, d_data->vel_y, d_data->vel_z,
        d_data->mass, N,
        params.center.x, params.center.y, params.center.z,
        params.radius, params.thickness,
        params.min_mass, params.max_mass,
        params.rotation_speed, seed
    );
    CUDA_CHECK_KERNEL();
}

// ParticleDataManager implementation
void ParticleDataManager::allocateDevice(ParticleData& data, size_t count) {
    data.count = count;
    size_t size = count * sizeof(float);
    
    CUDA_CHECK(cudaMalloc(&data.pos_x, size));
    CUDA_CHECK(cudaMalloc(&data.pos_y, size));
    CUDA_CHECK(cudaMalloc(&data.pos_z, size));
    CUDA_CHECK(cudaMalloc(&data.vel_x, size));
    CUDA_CHECK(cudaMalloc(&data.vel_y, size));
    CUDA_CHECK(cudaMalloc(&data.vel_z, size));
    CUDA_CHECK(cudaMalloc(&data.acc_x, size));
    CUDA_CHECK(cudaMalloc(&data.acc_y, size));
    CUDA_CHECK(cudaMalloc(&data.acc_z, size));
    CUDA_CHECK(cudaMalloc(&data.acc_old_x, size));
    CUDA_CHECK(cudaMalloc(&data.acc_old_y, size));
    CUDA_CHECK(cudaMalloc(&data.acc_old_z, size));
    CUDA_CHECK(cudaMalloc(&data.mass, size));
    
    // Initialize to zero
    CUDA_CHECK(cudaMemset(data.acc_x, 0, size));
    CUDA_CHECK(cudaMemset(data.acc_y, 0, size));
    CUDA_CHECK(cudaMemset(data.acc_z, 0, size));
    CUDA_CHECK(cudaMemset(data.acc_old_x, 0, size));
    CUDA_CHECK(cudaMemset(data.acc_old_y, 0, size));
    CUDA_CHECK(cudaMemset(data.acc_old_z, 0, size));
}

void ParticleDataManager::freeDevice(ParticleData& data) {
    if (data.pos_x) cudaFree(data.pos_x);
    if (data.pos_y) cudaFree(data.pos_y);
    if (data.pos_z) cudaFree(data.pos_z);
    if (data.vel_x) cudaFree(data.vel_x);
    if (data.vel_y) cudaFree(data.vel_y);
    if (data.vel_z) cudaFree(data.vel_z);
    if (data.acc_x) cudaFree(data.acc_x);
    if (data.acc_y) cudaFree(data.acc_y);
    if (data.acc_z) cudaFree(data.acc_z);
    if (data.acc_old_x) cudaFree(data.acc_old_x);
    if (data.acc_old_y) cudaFree(data.acc_old_y);
    if (data.acc_old_z) cudaFree(data.acc_old_z);
    if (data.mass) cudaFree(data.mass);
    data = ParticleData();
}

void ParticleDataManager::allocateHost(ParticleData& data, size_t count) {
    data.count = count;
    size_t size = count * sizeof(float);
    
    data.pos_x = new float[count];
    data.pos_y = new float[count];
    data.pos_z = new float[count];
    data.vel_x = new float[count];
    data.vel_y = new float[count];
    data.vel_z = new float[count];
    data.acc_x = new float[count];
    data.acc_y = new float[count];
    data.acc_z = new float[count];
    data.acc_old_x = new float[count];
    data.acc_old_y = new float[count];
    data.acc_old_z = new float[count];
    data.mass = new float[count];
}

void ParticleDataManager::freeHost(ParticleData& data) {
    delete[] data.pos_x;
    delete[] data.pos_y;
    delete[] data.pos_z;
    delete[] data.vel_x;
    delete[] data.vel_y;
    delete[] data.vel_z;
    delete[] data.acc_x;
    delete[] data.acc_y;
    delete[] data.acc_z;
    delete[] data.acc_old_x;
    delete[] data.acc_old_y;
    delete[] data.acc_old_z;
    delete[] data.mass;
    data = ParticleData();
}

void ParticleDataManager::copyToDevice(ParticleData& d_data, const ParticleData& h_data) {
    size_t size = h_data.count * sizeof(float);
    CUDA_CHECK(cudaMemcpy(d_data.pos_x, h_data.pos_x, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_data.pos_y, h_data.pos_y, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_data.pos_z, h_data.pos_z, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_data.vel_x, h_data.vel_x, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_data.vel_y, h_data.vel_y, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_data.vel_z, h_data.vel_z, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_data.acc_x, h_data.acc_x, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_data.acc_y, h_data.acc_y, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_data.acc_z, h_data.acc_z, size, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_data.mass, h_data.mass, size, cudaMemcpyHostToDevice));
}

void ParticleDataManager::copyToHost(ParticleData& h_data, const ParticleData& d_data) {
    size_t size = d_data.count * sizeof(float);
    CUDA_CHECK(cudaMemcpy(h_data.pos_x, d_data.pos_x, size, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_data.pos_y, d_data.pos_y, size, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_data.pos_z, d_data.pos_z, size, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_data.vel_x, d_data.vel_x, size, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_data.vel_y, d_data.vel_y, size, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_data.vel_z, d_data.vel_z, size, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_data.acc_x, d_data.acc_x, size, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_data.acc_y, d_data.acc_y, size, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_data.acc_z, d_data.acc_z, size, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_data.mass, d_data.mass, size, cudaMemcpyDeviceToHost));
}

// Host-side initialization (CPU)
std::mt19937 ParticleInitializer::createRNG(unsigned int seed) {
    return std::mt19937(seed);
}

void ParticleInitializer::initUniform(ParticleData& h_data, const UniformDistParams& params,
                                      unsigned int seed) {
    auto rng = createRNG(seed);
    std::uniform_real_distribution<float> dist_x(params.min_bounds.x, params.max_bounds.x);
    std::uniform_real_distribution<float> dist_y(params.min_bounds.y, params.max_bounds.y);
    std::uniform_real_distribution<float> dist_z(params.min_bounds.z, params.max_bounds.z);
    std::uniform_real_distribution<float> dist_m(params.min_mass, params.max_mass);
    
    for (size_t i = 0; i < h_data.count; i++) {
        h_data.pos_x[i] = dist_x(rng);
        h_data.pos_y[i] = dist_y(rng);
        h_data.pos_z[i] = dist_z(rng);
        h_data.vel_x[i] = 0.0f;
        h_data.vel_y[i] = 0.0f;
        h_data.vel_z[i] = 0.0f;
        h_data.mass[i] = dist_m(rng);
    }
    zeroAccelerations(h_data);
}

void ParticleInitializer::initSpherical(ParticleData& h_data, const SphericalDistParams& params,
                                        unsigned int seed) {
    auto rng = createRNG(seed);
    std::uniform_real_distribution<float> dist_01(0.0f, 1.0f);
    std::uniform_real_distribution<float> dist_m(params.min_mass, params.max_mass);
    
    for (size_t i = 0; i < h_data.count; i++) {
        // Uniform distribution in sphere volume
        float r = std::cbrt(dist_01(rng)) * params.radius;
        float theta = dist_01(rng) * 2.0f * 3.14159265f;
        float phi = std::acos(2.0f * dist_01(rng) - 1.0f);
        
        h_data.pos_x[i] = params.center.x + r * std::sin(phi) * std::cos(theta);
        h_data.pos_y[i] = params.center.y + r * std::sin(phi) * std::sin(theta);
        h_data.pos_z[i] = params.center.z + r * std::cos(phi);
        h_data.vel_x[i] = 0.0f;
        h_data.vel_y[i] = 0.0f;
        h_data.vel_z[i] = 0.0f;
        h_data.mass[i] = dist_m(rng);
    }
    zeroAccelerations(h_data);
}

void ParticleInitializer::initDisk(ParticleData& h_data, const DiskDistParams& params,
                                   unsigned int seed) {
    auto rng = createRNG(seed);
    std::uniform_real_distribution<float> dist_01(0.0f, 1.0f);
    std::uniform_real_distribution<float> dist_m(params.min_mass, params.max_mass);
    
    for (size_t i = 0; i < h_data.count; i++) {
        float r = std::sqrt(dist_01(rng)) * params.radius;
        float theta = dist_01(rng) * 2.0f * 3.14159265f;
        float z = (dist_01(rng) - 0.5f) * params.thickness;
        
        h_data.pos_x[i] = params.center.x + r * std::cos(theta);
        h_data.pos_y[i] = params.center.y + r * std::sin(theta);
        h_data.pos_z[i] = params.center.z + z;
        
        // Orbital velocity
        float v = params.rotation_speed * std::sqrt(r);
        h_data.vel_x[i] = -v * std::sin(theta);
        h_data.vel_y[i] = v * std::cos(theta);
        h_data.vel_z[i] = 0.0f;
        
        h_data.mass[i] = dist_m(rng);
    }
    zeroAccelerations(h_data);
}

void ParticleInitializer::zeroVelocities(ParticleData& h_data) {
    for (size_t i = 0; i < h_data.count; i++) {
        h_data.vel_x[i] = 0.0f;
        h_data.vel_y[i] = 0.0f;
        h_data.vel_z[i] = 0.0f;
    }
}

void ParticleInitializer::zeroAccelerations(ParticleData& h_data) {
    for (size_t i = 0; i < h_data.count; i++) {
        h_data.acc_x[i] = 0.0f;
        h_data.acc_y[i] = 0.0f;
        h_data.acc_z[i] = 0.0f;
        h_data.acc_old_x[i] = 0.0f;
        h_data.acc_old_y[i] = 0.0f;
        h_data.acc_old_z[i] = 0.0f;
    }
}

} // namespace nbody
