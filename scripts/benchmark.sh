#!/usr/bin/env bash
# Benchmark script for n-body project

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"

BENCHMARK_NAME="${1:-all}"
OUTPUT_PATH="${2:-${BUILD_DIR}/benchmark-results.json}"
PARTICLE_COUNT="${NBODY_BENCHMARK_PARTICLES:-4096}"
ITERATIONS="${NBODY_BENCHMARK_ITERATIONS:-5}"

if [ ! -x "${BUILD_DIR}/nbody_benchmarks" ]; then
    echo "❌ Benchmark executable not found. Run ./scripts/build.sh first."
    exit 1
fi

mkdir -p "$(dirname "$OUTPUT_PATH")"

echo "📊 Running benchmarks..."
"${BUILD_DIR}/nbody_benchmarks" \
    --benchmark "$BENCHMARK_NAME" \
    --particle-count "$PARTICLE_COUNT" \
    --iterations "$ITERATIONS" \
    --output "$OUTPUT_PATH"

echo "✅ Benchmark run complete!"
echo "📄 Results: ${OUTPUT_PATH}"
