#pragma once

#include "nbody/types.hpp"
#include <random>

namespace nbody {

// Particle data memory management
class ParticleDataManager {
public:
    // Allocate device memory for particles
    static void allocateDevice(ParticleData& data, size_t count);
    
    // Free device memory
    static void freeDevice(ParticleData& data);
    
    // Allocate host memory for particles
    static void allocateHost(ParticleData& data, size_t count);
    
    // Free host memory
    static void freeHost(ParticleData& data);
    
    // Copy data from host to device
    static void copyToDevice(ParticleData& d_data, const ParticleData& h_data);
    
    // Copy data from device to host
    static void copyToHost(ParticleData& h_data, const ParticleData& d_data);
    
    // Copy only positions from device to host
    static void copyPositionsToHost(float* h_pos_x, float* h_pos_y, float* h_pos_z,
                                    const ParticleData& d_data);
    
    // Copy only positions from host to device
    static void copyPositionsToDevice(ParticleData& d_data,
                                      const float* h_pos_x, const float* h_pos_y, const float* h_pos_z);
};

// Particle initialization
class ParticleInitializer {
public:
    // Initialize with uniform distribution
    static void initUniform(ParticleData& h_data, const UniformDistParams& params,
                           unsigned int seed = 42);
    
    // Initialize with spherical distribution
    static void initSpherical(ParticleData& h_data, const SphericalDistParams& params,
                             unsigned int seed = 42);
    
    // Initialize with disk distribution (galaxy-like)
    static void initDisk(ParticleData& h_data, const DiskDistParams& params,
                        unsigned int seed = 42);
    
    // Zero out velocities and accelerations
    static void zeroVelocities(ParticleData& h_data);
    static void zeroAccelerations(ParticleData& h_data);
    
private:
    static std::mt19937 createRNG(unsigned int seed);
};

// GPU kernels for particle initialization (declared in .cu file)
void launchInitUniformKernel(ParticleData* d_data, const UniformDistParams& params,
                             unsigned int seed, int block_size);
void launchInitSphericalKernel(ParticleData* d_data, const SphericalDistParams& params,
                               unsigned int seed, int block_size);
void launchInitDiskKernel(ParticleData* d_data, const DiskDistParams& params,
                          unsigned int seed, int block_size);

} // namespace nbody
