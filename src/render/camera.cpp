#include "nbody/camera.hpp"
#include <cmath>
#include <algorithm>

namespace nbody {

Camera::Camera(float fov, float aspect, float near, float far)
    : position_(0, 0, 50), target_(0, 0, 0), up_(0, 1, 0),
      fov_(fov), aspect_(aspect), near_(near), far_(far),
      view_dirty_(true), projection_dirty_(true) {}

glm::mat4 Camera::getViewMatrix() const {
    if (view_dirty_) {
        updateViewMatrix();
    }
    return view_matrix_;
}

glm::mat4 Camera::getProjectionMatrix() const {
    if (projection_dirty_) {
        updateProjectionMatrix();
    }
    return projection_matrix_;
}

glm::mat4 Camera::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

void Camera::updateViewMatrix() const {
    view_matrix_ = glm::lookAt(position_, target_, up_);
    view_dirty_ = false;
}

void Camera::updateProjectionMatrix() const {
    projection_matrix_ = glm::perspective(glm::radians(fov_), aspect_, near_, far_);
    projection_dirty_ = false;
}

void Camera::rotate(float yaw, float pitch) {
    glm::vec3 direction = position_ - target_;
    float distance = glm::length(direction);
    
    // Convert to spherical coordinates
    float theta = std::atan2(direction.x, direction.z);
    float phi = std::acos(direction.y / distance);
    
    // Apply rotation
    theta += yaw;
    phi = std::clamp(phi + pitch, 0.1f, 3.04f);  // Clamp to avoid gimbal lock
    
    // Convert back to Cartesian
    direction.x = distance * std::sin(phi) * std::sin(theta);
    direction.y = distance * std::cos(phi);
    direction.z = distance * std::sin(phi) * std::cos(theta);
    
    position_ = target_ + direction;
    view_dirty_ = true;
}

void Camera::pan(float dx, float dy) {
    glm::vec3 right = getRight();
    glm::vec3 up = glm::normalize(glm::cross(right, getForward()));
    
    glm::vec3 offset = right * dx + up * dy;
    position_ += offset;
    target_ += offset;
    view_dirty_ = true;
}

void Camera::zoom(float delta) {
    glm::vec3 direction = target_ - position_;
    float distance = glm::length(direction);
    
    // Prevent getting too close or too far
    float new_distance = std::clamp(distance - delta, 1.0f, 1000.0f);
    
    position_ = target_ - glm::normalize(direction) * new_distance;
    view_dirty_ = true;
}

void Camera::orbit(float yaw, float pitch) {
    rotate(yaw, pitch);
}

void Camera::reset() {
    position_ = glm::vec3(0, 0, 50);
    target_ = glm::vec3(0, 0, 0);
    up_ = glm::vec3(0, 1, 0);
    view_dirty_ = true;
}

void Camera::setOrbitDistance(float distance) {
    glm::vec3 direction = glm::normalize(position_ - target_);
    position_ = target_ + direction * distance;
    view_dirty_ = true;
}

float Camera::getOrbitDistance() const {
    return glm::length(position_ - target_);
}

// ColorMapper implementation
glm::vec3 ColorMapper::velocityToColor(float velocity, float max_velocity) {
    float t = std::clamp(velocity / max_velocity, 0.0f, 1.0f);
    return gradientMap(t, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 ColorMapper::depthToColor(float depth, float max_depth) {
    float t = std::clamp(depth / max_depth, 0.0f, 1.0f);
    return gradientMap(t, glm::vec3(1.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 1.0f));
}

glm::vec3 ColorMapper::densityToColor(float density, float max_density) {
    float t = std::clamp(density / max_density, 0.0f, 1.0f);
    return gradientMap(t, glm::vec3(0.2f, 0.2f, 0.8f), glm::vec3(1.0f, 1.0f, 0.2f));
}

glm::vec3 ColorMapper::gradientMap(float t, const glm::vec3& color_low, const glm::vec3& color_high) {
    return glm::mix(color_low, color_high, t);
}

} // namespace nbody
