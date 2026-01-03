#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "nbody/camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

using namespace nbody;

// Unit Tests

TEST(CameraTest, DefaultViewMatrix) {
    Camera camera;
    camera.setPosition(glm::vec3(0, 0, 10));
    camera.setTarget(glm::vec3(0, 0, 0));
    camera.setUp(glm::vec3(0, 1, 0));
    
    glm::mat4 view = camera.getViewMatrix();
    
    // Transform origin should be at (0, 0, -10) in view space
    glm::vec4 origin = view * glm::vec4(0, 0, 0, 1);
    EXPECT_NEAR(origin.x, 0, 1e-5);
    EXPECT_NEAR(origin.y, 0, 1e-5);
    EXPECT_NEAR(origin.z, -10, 1e-5);
}

TEST(CameraTest, ProjectionMatrix) {
    Camera camera(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
    
    glm::mat4 proj = camera.getProjectionMatrix();
    
    // Projection matrix should be valid
    EXPECT_NE(proj[0][0], 0);
    EXPECT_NE(proj[1][1], 0);
}

TEST(CameraTest, ZoomChangesDistance) {
    Camera camera;
    camera.setPosition(glm::vec3(0, 0, 50));
    camera.setTarget(glm::vec3(0, 0, 0));
    
    float initial_distance = camera.getOrbitDistance();
    
    camera.zoom(10.0f);
    
    float new_distance = camera.getOrbitDistance();
    EXPECT_LT(new_distance, initial_distance);
}

TEST(CameraTest, OrbitMaintainsDistance) {
    Camera camera;
    camera.setPosition(glm::vec3(0, 0, 50));
    camera.setTarget(glm::vec3(0, 0, 0));
    
    float initial_distance = camera.getOrbitDistance();
    
    camera.orbit(0.5f, 0.3f);
    
    float new_distance = camera.getOrbitDistance();
    EXPECT_NEAR(new_distance, initial_distance, 0.01f);
}

TEST(ColorMapperTest, VelocityToColor) {
    glm::vec3 color_low = ColorMapper::velocityToColor(0.0f, 10.0f);
    glm::vec3 color_high = ColorMapper::velocityToColor(10.0f, 10.0f);
    
    // Colors should be in valid range
    EXPECT_GE(color_low.r, 0.0f); EXPECT_LE(color_low.r, 1.0f);
    EXPECT_GE(color_low.g, 0.0f); EXPECT_LE(color_low.g, 1.0f);
    EXPECT_GE(color_low.b, 0.0f); EXPECT_LE(color_low.b, 1.0f);
    
    EXPECT_GE(color_high.r, 0.0f); EXPECT_LE(color_high.r, 1.0f);
    EXPECT_GE(color_high.g, 0.0f); EXPECT_LE(color_high.g, 1.0f);
    EXPECT_GE(color_high.b, 0.0f); EXPECT_LE(color_high.b, 1.0f);
    
    // Colors should be different
    EXPECT_NE(color_low, color_high);
}

// Property-Based Tests
// Feature: n-body-simulation, Property 9: Camera Transformation Correctness

RC_GTEST_PROP(Camera, ViewMatrixTransformsCorrectly,
              (float px, float py, float pz, float tx, float ty, float tz)) {
    // Feature: n-body-simulation, Property 9: Camera Transformation Correctness
    // Validates: Requirements 7.2
    
    RC_PRE(std::abs(px) < 100 && std::abs(py) < 100 && std::abs(pz) < 100);
    RC_PRE(std::abs(tx) < 100 && std::abs(ty) < 100 && std::abs(tz) < 100);
    
    glm::vec3 pos(px, py, pz);
    glm::vec3 target(tx, ty, tz);
    
    // Ensure camera is not at target
    float dist = glm::length(pos - target);
    RC_PRE(dist > 0.1f);
    
    Camera camera;
    camera.setPosition(pos);
    camera.setTarget(target);
    camera.setUp(glm::vec3(0, 1, 0));
    
    glm::mat4 view = camera.getViewMatrix();
    
    // Property: Target should be at origin in view space (along -Z)
    glm::vec4 target_view = view * glm::vec4(target, 1.0f);
    RC_ASSERT(std::abs(target_view.x) < 0.01f);
    RC_ASSERT(std::abs(target_view.y) < 0.01f);
    RC_ASSERT(target_view.z < 0);  // In front of camera
}

RC_GTEST_PROP(Camera, ProjectionPreservesRelativePositions,
              (float x1, float y1, float z1, float x2, float y2, float z2)) {
    // Feature: n-body-simulation, Property 9: Camera Transformation Correctness
    // Validates: Requirements 7.2
    
    RC_PRE(std::abs(x1) < 50 && std::abs(y1) < 50 && std::abs(z1) < 50);
    RC_PRE(std::abs(x2) < 50 && std::abs(y2) < 50 && std::abs(z2) < 50);
    RC_PRE(z1 < -1.0f && z2 < -1.0f);  // Both in front of camera
    
    Camera camera;
    camera.setPosition(glm::vec3(0, 0, 0));
    camera.setTarget(glm::vec3(0, 0, -1));
    
    glm::mat4 mvp = camera.getViewProjectionMatrix();
    
    glm::vec4 p1 = mvp * glm::vec4(x1, y1, z1, 1.0f);
    glm::vec4 p2 = mvp * glm::vec4(x2, y2, z2, 1.0f);
    
    // Perspective divide
    if (std::abs(p1.w) > 0.001f && std::abs(p2.w) > 0.001f) {
        p1 /= p1.w;
        p2 /= p2.w;
        
        // Property: Closer objects (larger z, less negative) should have larger w
        // and appear larger after projection
        if (z1 > z2) {  // p1 is closer
            // Closer objects should have smaller depth value
            RC_ASSERT(p1.z < p2.z);
        }
    }
}

// Feature: n-body-simulation, Property 10: Color Mapping Correctness

RC_GTEST_PROP(ColorMapper, ValidRGBRange, (float velocity, float max_velocity)) {
    // Feature: n-body-simulation, Property 10: Color Mapping Correctness
    // Validates: Requirements 7.4
    
    RC_PRE(max_velocity > 0.0f && max_velocity < 1000.0f);
    RC_PRE(velocity >= 0.0f && velocity <= max_velocity * 2.0f);  // Allow out of range
    RC_PRE(std::isfinite(velocity) && std::isfinite(max_velocity));
    
    glm::vec3 color = ColorMapper::velocityToColor(velocity, max_velocity);
    
    // Property: Color values should be in valid RGB range [0, 1]
    RC_ASSERT(color.r >= 0.0f && color.r <= 1.0f);
    RC_ASSERT(color.g >= 0.0f && color.g <= 1.0f);
    RC_ASSERT(color.b >= 0.0f && color.b <= 1.0f);
}

RC_GTEST_PROP(ColorMapper, MonotonicMapping, (float v1, float v2, float max_velocity)) {
    // Feature: n-body-simulation, Property 10: Color Mapping Correctness
    // Validates: Requirements 7.4
    
    RC_PRE(max_velocity > 0.0f && max_velocity < 1000.0f);
    RC_PRE(v1 >= 0.0f && v1 < max_velocity);
    RC_PRE(v2 >= 0.0f && v2 < max_velocity);
    RC_PRE(v1 != v2);
    RC_PRE(std::isfinite(v1) && std::isfinite(v2) && std::isfinite(max_velocity));
    
    glm::vec3 color1 = ColorMapper::velocityToColor(v1, max_velocity);
    glm::vec3 color2 = ColorMapper::velocityToColor(v2, max_velocity);
    
    // Property: Different velocities should produce different colors
    // (monotonic mapping means the gradient changes)
    if (std::abs(v1 - v2) > 0.01f * max_velocity) {
        RC_ASSERT(color1 != color2);
    }
}

RC_GTEST_PROP(ColorMapper, BoundaryColors, (float max_velocity)) {
    // Feature: n-body-simulation, Property 10: Color Mapping Correctness
    // Validates: Requirements 7.4
    
    RC_PRE(max_velocity > 0.0f && max_velocity < 1000.0f);
    RC_PRE(std::isfinite(max_velocity));
    
    glm::vec3 color_zero = ColorMapper::velocityToColor(0.0f, max_velocity);
    glm::vec3 color_max = ColorMapper::velocityToColor(max_velocity, max_velocity);
    
    // Property: Edge cases produce expected boundary colors
    // For velocity mapping: blue (low) to red (high)
    RC_ASSERT(color_zero.b > color_zero.r);  // More blue at low velocity
    RC_ASSERT(color_max.r > color_max.b);    // More red at high velocity
}
