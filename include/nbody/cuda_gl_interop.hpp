#pragma once

#include "nbody/types.hpp"
#include <GL/glew.h>
#include <cuda_gl_interop.h>

namespace nbody {

// CUDA-OpenGL interoperability for zero-copy rendering
class CudaGLInterop {
public:
  CudaGLInterop();
  ~CudaGLInterop();

  // Initialize with particle count
  void initialize(size_t particle_count);

  // Cleanup resources
  void cleanup();

  // Map OpenGL buffer to CUDA (returns device pointer)
  float* mapPositionBuffer();

  // Map velocity buffer to CUDA (returns device pointer)
  float* mapVelocityBuffer();

  // Unmap buffers (must be called before OpenGL rendering)
  void unmapPositionBuffer();
  void unmapVelocityBuffer();
  void unmapAllBuffers();

  // Check if buffer is currently mapped
  bool isMapped() const { return is_mapped_; }
  bool isVelocityMapped() const { return is_velocity_mapped_; }

  // Get OpenGL VBO handles for rendering
  GLuint getPositionVBO() const { return position_vbo_; }
  GLuint getVelocityVBO() const { return velocity_vbo_; }

  // Get particle count
  size_t getParticleCount() const { return particle_count_; }

  // Copy positions from ParticleData to interop buffer
  void updatePositions(const ParticleData* d_particles);

  // Copy velocities from ParticleData to interop buffer
  void updateVelocities(const ParticleData* d_particles);

  // Verify data integrity (for testing)
  bool verifyDataIntegrity(const float* expected_data, size_t count);

private:
  GLuint position_vbo_;                 // OpenGL Vertex Buffer Object for positions
  GLuint velocity_vbo_;                 // OpenGL Vertex Buffer Object for velocities
  cudaGraphicsResource* cuda_vbo_;      // CUDA graphics resource handle for positions
  cudaGraphicsResource* cuda_vel_vbo_;  // CUDA graphics resource handle for velocities
  bool is_mapped_;
  bool is_velocity_mapped_;
  bool is_initialized_;
  size_t particle_count_;
};

// GPU kernel to copy SoA positions to interleaved VBO format
void launchCopyPositionsToVBOKernel(float* d_vbo, const ParticleData* d_particles, int block_size);

// GPU kernel to copy SoA velocities to interleaved VBO format
void launchCopyVelocitiesToVBOKernel(float* d_vbo, const ParticleData* d_particles, int block_size);

}  // namespace nbody
