#pragma once

#include <cuda_runtime.h>
#include <stdexcept>
#include <string>
#include <sstream>

namespace nbody {

// CUDA Exception class
class CudaException : public std::runtime_error {
public:
    CudaException(const char* msg, const char* file, int line)
        : std::runtime_error(formatMessage(msg, file, line)),
          error_msg_(msg), file_(file), line_(line) {}
    
    const char* getErrorMsg() const { return error_msg_; }
    const char* getFile() const { return file_; }
    int getLine() const { return line_; }

private:
    static std::string formatMessage(const char* msg, const char* file, int line) {
        std::ostringstream oss;
        oss << "CUDA Error: " << msg << " at " << file << ":" << line;
        return oss.str();
    }
    
    const char* error_msg_;
    const char* file_;
    int line_;
};

// OpenGL Exception class
class OpenGLException : public std::runtime_error {
public:
    OpenGLException(const char* operation, unsigned int error_code)
        : std::runtime_error(formatMessage(operation, error_code)),
          operation_(operation), error_code_(error_code) {}
    
    const char* getOperation() const { return operation_; }
    unsigned int getErrorCode() const { return error_code_; }

private:
    static std::string formatMessage(const char* operation, unsigned int error_code) {
        std::ostringstream oss;
        oss << "OpenGL Error in " << operation << ": code " << error_code;
        return oss.str();
    }
    
    const char* operation_;
    unsigned int error_code_;
};

// Resource Exception class
class ResourceException : public std::runtime_error {
public:
    ResourceException(const char* msg, size_t required, size_t available)
        : std::runtime_error(formatMessage(msg, required, available)),
          required_(required), available_(available) {}
    
    size_t getRequired() const { return required_; }
    size_t getAvailable() const { return available_; }

private:
    static std::string formatMessage(const char* msg, size_t required, size_t available) {
        std::ostringstream oss;
        oss << msg << " - Required: " << required << " bytes, Available: " << available << " bytes";
        return oss.str();
    }
    
    size_t required_;
    size_t available_;
};

// Validation Exception class
class ValidationException : public std::runtime_error {
public:
    explicit ValidationException(const std::string& msg)
        : std::runtime_error("Validation Error: " + msg) {}
};

// CUDA error checking macro
#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            throw nbody::CudaException(cudaGetErrorString(err), __FILE__, __LINE__); \
        } \
    } while(0)

// CUDA kernel launch error check
#define CUDA_CHECK_KERNEL() \
    do { \
        cudaError_t err = cudaGetLastError(); \
        if (err != cudaSuccess) { \
            throw nbody::CudaException(cudaGetErrorString(err), __FILE__, __LINE__); \
        } \
        err = cudaDeviceSynchronize(); \
        if (err != cudaSuccess) { \
            throw nbody::CudaException(cudaGetErrorString(err), __FILE__, __LINE__); \
        } \
    } while(0)

// OpenGL error checking function
void checkGLError(const char* operation);

// Resource validation
void validateResourceRequirements(size_t particle_count);

// Input validation
void validateSimulationConfig(const struct SimulationConfig& config);
void validateParticleCount(size_t count);
void validateTimeStep(float dt);
void validateSoftening(float eps);
void validateTheta(float theta);

} // namespace nbody
