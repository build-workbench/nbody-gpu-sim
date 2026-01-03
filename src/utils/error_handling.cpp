#include "nbody/error_handling.hpp"
#include "nbody/types.hpp"
#include <GL/glew.h>
#include <cmath>

namespace nbody {

void checkGLError(const char* operation) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        throw OpenGLException(operation, err);
    }
}

void validateResourceRequirements(size_t particle_count) {
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);
    
    // Estimate memory requirements
    // pos(3) + vel(3) + acc(6) + mass(1) = 13 floats per particle
    size_t required_memory = particle_count * sizeof(float) * 13;
    
    // Add overhead for acceleration structures
    required_memory *= 2;
    
    if (required_memory > prop.totalGlobalMem * 0.8) {
        throw ResourceException("Insufficient GPU memory", required_memory,
                               static_cast<size_t>(prop.totalGlobalMem * 0.8));
    }
}

void validateSimulationConfig(const SimulationConfig& config) {
    validateParticleCount(config.particle_count);
    validateTimeStep(config.dt);
    validateSoftening(config.softening);
    
    if (config.force_method == ForceMethod::BARNES_HUT) {
        validateTheta(config.barnes_hut_theta);
    }
    
    if (config.G <= 0) {
        throw ValidationException("Gravitational constant must be positive");
    }
    
    if (config.cuda_block_size <= 0 || config.cuda_block_size > 1024) {
        throw ValidationException("CUDA block size must be between 1 and 1024");
    }
}

void validateParticleCount(size_t count) {
    if (count == 0) {
        throw ValidationException("Particle count must be greater than 0");
    }
    
    if (count > 100000000) {  // 100 million
        throw ValidationException("Particle count exceeds maximum supported (100M)");
    }
    
    validateResourceRequirements(count);
}

void validateTimeStep(float dt) {
    if (dt <= 0) {
        throw ValidationException("Time step must be positive");
    }
    
    if (std::isnan(dt) || std::isinf(dt)) {
        throw ValidationException("Time step must be a finite number");
    }
    
    if (dt > 1.0f) {
        throw ValidationException("Time step is too large (max 1.0)");
    }
}

void validateSoftening(float eps) {
    if (eps < 0) {
        throw ValidationException("Softening parameter must be non-negative");
    }
    
    if (std::isnan(eps) || std::isinf(eps)) {
        throw ValidationException("Softening parameter must be a finite number");
    }
}

void validateTheta(float theta) {
    if (theta < 0 || theta > 2.0f) {
        throw ValidationException("Barnes-Hut theta must be between 0 and 2");
    }
    
    if (std::isnan(theta) || std::isinf(theta)) {
        throw ValidationException("Barnes-Hut theta must be a finite number");
    }
}

} // namespace nbody
