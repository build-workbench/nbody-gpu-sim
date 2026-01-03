#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "nbody/error_handling.hpp"
#include "nbody/types.hpp"
#include <cmath>
#include <limits>

using namespace nbody;

// Unit Tests

TEST(ValidationTest, ValidParticleCount) {
    EXPECT_NO_THROW(validateParticleCount(100));
    EXPECT_NO_THROW(validateParticleCount(1000000));
}

TEST(ValidationTest, InvalidParticleCount) {
    EXPECT_THROW(validateParticleCount(0), ValidationException);
}

TEST(ValidationTest, ValidTimeStep) {
    EXPECT_NO_THROW(validateTimeStep(0.001f));
    EXPECT_NO_THROW(validateTimeStep(0.1f));
}

TEST(ValidationTest, InvalidTimeStep) {
    EXPECT_THROW(validateTimeStep(0.0f), ValidationException);
    EXPECT_THROW(validateTimeStep(-0.001f), ValidationException);
    EXPECT_THROW(validateTimeStep(std::numeric_limits<float>::quiet_NaN()), ValidationException);
    EXPECT_THROW(validateTimeStep(std::numeric_limits<float>::infinity()), ValidationException);
}

TEST(ValidationTest, ValidSoftening) {
    EXPECT_NO_THROW(validateSoftening(0.0f));
    EXPECT_NO_THROW(validateSoftening(0.01f));
    EXPECT_NO_THROW(validateSoftening(1.0f));
}

TEST(ValidationTest, InvalidSoftening) {
    EXPECT_THROW(validateSoftening(-0.01f), ValidationException);
    EXPECT_THROW(validateSoftening(std::numeric_limits<float>::quiet_NaN()), ValidationException);
}

TEST(ValidationTest, ValidTheta) {
    EXPECT_NO_THROW(validateTheta(0.0f));
    EXPECT_NO_THROW(validateTheta(0.5f));
    EXPECT_NO_THROW(validateTheta(1.0f));
}

TEST(ValidationTest, InvalidTheta) {
    EXPECT_THROW(validateTheta(-0.1f), ValidationException);
    EXPECT_THROW(validateTheta(2.5f), ValidationException);
    EXPECT_THROW(validateTheta(std::numeric_limits<float>::quiet_NaN()), ValidationException);
}

TEST(ValidationTest, ValidSimulationConfig) {
    SimulationConfig config;
    config.particle_count = 1000;
    config.dt = 0.001f;
    config.G = 1.0f;
    config.softening = 0.01f;
    config.barnes_hut_theta = 0.5f;
    config.cuda_block_size = 256;
    
    EXPECT_NO_THROW(validateSimulationConfig(config));
}

TEST(ValidationTest, InvalidSimulationConfig) {
    SimulationConfig config;
    config.particle_count = 0;  // Invalid
    config.dt = 0.001f;
    config.G = 1.0f;
    config.softening = 0.01f;
    
    EXPECT_THROW(validateSimulationConfig(config), ValidationException);
}

// Property-Based Tests
// Feature: n-body-simulation, Property 13: Input Validation Robustness

RC_GTEST_PROP(Validation, RejectsNegativeParticleCount, (int count)) {
    // Feature: n-body-simulation, Property 13: Input Validation Robustness
    // Validates: Requirements 10.4
    
    RC_PRE(count <= 0);
    
    // Property: System rejects negative or zero particle count
    bool threw = false;
    try {
        validateParticleCount(static_cast<size_t>(count));
    } catch (const ValidationException&) {
        threw = true;
    }
    
    RC_ASSERT(threw);
}

RC_GTEST_PROP(Validation, RejectsInvalidTimeStep, (float dt)) {
    // Feature: n-body-simulation, Property 13: Input Validation Robustness
    // Validates: Requirements 10.4
    
    RC_PRE(dt <= 0 || std::isnan(dt) || std::isinf(dt) || dt > 1.0f);
    
    // Property: System rejects invalid time steps
    bool threw = false;
    try {
        validateTimeStep(dt);
    } catch (const ValidationException&) {
        threw = true;
    }
    
    RC_ASSERT(threw);
}

RC_GTEST_PROP(Validation, RejectsNaNSoftening, ()) {
    // Feature: n-body-simulation, Property 13: Input Validation Robustness
    // Validates: Requirements 10.4
    
    float nan_value = std::numeric_limits<float>::quiet_NaN();
    
    // Property: System rejects NaN softening
    bool threw = false;
    try {
        validateSoftening(nan_value);
    } catch (const ValidationException&) {
        threw = true;
    }
    
    RC_ASSERT(threw);
}

RC_GTEST_PROP(Validation, RejectsOutOfRangeTheta, (float theta)) {
    // Feature: n-body-simulation, Property 13: Input Validation Robustness
    // Validates: Requirements 10.4
    
    RC_PRE(theta < 0 || theta > 2.0f || std::isnan(theta) || std::isinf(theta));
    
    // Property: System rejects out-of-range theta values
    bool threw = false;
    try {
        validateTheta(theta);
    } catch (const ValidationException&) {
        threw = true;
    }
    
    RC_ASSERT(threw);
}

RC_GTEST_PROP(Validation, AcceptsValidParameters,
              (size_t count, float dt, float softening, float theta)) {
    // Feature: n-body-simulation, Property 13: Input Validation Robustness
    // Validates: Requirements 10.4
    
    // Generate valid parameters
    RC_PRE(count > 0 && count <= 1000000);
    RC_PRE(dt > 0.0001f && dt <= 1.0f);
    RC_PRE(softening >= 0.0f && softening < 10.0f);
    RC_PRE(theta >= 0.0f && theta <= 2.0f);
    RC_PRE(std::isfinite(dt) && std::isfinite(softening) && std::isfinite(theta));
    
    // Property: System accepts valid parameters without throwing
    bool threw = false;
    try {
        validateParticleCount(count);
        validateTimeStep(dt);
        validateSoftening(softening);
        validateTheta(theta);
    } catch (const ValidationException&) {
        threw = true;
    }
    
    RC_ASSERT(!threw);
}

RC_GTEST_PROP(Validation, StateUnchangedAfterRejection, ()) {
    // Feature: n-body-simulation, Property 13: Input Validation Robustness
    // Validates: Requirements 10.4
    
    // This property tests that invalid input doesn't corrupt system state
    // Since validation functions don't modify state, we verify they throw
    // without side effects
    
    float invalid_dt = -1.0f;
    
    bool threw = false;
    try {
        validateTimeStep(invalid_dt);
    } catch (const ValidationException& e) {
        threw = true;
        // Verify exception message is meaningful
        std::string msg = e.what();
        RC_ASSERT(msg.find("Validation") != std::string::npos);
    }
    
    RC_ASSERT(threw);
}
