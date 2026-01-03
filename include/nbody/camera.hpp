#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace nbody {

// Camera controller for 3D navigation
class Camera {
public:
    Camera(float fov = 45.0f, float aspect = 16.0f/9.0f, float near = 0.1f, float far = 1000.0f);
    
    // Get transformation matrices
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewProjectionMatrix() const;
    
    // Camera positioning
    void setPosition(const glm::vec3& pos) { position_ = pos; updateViewMatrix(); }
    void setTarget(const glm::vec3& target) { target_ = target; updateViewMatrix(); }
    void setUp(const glm::vec3& up) { up_ = up; updateViewMatrix(); }
    
    glm::vec3 getPosition() const { return position_; }
    glm::vec3 getTarget() const { return target_; }
    glm::vec3 getUp() const { return up_; }
    glm::vec3 getForward() const { return glm::normalize(target_ - position_); }
    glm::vec3 getRight() const { return glm::normalize(glm::cross(getForward(), up_)); }
    
    // Projection settings
    void setFOV(float fov) { fov_ = fov; updateProjectionMatrix(); }
    void setAspectRatio(float aspect) { aspect_ = aspect; updateProjectionMatrix(); }
    void setNearFar(float near, float far) { near_ = near; far_ = far; updateProjectionMatrix(); }
    
    float getFOV() const { return fov_; }
    float getAspectRatio() const { return aspect_; }
    float getNear() const { return near_; }
    float getFar() const { return far_; }
    
    // Camera controls
    void rotate(float yaw, float pitch);  // Rotate around target
    void pan(float dx, float dy);         // Pan in view plane
    void zoom(float delta);               // Move toward/away from target
    void orbit(float yaw, float pitch);   // Orbit around target
    
    // Reset to default position
    void reset();
    
    // Set orbit distance
    void setOrbitDistance(float distance);
    float getOrbitDistance() const;
    
private:
    // Camera state
    glm::vec3 position_;
    glm::vec3 target_;
    glm::vec3 up_;
    
    // Projection parameters
    float fov_;
    float aspect_;
    float near_;
    float far_;
    
    // Cached matrices
    mutable glm::mat4 view_matrix_;
    mutable glm::mat4 projection_matrix_;
    mutable bool view_dirty_;
    mutable bool projection_dirty_;
    
    void updateViewMatrix() const;
    void updateProjectionMatrix() const;
};

// Color mapping utilities
struct ColorMapper {
    // Map velocity magnitude to color
    static glm::vec3 velocityToColor(float velocity, float max_velocity);
    
    // Map depth to color
    static glm::vec3 depthToColor(float depth, float max_depth);
    
    // Map density to color
    static glm::vec3 densityToColor(float density, float max_density);
    
    // Generic gradient mapping
    static glm::vec3 gradientMap(float t, const glm::vec3& color_low, const glm::vec3& color_high);
};

} // namespace nbody
