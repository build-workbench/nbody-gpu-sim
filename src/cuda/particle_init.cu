#include "nbody/error_handling.hpp"
#include "nbody/particle_data.hpp"
#include <cuda_runtime.h>

namespace nbody {

// Particle initialization runs on the host (ParticleInitializer in this
// file); the device side only needs memory management and copies.

// ParticleDataManager implementation
void ParticleDataManager::allocateDevice(ParticleData& data, size_t count) {
  // Start from a clean, nulled-out struct so the cleanup below is safe if a
  // later allocation throws (freeDevice skips null pointers).
  data = ParticleData();
  data.count = count;
  const size_t size = count * sizeof(float);

  try {
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
  } catch (...) {
    // Don't leak the allocations that succeeded before the failure.
    freeDevice(data);
    throw;
  }
}

void ParticleDataManager::freeDevice(ParticleData& data) {
  if (data.pos_x)
    cudaFree(data.pos_x);
  if (data.pos_y)
    cudaFree(data.pos_y);
  if (data.pos_z)
    cudaFree(data.pos_z);
  if (data.vel_x)
    cudaFree(data.vel_x);
  if (data.vel_y)
    cudaFree(data.vel_y);
  if (data.vel_z)
    cudaFree(data.vel_z);
  if (data.acc_x)
    cudaFree(data.acc_x);
  if (data.acc_y)
    cudaFree(data.acc_y);
  if (data.acc_z)
    cudaFree(data.acc_z);
  if (data.acc_old_x)
    cudaFree(data.acc_old_x);
  if (data.acc_old_y)
    cudaFree(data.acc_old_y);
  if (data.acc_old_z)
    cudaFree(data.acc_old_z);
  if (data.mass)
    cudaFree(data.mass);
  data = ParticleData();
}

void ParticleDataManager::allocateHost(ParticleData& data, size_t count) {
  data.count = count;

  // Use exception-safe allocation with cleanup on failure
  // If any allocation fails, previously allocated memory will be freed
  try {
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
  } catch (const std::bad_alloc& e) {
    // Clean up any allocations that succeeded before the failure
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
    throw ResourceException("Failed to allocate host memory for particles",
                            count * 13 * sizeof(float), 0);
  }
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

    // Orbital velocity (v ∝ √r, solid-body-like; see the GPU kernel note)
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

}  // namespace nbody
