cmake_minimum_required(VERSION 3.18)
project(NBodySimulation LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# CUDA settings
find_package(CUDAToolkit REQUIRED)
set(CMAKE_CUDA_ARCHITECTURES 75 80 86)

# OpenGL and GLFW
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# GLEW for OpenGL extensions
find_package(GLEW REQUIRED)

# GLM for math
find_package(glm REQUIRED)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# RapidCheck for property-based testing
FetchContent_Declare(
    rapidcheck
    GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
    GIT_TAG master
)
FetchContent_MakeAvailable(rapidcheck)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(CUDA_SOURCES
    src/cuda/force_direct.cu
    src/cuda/force_barnes_hut.cu
    src/cuda/force_spatial_hash.cu
    src/cuda/integrator.cu
    src/cuda/particle_init.cu
)

set(CPP_SOURCES
    src/core/particle_system.cpp
    src/core/force_calculator.cpp
    src/core/barnes_hut_tree.cpp
    src/core/spatial_hash_grid.cpp
    src/render/renderer.cpp
    src/render/camera.cpp
    src/render/cuda_gl_interop.cpp
    src/utils/error_handling.cpp
    src/utils/serialization.cpp
)

# Main library
add_library(nbody_lib STATIC ${CUDA_SOURCES} ${CPP_SOURCES})
target_link_libraries(nbody_lib PUBLIC
    CUDA::cudart
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
)
target_include_directories(nbody_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(nbody_lib PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

# Main executable
add_executable(nbody_sim src/main.cpp)
target_link_libraries(nbody_sim PRIVATE nbody_lib)

# Tests
enable_testing()
add_executable(nbody_tests
    tests/test_main.cpp
    tests/test_particle_data.cpp
    tests/test_force_calculation.cpp
    tests/test_integrator.cpp
    tests/test_barnes_hut.cpp
    tests/test_spatial_hash.cpp
    tests/test_serialization.cpp
    tests/test_camera.cpp
    tests/test_color_mapping.cpp
    tests/test_validation.cpp
)
target_link_libraries(nbody_tests PRIVATE
    nbody_lib
    GTest::gtest
    GTest::gtest_main
    rapidcheck
)
include(GoogleTest)
gtest_discover_tests(nbody_tests)

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
