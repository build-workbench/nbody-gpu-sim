#include "nbody/error_handling.hpp"
#include "nbody/serialization.hpp"
#include "nbody/types.hpp"
#include <cmath>

#if defined(NBODY_WITH_RENDERING) && NBODY_WITH_RENDERING
#include <GL/glew.h>
#endif

namespace nbody {

void checkGLError(const char* operation) {
#if defined(NBODY_WITH_RENDERING) && NBODY_WITH_RENDERING
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    // Clear the entire error queue
    GLenum first_error = err;
    while (glGetError() != GL_NO_ERROR) {}
    throw OpenGLException(operation, first_error);
  }
#else
  (void)operation;
#endif
}

void validateResourceRequirements(size_t particle_count) {
#if defined(NBODY_WITH_CUDA) && NBODY_WITH_CUDA
  cudaDeviceProp prop;
  CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));

  // Estimate memory requirements
  // pos(3) + vel(3) + acc(6) + mass(1) = 13 floats per particle
  size_t required_memory = particle_count * sizeof(float) * 13;

  // Add overhead for acceleration structures
  required_memory *= 2;

  if (required_memory > prop.totalGlobalMem * 0.8) {
    throw ResourceException("Insufficient GPU memory", required_memory,
                            static_cast<size_t>(prop.totalGlobalMem * 0.8));
  }
#else
  (void)particle_count;
#endif
}

void validateSimulationConfig(const SimulationConfig& config) {
  validateParticleCountRange(config.particle_count);
  validateTimeStep(config.dt);
  validateSoftening(config.softening);

  if (config.force_method == ForceMethod::BARNES_HUT) {
    validateTheta(config.barnes_hut_theta);
  }

  if (config.G <= 0 || std::isnan(config.G) || std::isinf(config.G)) {
    throw ValidationException("Gravitational constant must be positive and finite");
  }

  if (config.force_method == ForceMethod::SPATIAL_HASH) {
    if (config.spatial_hash_cell_size <= 0 || std::isnan(config.spatial_hash_cell_size) ||
        std::isinf(config.spatial_hash_cell_size)) {
      throw ValidationException("Spatial hash cell size must be positive and finite");
    }

    if (config.spatial_hash_cutoff <= 0 || std::isnan(config.spatial_hash_cutoff) ||
        std::isinf(config.spatial_hash_cutoff)) {
      throw ValidationException("Spatial hash cutoff must be positive and finite");
    }

    // The 3x3x3 neighbor search only finds all pairs within cutoff when
    // cell_size >= cutoff; otherwise interactions are silently missed.
    if (config.spatial_hash_cutoff > config.spatial_hash_cell_size) {
      throw ValidationException("Spatial hash cutoff must not exceed cell size");
    }
  }

  if (config.cuda_block_size <= 0 || config.cuda_block_size > 1024) {
    throw ValidationException("CUDA block size must be between 1 and 1024");
  }

  // The shared-memory energy reductions assume a power-of-two block size;
  // with any other size the final partial sums are silently dropped.
  if ((config.cuda_block_size & (config.cuda_block_size - 1)) != 0) {
    throw ValidationException("CUDA block size must be a power of two");
  }
}

void validateParticleCountRange(size_t count) {
  if (count == 0) {
    throw ValidationException("Particle count must be greater than 0");
  }

  if (count > MAX_PARTICLE_COUNT) {
    throw ValidationException("Particle count exceeds maximum supported (100M)");
  }
}

void validateParticleCount(size_t count) {
  validateParticleCountRange(count);
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
  if (std::isnan(eps) || std::isinf(eps)) {
    throw ValidationException("Softening parameter must be a finite number");
  }

  // eps = 0 makes coincident particles evaluate rsqrtf(0) = inf, which turns
  // into NaN accelerations and poisons the whole simulation.
  if (eps <= 0) {
    throw ValidationException("Softening parameter must be positive");
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

}  // namespace nbody
