cmake_minimum_required(VERSION 3.18)
project(NBodySimulation
    VERSION 2.0.0
    DESCRIPTION "GPU-accelerated N-Body particle simulation with real-time visualization"
    LANGUAGES CXX CUDA
)

# ── C++ / CUDA 标准 ──────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── CUDA 配置 ────────────────────────────────────────────────────
find_package(CUDAToolkit REQUIRED)

if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
        set(CMAKE_CUDA_ARCHITECTURES native)
    else()
        set(CMAKE_CUDA_ARCHITECTURES 75 80 86 89)
    endif()
endif()

# ── 第三方依赖 ───────────────────────────────────────────────────
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

# ── 构建选项 ─────────────────────────────────────────────────────
option(NBODY_BUILD_TESTS "Build unit tests" ON)

include(FetchContent)

if(NBODY_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

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

# ── 主库 ─────────────────────────────────────────────────────────
add_library(nbody_lib STATIC ${CUDA_SOURCES} ${CPP_SOURCES})
add_library(nbody::nbody_lib ALIAS nbody_lib)

target_include_directories(nbody_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(nbody_lib PUBLIC
    CUDA::cudart
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
)

set_target_properties(nbody_lib PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
)

target_compile_options(nbody_lib
    PRIVATE
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:GNU,Clang>>:-Wall -Wextra>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/W4>
)

# ── 主程序 ───────────────────────────────────────────────────────
add_executable(nbody_sim src/main.cpp)
target_link_libraries(nbody_sim PRIVATE nbody_lib)

# ── 测试 ─────────────────────────────────────────────────────────
if(NBODY_BUILD_TESTS)
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
    )
    include(GoogleTest)
    gtest_discover_tests(nbody_tests)
endif()

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
