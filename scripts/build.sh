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
    echo "⚠ CUDA not found; building CPU-only mode"
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
    -G "Unix Makefiles"

# Build
echo "⚡ Building..."
cmake --build . -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

if [ -f "${BUILD_DIR}/compile_commands.json" ]; then
  ln -sfn "${BUILD_DIR}/compile_commands.json" "${PROJECT_DIR}/compile_commands.json"
  echo "🔗 Linked compile_commands.json at project root"
fi

echo "✅ Build complete!"
echo "📁 Executable: ${BUILD_DIR}/nbody_sim"
echo ""
echo "📌 Tip: To force disable CUDA, use:"
echo "   cmake .. -DNBODY_ENABLE_CUDA=OFF"

