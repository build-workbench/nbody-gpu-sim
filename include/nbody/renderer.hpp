#pragma once

#include "nbody/types.hpp"
#include "nbody/camera.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

namespace nbody {

// OpenGL renderer for particle visualization
class Renderer {
public:
    Renderer();
    ~Renderer();
    
    // Initialize renderer with window dimensions
    void initialize(int width, int height);
    
    // Cleanup resources
    void cleanup();
    
    // Render particles from VBO
    void render(GLuint position_vbo, size_t particle_count);
    
    // Camera access
    void setCamera(const Camera& camera) { camera_ = camera; }
    Camera& getCamera() { return camera_; }
    const Camera& getCamera() const { return camera_; }
    
    // Rendering settings
    void setColorMode(ColorMode mode) { color_mode_ = mode; }
    void setPointSize(float size) { point_size_ = size; }
    void setMaxDepth(float depth) { max_depth_ = depth; }
    void setMaxVelocity(float velocity) { max_velocity_ = velocity; }
    
    ColorMode getColorMode() const { return color_mode_; }
    float getPointSize() const { return point_size_; }
    
    // Window resize handling
    void onResize(int width, int height);
    
    // Get shader program for testing
    GLuint getShaderProgram() const { return shader_program_; }
    
private:
    // OpenGL objects
    GLuint shader_program_;
    GLuint vao_;
    
    // Uniform locations
    GLint view_loc_;
    GLint projection_loc_;
    GLint point_size_loc_;
    GLint max_depth_loc_;
    GLint max_velocity_loc_;
    GLint color_mode_loc_;
    
    // Rendering state
    Camera camera_;
    ColorMode color_mode_;
    float point_size_;
    float max_depth_;
    float max_velocity_;
    int width_;
    int height_;
    bool is_initialized_;
    
    // Shader compilation
    void compileShaders();
    GLuint compileShader(GLenum type, const char* source);
    void checkShaderCompilation(GLuint shader);
    void checkProgramLinking(GLuint program);
};

// Shader source code
extern const char* VERTEX_SHADER_SOURCE;
extern const char* FRAGMENT_SHADER_SOURCE;

} // namespace nbody
