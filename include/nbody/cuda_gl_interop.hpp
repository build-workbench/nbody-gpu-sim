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
    
    // Unmap buffer (must be called before OpenGL rendering)
    void unmapPositionBuffer();
    
    // Check if buffer is currently mapped
    bool isMapped() const { return is_mapped_; }
    
    // Get OpenGL VBO handle for rendering
    GLuint getPositionVBO() const { return position_vbo_; }
    
    // Get particle count
    size_t getParticleCount() const { return particle_count_; }
    
    // Copy positions from ParticleData to interop buffer
    void updatePositions(const ParticleData* d_particles);
    
    // Verify data integrity (for testing)
    bool verifyDataIntegrity(const float* expected_data, size_t count);
    
private:
    GLuint position_vbo_;              // OpenGL Vertex Buffer Object
    cudaGraphicsResource* cuda_vbo_;   // CUDA graphics resource handle
    bool is_mapped_;
    bool is_initialized_;
    size_t particle_count_;
};

// GPU kernel to copy SoA positions to interleaved VBO format
void launchCopyPositionsToVBOKernel(float* d_vbo, const ParticleData* d_particles, int block_size);

} // namespace nbody
