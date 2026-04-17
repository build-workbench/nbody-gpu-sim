#!/usr/bin/env bash
# Build script for n-body project

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"

# Default build type
BUILD_TYPE="${1:-Release}"

echo "🔨 Building n-body project (${BUILD_TYPE})..."

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo "⚙️  Configuring..."
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -G "Unix Makefiles"

# Build
echo "⚡ Building..."
cmake --build . -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo "✅ Build complete!"
echo "📁 Executable: ${BUILD_DIR}/nbody_sim"
