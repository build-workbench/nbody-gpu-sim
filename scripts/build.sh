#!/usr/bin/env bash
# Build script for n-body project

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"

# Default build type
BUILD_TYPE="${1:-Release}"

echo "🔨 Building n-body project (${BUILD_TYPE})..."

# Detect CUDA availability
if command -v nvcc &> /dev/null; then
    CUDA_ENABLED=ON
    echo "✓ CUDA detected"
else
    CUDA_ENABLED=OFF
    echo "⚠ CUDA not found; CMake will auto-disable rendering, UI and examples"
    echo "  (the headless core library and core tests are still built)."
fi

# Detect rendering dependencies (only meaningful when CUDA is available).
# Without them, configure would hard-fail in find_package; degrade gracefully
# to a headless build instead.
RENDERING_ENABLED="$CUDA_ENABLED"
if [ "$RENDERING_ENABLED" = "ON" ]; then
    HAVE_GL=1
    pkg-config --exists glfw3 2>/dev/null || [ -f /usr/include/GLFW/glfw3.h ] || HAVE_GL=0
    pkg-config --exists glew 2>/dev/null || [ -f /usr/include/GL/glew.h ] || HAVE_GL=0
    [ -d /usr/include/glm ] || HAVE_GL=0
    if [ "$HAVE_GL" = "0" ]; then
        RENDERING_ENABLED=OFF
        echo "⚠ OpenGL dev packages (GLFW/GLEW/GLM) not found; disabling rendering."
        echo "  Install them for the visualizer, e.g.:"
        echo "    sudo apt install libglfw3-dev libglew-dev libglm-dev"
    fi
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo "⚙️  Configuring..."
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DNBODY_BUILD_EXAMPLES=ON \
    -DNBODY_ENABLE_CUDA="$CUDA_ENABLED" \
    -DNBODY_ENABLE_RENDERING="$RENDERING_ENABLED" \
    -G "Unix Makefiles"

# Build
echo "⚡ Building..."
cmake --build . -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

if [ -f "${BUILD_DIR}/compile_commands.json" ]; then
  ln -sfn "${BUILD_DIR}/compile_commands.json" "${PROJECT_DIR}/compile_commands.json"
  echo "🔗 Linked compile_commands.json at project root"
fi

echo "✅ Build complete!"
if [ -f "${BUILD_DIR}/nbody_sim" ]; then
  echo "📁 Executable: ${BUILD_DIR}/nbody_sim"
else
  echo "📦 Core library: ${BUILD_DIR}/libnbody_lib.a"
  echo "ℹ️  Renderer and examples may be disabled in headless core-only builds."
fi

if [ -f "${BUILD_DIR}/nbody_core_tests" ]; then
  echo "🧪 Headless tests: ${BUILD_DIR}/nbody_core_tests"
fi

if [ -f "${BUILD_DIR}/nbody_tests" ]; then
  echo "🧪 CUDA tests: ${BUILD_DIR}/nbody_tests"
fi

if [ -f "${BUILD_DIR}/nbody_render_tests" ]; then
  echo "🧪 Render tests: ${BUILD_DIR}/nbody_render_tests"
fi

if [ -f "${BUILD_DIR}/nbody_benchmarks" ]; then
  echo "📊 Benchmarks: ${BUILD_DIR}/nbody_benchmarks"
fi

echo ""
echo "📌 Tips:"
echo "   To force disable CUDA: cmake .. -DNBODY_ENABLE_CUDA=OFF"
echo "   To force disable rendering: cmake .. -DNBODY_ENABLE_RENDERING=OFF"
echo "   To enable profiling hooks: cmake .. -DNBODY_ENABLE_PROFILING=ON"
