// Color mapping tests are included in test_camera.cpp
// This file is kept for organizational purposes

#include <gtest/gtest.h>
#include "nbody/camera.hpp"

using namespace nbody;

// Additional color mapping tests

TEST(ColorMappingTest, DepthToColor) {
    glm::vec3 near_color = ColorMapper::depthToColor(0.0f, 100.0f);
    glm::vec3 far_color = ColorMapper::depthToColor(100.0f, 100.0f);
    
    // Near should be warm (orange), far should be cool (blue)
    EXPECT_GT(near_color.r, near_color.b);
    EXPECT_GT(far_color.b, far_color.r);
}

TEST(ColorMappingTest, DensityToColor) {
    glm::vec3 low_density = ColorMapper::densityToColor(0.0f, 100.0f);
    glm::vec3 high_density = ColorMapper::densityToColor(100.0f, 100.0f);
    
    // Colors should be different
    EXPECT_NE(low_density, high_density);
    
    // Both should be valid
    EXPECT_GE(low_density.r, 0.0f); EXPECT_LE(low_density.r, 1.0f);
    EXPECT_GE(high_density.r, 0.0f); EXPECT_LE(high_density.r, 1.0f);
}

TEST(ColorMappingTest, GradientMap) {
    glm::vec3 start(1.0f, 0.0f, 0.0f);  // Red
    glm::vec3 end(0.0f, 0.0f, 1.0f);    // Blue
    
    glm::vec3 mid = ColorMapper::gradientMap(0.5f, start, end);
    
    // Mid should be purple-ish
    EXPECT_NEAR(mid.r, 0.5f, 0.01f);
    EXPECT_NEAR(mid.b, 0.5f, 0.01f);
}
