cmake_minimum_required(VERSION 3.18)

# ── 构建选项 (在 project() 之前) ─────────────────────────────────
option(NBODY_ENABLE_CUDA "Enable CUDA support" ON)
option(NBODY_ENABLE_RENDERING "Enable OpenGL/GLFW visualization surfaces" ON)
option(NBODY_ENABLE_UI "Enable Dear ImGui diagnostics panel (requires rendering)" ON)
option(NBODY_ENABLE_PROFILING "Enable built-in profiling instrumentation" OFF)
option(NBODY_ENABLE_HDF5 "Enable HDF5 for scientific data export/import" ON)

if(NBODY_ENABLE_CUDA)
    project(NBodySimulation
        VERSION 2.1.0
        DESCRIPTION "GPU-accelerated N-Body particle simulation with real-time visualization"
        LANGUAGES CXX CUDA
    )
else()
    project(NBodySimulation
        VERSION 2.1.0
        DESCRIPTION "N-Body particle simulation with real-time visualization"
        LANGUAGES CXX
    )
endif()

# ── C++ / CUDA 标准 ──────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── CUDA 配置 ────────────────────────────────────────────────────
if(NBODY_ENABLE_CUDA)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    find_package(CUDAToolkit REQUIRED)
    
    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
        if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
            set(CMAKE_CUDA_ARCHITECTURES native)
        else()
            # Support RTX 20/30/40 series: 75 (Turing), 80 (Ampere), 86 (Ampere), 89 (Ada), 90 (Hopper)
            set(CMAKE_CUDA_ARCHITECTURES 75 80 86 89 90)
        endif()
    endif()
else()
    message(STATUS "CUDA disabled (NBODY_ENABLE_CUDA=OFF)")
endif()

# ── 构建选项 ─────────────────────────────────────────────────────
option(NBODY_BUILD_TESTS "Build unit tests" ON)
option(NBODY_BUILD_EXAMPLES "Build example executables" ON)
option(NBODY_BUILD_BENCHMARKS "Build benchmark executables" ON)

if(NBODY_ENABLE_RENDERING AND NOT NBODY_ENABLE_CUDA)
    message(WARNING "Rendering requires CUDA-enabled interop in the current codebase; disabling rendering because NBODY_ENABLE_CUDA=OFF")
    set(NBODY_ENABLE_RENDERING OFF)
endif()

if(NBODY_ENABLE_UI AND NOT NBODY_ENABLE_RENDERING)
    message(STATUS "UI disabled (NBODY_ENABLE_RENDERING=OFF)")
    set(NBODY_ENABLE_UI OFF)
endif()

if(NBODY_BUILD_EXAMPLES AND NOT NBODY_ENABLE_CUDA)
    message(WARNING "Examples require CUDA support in the current codebase; disabling examples because NBODY_ENABLE_CUDA=OFF")
    set(NBODY_BUILD_EXAMPLES OFF)
endif()

# ── 第三方依赖 ───────────────────────────────────────────────────
if(NBODY_ENABLE_RENDERING)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 3.3 REQUIRED)
    find_package(GLEW 2.1 REQUIRED)
endif()

if(NBODY_ENABLE_RENDERING OR (NBODY_BUILD_TESTS AND NBODY_ENABLE_CUDA))
    find_package(glm 0.9.9 REQUIRED)
endif()

include(FetchContent)

if(NBODY_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    if(NBODY_ENABLE_CUDA)
        FetchContent_Declare(
            rapidcheck
            GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
            GIT_TAG 1.1.0
        )
        FetchContent_MakeAvailable(rapidcheck)
    endif()
endif()

if(NBODY_ENABLE_UI)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking
    )
    FetchContent_MakeAvailable(imgui)

    # ImGui library with core and backend files
    set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    add_library(imgui STATIC ${IMGUI_SOURCES})
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
endif()

