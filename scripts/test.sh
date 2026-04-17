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

TESTS_BIN="${BUILD_DIR}/nbody_tests"

if [ ! -f "$TESTS_BIN" ]; then
    echo "❌ Tests not built. Run ./scripts/build.sh first."
    exit 1
fi

echo "🧪 Running tests..."
cd "$BUILD_DIR"

if [ "${1:-}" = "--verbose" ]; then
    ./nbody_tests --gtest_color=yes
else
    ./nbody_tests --gtest_color=yes --gtest_brief=1
fi

echo "✅ Tests complete!"
