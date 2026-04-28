#!/usr/bin/env bash
# Test script for n-body project

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"

# Check if build exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ Build directory not found. Run ./scripts/build.sh first."
    exit 1
fi

CACHE_FILE="${BUILD_DIR}/CMakeCache.txt"
CTEST_FILE="${BUILD_DIR}/CTestTestfile.cmake"

if [ ! -f "$CTEST_FILE" ]; then
    if [ -f "$CACHE_FILE" ]; then
        TESTS_ENABLED="$(grep '^NBODY_BUILD_TESTS:BOOL=' "$CACHE_FILE" | cut -d= -f2 || true)"
        CUDA_ENABLED="$(grep '^NBODY_ENABLE_CUDA:BOOL=' "$CACHE_FILE" | cut -d= -f2 || true)"

        if [ "${TESTS_ENABLED:-ON}" != "ON" ]; then
            echo "❌ Tests were not built in the current configuration."
            echo "   Reconfigure with NBODY_BUILD_TESTS=ON to run the test suite."
            exit 1
        fi

        if [ "${CUDA_ENABLED:-ON}" != "ON" ]; then
            echo "❌ No discovered tests are available in the current build directory."
            echo "ℹ️  Rebuild with ./scripts/build.sh so the headless observability tests are generated."
            exit 1
        fi
    fi

    echo "❌ Tests not built. Run ./scripts/build.sh first."
    exit 1
fi

echo "🧪 Running tests..."
cd "$BUILD_DIR"

if [ "${1:-}" = "--verbose" ]; then
    ctest --output-on-failure -V
else
    ctest --output-on-failure
fi

echo "✅ Tests complete!"
