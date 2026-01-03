#include "nbody/renderer.hpp"
#include "nbody/error_handling.hpp"
#include <iostream>

namespace nbody {

// Vertex shader source
const char* VERTEX_SHADER_SOURCE = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform float pointSize;

out float depth;

void main() {
    vec4 viewPos = view * vec4(aPos, 1.0);
    gl_Position = projection * viewPos;
    gl_PointSize = pointSize / max(-viewPos.z, 0.1);
    depth = -viewPos.z;
}
)";

// Fragment shader source
const char* FRAGMENT_SHADER_SOURCE = R"(
#version 330 core
in float depth;
out vec4 FragColor;

uniform float maxDepth;
uniform float maxVelocity;
uniform int colorMode;

void main() {
    // Circular point sprite
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    if (dist > 0.5) discard;
    
    // Color based on mode
    vec3 color;
    float t = clamp(depth / maxDepth, 0.0, 1.0);
    
    if (colorMode == 0) {
        // Depth-based coloring (warm to cool)
        color = mix(vec3(1.0, 0.5, 0.0), vec3(0.0, 0.5, 1.0), t);
    } else if (colorMode == 1) {
        // Velocity-based (would need velocity data)
        color = mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0), t);
    } else {
        // Density-based
        color = mix(vec3(0.2, 0.2, 0.8), vec3(1.0, 1.0, 0.2), t);
    }
    
    // Edge softening
    float alpha = 1.0 - smoothstep(0.4, 0.5, dist);
    
    FragColor = vec4(color, alpha);
}
)";

Renderer::Renderer()
    : shader_program_(0), vao_(0),
      color_mode_(ColorMode::DEPTH), point_size_(2.0f),
      max_depth_(100.0f), max_velocity_(10.0f),
      width_(1280), height_(720), is_initialized_(false) {}

Renderer::~Renderer() {
    cleanup();
}

void Renderer::initialize(int width, int height) {
    width_ = width;
    height_ = height;
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        throw OpenGLException("glewInit", 0);
    }
    
    // Compile shaders
    compileShaders();
    
    // Create VAO
    glGenVertexArrays(1, &vao_);
    
    // Enable point sprites
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set up camera
    camera_.setPosition(glm::vec3(0, 0, 50));
    camera_.setTarget(glm::vec3(0, 0, 0));
    camera_.setAspectRatio(static_cast<float>(width) / height);
    
    is_initialized_ = true;
}

void Renderer::cleanup() {
    if (shader_program_) {
        glDeleteProgram(shader_program_);
        shader_program_ = 0;
    }
    if (vao_) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    is_initialized_ = false;
}

void Renderer::compileShaders() {
    GLuint vertex_shader = compileShader(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE);
    GLuint fragment_shader = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);
    
    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertex_shader);
    glAttachShader(shader_program_, fragment_shader);
    glLinkProgram(shader_program_);
    checkProgramLinking(shader_program_);
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    // Get uniform locations
    view_loc_ = glGetUniformLocation(shader_program_, "view");
    projection_loc_ = glGetUniformLocation(shader_program_, "projection");
    point_size_loc_ = glGetUniformLocation(shader_program_, "pointSize");
    max_depth_loc_ = glGetUniformLocation(shader_program_, "maxDepth");
    max_velocity_loc_ = glGetUniformLocation(shader_program_, "maxVelocity");
    color_mode_loc_ = glGetUniformLocation(shader_program_, "colorMode");
}

GLuint Renderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    checkShaderCompilation(shader);
    return shader;
}

void Renderer::checkShaderCompilation(GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        throw OpenGLException(info_log, 0);
    }
}

void Renderer::checkProgramLinking(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, nullptr, info_log);
        throw OpenGLException(info_log, 0);
    }
}

void Renderer::render(GLuint position_vbo, size_t particle_count) {
    if (!is_initialized_) return;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
    
    glUseProgram(shader_program_);
    
    // Set uniforms
    glUniformMatrix4fv(view_loc_, 1, GL_FALSE, &camera_.getViewMatrix()[0][0]);
    glUniformMatrix4fv(projection_loc_, 1, GL_FALSE, &camera_.getProjectionMatrix()[0][0]);
    glUniform1f(point_size_loc_, point_size_);
    glUniform1f(max_depth_loc_, max_depth_);
    glUniform1f(max_velocity_loc_, max_velocity_);
    glUniform1i(color_mode_loc_, static_cast<int>(color_mode_));
    
    // Bind VAO and VBO
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    
    // Draw particles
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(particle_count));
    
    glBindVertexArray(0);
}

void Renderer::onResize(int width, int height) {
    width_ = width;
    height_ = height;
    glViewport(0, 0, width, height);
    camera_.setAspectRatio(static_cast<float>(width) / height);
}

} // namespace nbody