# ── HDF5 for scientific data export/import ───────────────────────────────────
if(NBODY_ENABLE_HDF5)
    find_package(HDF5 QUIET COMPONENTS CXX)
    if(HDF5_FOUND)
        message(STATUS "HDF5 found: ${HDF5_VERSION}")
    else()
        message(STATUS "HDF5 not found - fetching via FetchContent")
        include(FetchContent)
        FetchContent_Declare(
            hdf5
            GIT_REPOSITORY https://github.com/HDFGroup/hdf5.git
            GIT_TAG hdf5_1.14.4
        )
        set(HDF5_ENABLE_PARALLEL OFF CACHE BOOL "" FORCE)
        set(HDF5_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(HDF5_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(HDF5_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(hdf5)
        set(HDF5_CXX_LIBRARIES hdf5::hdf5_cpp-static)
        set(HDF5_CXX_INCLUDE_DIRS ${hdf5_SOURCE_DIR}/c++/src ${hdf5_BINARY_DIR})
    endif()
endif()

# Source files
set(CUDA_SOURCES)
if(NBODY_ENABLE_CUDA)
    set(CUDA_SOURCES
        src/cuda/force_direct.cu
        src/cuda/force_barnes_hut.cu
        src/cuda/force_spatial_hash.cu
        src/cuda/integrator.cu
        src/cuda/particle_init.cu
    )
endif()

set(CPP_SOURCES
    src/core/app_cli.cpp
    src/core/particle_system.cpp
    src/core/barnes_hut_tree.cpp
    src/core/spatial_hash_grid.cpp
    src/utils/error_handling.cpp
    src/utils/performance_observability.cpp
    src/utils/serialization.cpp
    src/utils/simulation_state.cpp
)

if(NBODY_ENABLE_RENDERING)
    list(APPEND CPP_SOURCES
        src/render/camera.cpp
        src/render/renderer.cpp
    )
endif()

if(NBODY_ENABLE_RENDERING AND NBODY_ENABLE_UI)
    list(APPEND CPP_SOURCES
        src/render/ui_panel.cpp
    )
endif()

if(NBODY_ENABLE_RENDERING AND NBODY_ENABLE_CUDA)
    list(APPEND CPP_SOURCES
        src/render/cuda_gl_interop.cpp
    )
endif()

if(NBODY_ENABLE_HDF5)
    list(APPEND CPP_SOURCES
        src/utils/hdf5_io.cpp
    )
endif()

# ── 主库 ─────────────────────────────────────────────────────────
add_library(nbody_lib STATIC ${CUDA_SOURCES} ${CPP_SOURCES})
add_library(nbody::nbody_lib ALIAS nbody_lib)

target_include_directories(nbody_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(nbody_lib
    PUBLIC
        NBODY_WITH_CUDA=$<BOOL:${NBODY_ENABLE_CUDA}>
        NBODY_WITH_RENDERING=$<BOOL:${NBODY_ENABLE_RENDERING}>
        NBODY_WITH_UI=$<BOOL:${NBODY_ENABLE_UI}>
        NBODY_WITH_HDF5=$<BOOL:${NBODY_ENABLE_HDF5}>
        NBODY_ENABLE_PROFILING=$<BOOL:${NBODY_ENABLE_PROFILING}>
)

if(NBODY_ENABLE_RENDERING OR (NBODY_BUILD_TESTS AND NBODY_ENABLE_CUDA))
    target_link_libraries(nbody_lib PUBLIC glm::glm)
endif()

if(NBODY_ENABLE_CUDA)
    target_link_libraries(nbody_lib PUBLIC CUDA::cudart)
    set_target_properties(nbody_lib PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
endif()

if(NBODY_ENABLE_RENDERING)
    target_link_libraries(nbody_lib PUBLIC
        OpenGL::GL
        glfw
        GLEW::GLEW
    )
endif()

if(NBODY_ENABLE_UI)
    target_link_libraries(nbody_lib PUBLIC imgui)
endif()

if(NBODY_ENABLE_HDF5)
    if(HDF5_FOUND)
        target_include_directories(nbody_lib PUBLIC ${HDF5_INCLUDE_DIRS})
        target_link_libraries(nbody_lib PUBLIC ${HDF5_CXX_LIBRARIES})
    else()
        target_link_libraries(nbody_lib PUBLIC hdf5::hdf5_cpp-static)
    endif()
endif()

target_compile_options(nbody_lib
    PRIVATE
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:GNU,Clang>>:-Wall -Wextra>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/W4>
)

# ── 主程序 ───────────────────────────────────────────────────────
if(NBODY_ENABLE_RENDERING)
    add_executable(nbody_sim src/main.cpp)
    target_link_libraries(nbody_sim PRIVATE nbody_lib)
endif()

# ── 示例程序 ─────────────────────────────────────────────────────
if(NBODY_BUILD_EXAMPLES)
    set(NBODY_EXAMPLES
        example_basic
        example_force_methods
        example_custom_distribution
        example_energy_conservation
    )

    foreach(example_name IN LISTS NBODY_EXAMPLES)
        add_executable(${example_name} examples/${example_name}.cpp)
        target_link_libraries(${example_name} PRIVATE nbody_lib)
    endforeach()
endif()

if(NBODY_BUILD_BENCHMARKS)
    add_executable(nbody_benchmarks
        benchmarks/benchmark_main.cpp
    )
    target_link_libraries(nbody_benchmarks PRIVATE nbody_lib)
endif()

# ── 测试 ─────────────────────────────────────────────────────────
if(NBODY_BUILD_TESTS)
    enable_testing()
    add_executable(nbody_observability_tests
        tests/test_main.cpp
        tests/test_app_cli.cpp
        tests/test_performance_observability.cpp
        tests/test_ui_panel.cpp
    )
    target_link_libraries(nbody_observability_tests PRIVATE
        nbody_lib
        GTest::gtest
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(nbody_observability_tests)
    if(NBODY_ENABLE_CUDA)
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
            tests/test_hdf5_io.cpp
        )
        target_link_libraries(nbody_tests PRIVATE
            nbody_lib
            GTest::gtest
            GTest::gtest_main
            rapidcheck
        )
        gtest_discover_tests(nbody_tests)
    endif()
endif()
