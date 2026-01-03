#include "nbody/cuda_gl_interop.hpp"
#include "nbody/error_handling.hpp"

namespace nbody {

// Kernel to copy SoA positions to interleaved VBO format
__global__ void copyPositionsToVBOKernel(
    float* vbo,
    const float* pos_x, const float* pos_y, const float* pos_z,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    vbo[i * 3 + 0] = pos_x[i];
    vbo[i * 3 + 1] = pos_y[i];
    vbo[i * 3 + 2] = pos_z[i];
}

void launchCopyPositionsToVBOKernel(float* d_vbo, const ParticleData* d_particles, int block_size) {
    int N = static_cast<int>(d_particles->count);
    int num_blocks = (N + block_size - 1) / block_size;
    
    copyPositionsToVBOKernel<<<num_blocks, block_size>>>(
        d_vbo, d_particles->pos_x, d_particles->pos_y, d_particles->pos_z, N
    );
    CUDA_CHECK_KERNEL();
}

CudaGLInterop::CudaGLInterop()
    : position_vbo_(0), cuda_vbo_(nullptr),
      is_mapped_(false), is_initialized_(false), particle_count_(0) {}

CudaGLInterop::~CudaGLInterop() {
    cleanup();
}

void CudaGLInterop::initialize(size_t particle_count) {
    if (is_initialized_) {
        cleanup();
    }
    
    particle_count_ = particle_count;
    
    // Create OpenGL VBO
    glGenBuffers(1, &position_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo_);
    glBufferData(GL_ARRAY_BUFFER, particle_count * 3 * sizeof(float),
                 nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    checkGLError("CudaGLInterop::initialize - VBO creation");
    
    // Register VBO with CUDA
    CUDA_CHECK(cudaGraphicsGLRegisterBuffer(&cuda_vbo_, position_vbo_,
                                             cudaGraphicsMapFlagsWriteDiscard));
    
    is_initialized_ = true;
}

void CudaGLInterop::cleanup() {
    if (is_mapped_) {
        unmapPositionBuffer();
    }
    
    if (cuda_vbo_) {
        cudaGraphicsUnregisterResource(cuda_vbo_);
        cuda_vbo_ = nullptr;
    }
    
    if (position_vbo_) {
        glDeleteBuffers(1, &position_vbo_);
        position_vbo_ = 0;
    }
    
    is_initialized_ = false;
}

float* CudaGLInterop::mapPositionBuffer() {
    if (!is_initialized_ || is_mapped_) {
        return nullptr;
    }
    
    CUDA_CHECK(cudaGraphicsMapResources(1, &cuda_vbo_, 0));
    
    float* d_ptr;
    size_t size;
    CUDA_CHECK(cudaGraphicsResourceGetMappedPointer(
        reinterpret_cast<void**>(&d_ptr), &size, cuda_vbo_));
    
    is_mapped_ = true;
    return d_ptr;
}

void CudaGLInterop::unmapPositionBuffer() {
    if (!is_mapped_) return;
    
    CUDA_CHECK(cudaGraphicsUnmapResources(1, &cuda_vbo_, 0));
    is_mapped_ = false;
}

void CudaGLInterop::updatePositions(const ParticleData* d_particles) {
    float* d_vbo = mapPositionBuffer();
    if (d_vbo) {
        launchCopyPositionsToVBOKernel(d_vbo, d_particles, 256);
        unmapPositionBuffer();
    }
}

bool CudaGLInterop::verifyDataIntegrity(const float* expected_data, size_t count) {
    if (!is_initialized_ || count != particle_count_ * 3) {
        return false;
    }
    
    // Map and read back data
    float* d_vbo = mapPositionBuffer();
    if (!d_vbo) return false;
    
    std::vector<float> actual_data(count);
    CUDA_CHECK(cudaMemcpy(actual_data.data(), d_vbo, count * sizeof(float),
                          cudaMemcpyDeviceToHost));
    
    unmapPositionBuffer();
    
    // Compare
    for (size_t i = 0; i < count; i++) {
        if (std::abs(actual_data[i] - expected_data[i]) > 1e-5f) {
            return false;
        }
    }
    
    return true;
}

} // namespace nbody
